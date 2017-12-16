#include <stdint.h>
#include <stdbool.h>

#include <arch/memory.h>
#include <arch/virtual_memory.h>
#include <kernel/kassert.h>
#include <kernel/kernel_image.h>
#include <kernel/memory.h>
#include <kernel/paging.h>
#include <kernel/process.h>
#include <kernel/sched/sched.h>
#include <kernel/types.h>
#include <libk/libk.h>

#define index_in_pd(addr) (addr >> 22)
#define index_in_pt(addr) ((addr >> 12) & 0x3ff)

#define p_addr2pd_addr(paddr) (paddr >> 12)
#define p_addr2pt_addr(paddr) p_addr2pd_addr(paddr)

#define pd_addr2p_addr(pd_addr) (pd_addr << 12)
#define pt_addr2p_addr(pt_addr) pd_addr2p_addr(pt_addr)

#define PAGE_DIRECTORY_ENTRY_COUNT	1024
#define PAGE_TABLE_ENTRY_COUNT		1024
struct page_directory_entry
{
	uint8_t present:1;
	uint8_t read_write:1;
	uint8_t user:1;
	uint8_t write_through:1;
	uint8_t cache_disable:1;
	uint8_t accessed:1;
	uint8_t zero:1;
	uint8_t page_size:1;
	uint8_t ignored:1;
	uint8_t available:3;

	p_addr_t address:20;
} __attribute__((packed));

struct page_table_entry
{
	uint8_t present:1;
	uint8_t read_write:1;
	uint8_t user:1;
	uint8_t write_through:1;
	uint8_t cache_disable:1;
	uint8_t accessed:1;
	uint8_t dirty:1;
	uint8_t zero:1;
	uint8_t global:1;
	uint8_t available:3;

	p_addr_t address:20;
} __attribute__((packed));


/*
 * boot page directory and boot page table symbols
 * defined in boot.s
 */
extern uint8_t __boot_page_directory_phys;
extern uint8_t __boot_page_directory;
extern uint8_t __boot_page_table_phys;
extern uint8_t __boot_page_table;

/*
 * saves how many times a page table is referenced i.e. how many entries
 * are present in the page table.
 * When there are no more entries, we can free the page frame holding the
 * page table
 */
static unsigned int kernel_addr_space_pd_reference_pf_refs[KERNEL_VADDR_SPACE_PAGE_DIRECTORY_ENTRIES] = { 0 };

static inline void paging_ref_page_table(int pd_index)
{
	// increment reference count
	if (pd_index < index_in_pd(MIRRORING_VADDR_BEGIN)) {
		++kernel_addr_space_pd_reference_pf_refs[pd_index];
	}
	else {
		// icrement reference count in current vm_context
		++sched_get_current_process()->vm_ctx.page_tables_page_frame_refs[pd_index - KERNEL_VADDR_SPACE_PAGE_DIRECTORY_ENTRIES];
	}
}

static inline unsigned int paging_unref_page_table(int pd_index)
{
	unsigned int* ref_count = NULL;
	if (pd_index < index_in_pd(MIRRORING_VADDR_BEGIN)) {
		ref_count = &kernel_addr_space_pd_reference_pf_refs[pd_index];
	}
	else {
		// fetch reference count in current vm_context
		ref_count = &sched_get_current_process()->vm_ctx.page_tables_page_frame_refs[pd_index - KERNEL_VADDR_SPACE_PAGE_DIRECTORY_ENTRIES];
	}

	kassert(ref_count != NULL);
	kassert(*ref_count > 0);

	return --(*ref_count);
}

static inline void invlpg(v_addr_t addr)
{
	__asm__ __volatile__ ("invlpg %0" : : "m" (addr) : "memory");
}

static void setup_mirroring(v_addr_t page_directory, p_addr_t page_directory_phys)
{
	struct page_directory_entry* mirroring_entry =
		(struct page_directory_entry*)page_directory +
		index_in_pd(MIRRORING_VADDR_BEGIN);

	mirroring_entry->present = 1;
	mirroring_entry->read_write = 1;
	mirroring_entry->user = 0;
	mirroring_entry->address = p_addr2pd_addr(page_directory_phys);
	invlpg(MIRRORING_VADDR_BEGIN);
}

/**
 * The kernel was loaded at 1MB and the first 4MB of physical memory were
 * mapped to KERNEL_VADDR_SPACE_START in boot.s
 * Unmap the "extra" pages that were map so only the frames were the kernel
 * actually lives are mapped.
 */
static void map_to_fit_kernel(void)
{
	// kernel top virtual page
	const v_addr_t kernel_top = kernel_image_get_top_page();
	const int pt_index = index_in_pt(kernel_top);
	struct page_table_entry* const pt = (struct page_table_entry*)&__boot_page_table;

	// Clear every entry in the kernel page table starting at the entry for the
	// kernel_top page
	v_addr_t page = kernel_top;
	for (int i = pt_index; i < PAGE_TABLE_ENTRY_COUNT; ++i, page += PAGE_SIZE) {
		memset(&pt[i], 0, sizeof(struct page_table_entry));
		invlpg(page);
	}
}

/**
 * The first 4MB of physical memory were identity mapped to avoid a crash,
 * we can now remove this mapping.
 */
static void unmap_identity_map(void)
{
	struct page_directory_entry* pd = (struct page_directory_entry*)&__boot_page_directory;
	// clear the page directory entry
	memset(&pd[0], 0, sizeof(struct page_directory_entry));

	// invalidate the translations for the first 4MB/4KB = 1024 pages
	v_addr_t p = 0;
	for (int i = 0; i < 1024; ++i) {
		invlpg(p);
		p += PAGE_SIZE;
	}
}

void paging_init(void)
{
	map_to_fit_kernel();

	setup_mirroring((v_addr_t)&__boot_page_directory, (p_addr_t)&__boot_page_directory_phys);

	unmap_identity_map();
}

static inline struct page_directory_entry* get_page_directory_entry(v_addr_t vaddr)
{
	int pde_index = index_in_pd(vaddr);
	return ((struct page_directory_entry*)(MIRRORING_VADDR_BEGIN +
			(index_in_pd(MIRRORING_VADDR_BEGIN) << PAGE_SIZE_SHIFT)) +
			pde_index);
}

static inline struct page_table_entry* get_page_table_entry(v_addr_t vaddr)
{
	int pde_index = index_in_pd(vaddr);
	int pte_index = index_in_pt(vaddr);
	return ((struct page_table_entry*)(MIRRORING_VADDR_BEGIN +
			(pde_index << PAGE_SIZE_SHIFT)) + pte_index);
}

int paging_map(p_addr_t paddr, v_addr_t vaddr, uint8_t flags)
{
	struct page_directory_entry* pde = get_page_directory_entry(vaddr);
	struct page_table_entry* pte = get_page_table_entry(vaddr);

	// no page table
	if (!pde->present) {
		p_addr_t page_table = memory_page_frame_alloc();
		kassert(page_table != (p_addr_t)NULL);

		pde->present = 1;
		pde->read_write = 1;
		pde->user = virtual_memory_is_userspace_address(vaddr);
		pde->address = p_addr2pd_addr(page_table);

		invlpg((v_addr_t)pte);

		memset((void*)page_frame_align_inf((v_addr_t)pte), 0, PAGE_SIZE);
	}


	// a page frame is already mapped for the virtual page
	// (vaddr & 0xfffff000) -> (vaddr | 0xfff)
	if (pte->present)
		return -1;

	pte->present = 1;
	pte->read_write = ((flags & VM_OPT_WRITE) == VM_OPT_WRITE) ? 1 : 0;
	pte->user = ((flags & VM_OPT_USER) == VM_OPT_USER) ? 1 : 0;
	pte->address = p_addr2pt_addr(paddr);

	paging_ref_page_table(index_in_pd(vaddr));

	invlpg(vaddr);

	return 0;
}

static int _paging_unmap(v_addr_t vaddr, bool free_page_frame)
{
	struct page_directory_entry* pde = get_page_directory_entry(vaddr);
	struct page_table_entry* pte = get_page_table_entry(vaddr);

	if (!pde->present || !pte->present)
		return -1;

	// free the page frame
	if (free_page_frame)
		memory_page_frame_free(pt_addr2p_addr(pte->address));
	// reset the page table entry
	memset(pte, 0, sizeof(struct page_table_entry));

	invlpg(vaddr);

	// unref the physical page holding the page table
	// if this was the last reference, free the page frame and reset the
	// page directory entry
	const p_addr_t page_table = pd_addr2p_addr(pde->address);
	const int pd_index = index_in_pd(vaddr);
	if (paging_unref_page_table(pd_index) == 0) {
		memory_page_frame_free(page_table);

		memset(pde, 0, sizeof(struct page_directory_entry));

		invlpg((v_addr_t)pte);
	}

	return 0;
}

int paging_unmap(v_addr_t vaddr)
{
	return _paging_unmap(vaddr, true);
}

void paging_switch_cr3(p_addr_t cr3, bool init_userspace)
{
	const v_addr_t cr3_map = KERNEL_VADDR_SPACE_RESERVED;
	const struct page_directory_entry* pd =
		(struct page_directory_entry*)(MIRRORING_VADDR_BEGIN +
			(index_in_pd(MIRRORING_VADDR_BEGIN) << PAGE_SIZE_SHIFT));

	paging_map(cr3, (v_addr_t)cr3_map, VM_OPT_WRITE);

	const int kernel_vaddr_space_start_pd_index = index_in_pd(KERNEL_VADDR_SPACE_START);
	// copy kernel space entries
	memcpy((struct page_directory_entry*)cr3_map + kernel_vaddr_space_start_pd_index,
		   pd + kernel_vaddr_space_start_pd_index,
		   KERNEL_VADDR_SPACE_PAGE_DIRECTORY_ENTRIES *
		   sizeof(struct page_directory_entry));
	// clear reserved entry
	memset((struct page_directory_entry*)cr3_map +
			index_in_pd(KERNEL_VADDR_SPACE_RESERVED), 0,
			sizeof(struct page_directory_entry));
	// mirroring
	setup_mirroring(cr3_map, cr3);
	if (init_userspace) {
		// clear user space entries
		memset((void*)cr3_map, 0,
				USER_VADDR_SPACE_PAGE_DIRECTORY_ENTRIES *
				sizeof(struct page_table_entry));
	}

	// unmap but don't free the cr3 page frame
	_paging_unmap((v_addr_t)cr3_map, false);

	__asm__ __volatile__ (
			"movl %0, %%eax\n"
			"movl %%eax, %%cr3"
			:
			: "m" (cr3)
			: "eax", "memory");
}
