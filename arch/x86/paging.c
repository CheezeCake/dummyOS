#include <stdint.h>
#include <stdbool.h>
#include <kernel/paging.h>
#include <kernel/kassert.h>
#include <kernel/types.h>
#include <kernel/libk.h>
#include <kernel/memory.h>
#include <arch/memory.h>
#include <arch/virtual_memory.h>

#include <kernel/log.h>

#define index_in_pd(addr) (addr >> 22)
#define index_in_pt(addr) ((addr >> 12) & 0x3ff)

#define p_addr2pd_addr(paddr) (paddr >> 12)
#define p_addr2pt_addr(paddr) p_addr2pd_addr(paddr)

#define pd_addr2p_addr(pd_addr) (pd_addr << 12)
#define pt_addr2p_addr(pt_addr) pd_addr2p_addr(pt_addr)

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


static inline void invlpg(uint32_t addr)
{
	__asm__ __volatile__ ("invlpg %0" : :"m" (addr) : "memory");
}

static void identity_mapping(p_addr_t page_directory, p_addr_t from, p_addr_t to)
{
	/*
	 * paging is not enabled here, we are still working with physical adresses
	 */
	kassert(from <= to);
	for (p_addr_t addr = from; addr < to; addr += PAGE_SIZE) {
		int index_pd = index_in_pd(addr);
		int index_pt = index_in_pt(addr);

		struct page_directory_entry* pde = (struct page_directory_entry*)page_directory + index_pd;
		p_addr_t page_table = 0;
		struct page_table_entry* pte = NULL;
		if (pde->present) {
			page_table = pd_addr2p_addr(pde->address);
			memory_ref_page_frame(page_table);
		}
		else {
			page_table = memory_page_frame_alloc();
			kassert(page_table != (p_addr_t)NULL);
			memset((void*)page_table, 0, PAGE_SIZE);

			pde->present = 1;
			pde->read_write = 1;
			pde->address = p_addr2pd_addr(page_table);
		}

		pte = (struct page_table_entry*)page_table + index_pt;
		pte->present = 1;
		pte->read_write = 1;
		pte->address = p_addr2pt_addr(addr);
	}
}

void paging_init(p_addr_t kernel_top_page_frame)
{
	p_addr_t page_directory = memory_page_frame_alloc();
	kassert(page_directory != (p_addr_t)NULL);
	memset((void*)page_directory, 0, PAGE_SIZE);

	// mirroring
	struct page_directory_entry* mirroring_entry =
		(struct page_directory_entry*)page_directory +
		index_in_pd(MIRRORING_VADDR_BEGIN);
	mirroring_entry->present = 1;
	mirroring_entry->read_write = 1;
	mirroring_entry->user = 0;
	mirroring_entry->address = p_addr2pd_addr(page_directory);


	identity_mapping(page_directory, X86_MEMORY_HARDWARE_MAP_BEGIN,
			X86_MEMORY_HARDWARE_MAP_END);
	identity_mapping(page_directory, get_kernel_base_page_frame(),
			kernel_top_page_frame);

	__asm__ (
			"movl %0, %%eax\n"
			"movl %%eax, %%cr3\n"
			"movl %%cr0, %%eax\n"
			"orl $0x80000000, %%eax\n"
			"movl %%eax, %%cr0\n"
			:
			: "m" (page_directory));
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
		// vaddr < MIRRORING_VADDR_BEGIN => kernel page table
		pde->user = (vaddr < MIRRORING_VADDR_BEGIN) ? 0 : 1;
		pde->address = p_addr2pd_addr(page_table);

		invlpg((uint32_t)pte);

		memset(pte, 0, PAGE_SIZE);
	}


	// a page frame is already mapped for the virtual page
	// (vaddr & 0xfffff000) -> (vaddr | 0xfff)
	if (pte->present)
		return -1;

	pte->present = 1;
	pte->read_write = ((flags & VM_OPT_WRITE) == VM_OPT_WRITE) ? 1 : 0;
	pte->user = ((flags & VM_OPT_USER) == VM_OPT_USER) ? 1 : 0;
	pte->address = p_addr2pt_addr(paddr);
	memory_ref_page_frame(paddr);

	invlpg(vaddr);

	return 0;
}

int paging_unmap(v_addr_t vaddr)
{
	struct page_directory_entry* pde = get_page_directory_entry(vaddr);
	struct page_table_entry* pte = NULL;

	if (!pde->present)
		return -1;

	pte = get_page_table_entry(vaddr);
	if (!pde->present)
		return -1;

	// unref the physical page and reset the page table entry
	memory_unref_page_frame(pt_addr2p_addr(pte->address));
	memset(pte, 0, sizeof(struct page_table_entry));

	invlpg(vaddr);

	// unref the physical page holding the page table
	// if this was the last reference, reset the page directory entry
	if (memory_unref_page_frame(pd_addr2p_addr(pde->address)) == 0) {
		memset(pde, 0, sizeof(struct page_directory_entry));
		invlpg((uint32_t)pte);
	}

	return 0;
}
