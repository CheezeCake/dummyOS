#include <arch/memory.h>
#include <arch/vm.h>
#include <dummyos/errno.h>
#include <kernel/kassert.h>
#include <kernel/kernel_image.h>
#include <kernel/mm/memory.h>
#include <kernel/process.h>
#include <kernel/sched/sched.h>
#include <kernel/types.h>
#include <libk/libk.h>
#include "paging.h"

#define index_in_pd(addr) (addr >> 22)
#define index_in_pt(addr) ((addr >> 12) & 0x3ff)

#define p_addr2pd_addr(paddr) (paddr >> 12)
#define p_addr2pt_addr(paddr) p_addr2pd_addr(paddr)

#define pd_addr2p_addr(pd_addr) (pd_addr << 12)
#define pt_addr2p_addr(pt_addr) pd_addr2p_addr(pt_addr)

#define pd_pt_index2v_addr(pd_index, pt_index) (pd_index << 22 | pt_index << 12)

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
typedef struct page_directory_entry pde_t;

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
typedef struct page_table_entry pte_t;

/*
 * boot page directory and boot page table symbols
 * defined in boot.s
 */
extern pde_t __boot_page_directory_phys;
extern pde_t __boot_page_directory;
extern pte_t __boot_page_table_phys;
extern pte_t __boot_page_table;

static inline void invlpg(v_addr_t addr)
{
	__asm__ volatile ("invlpg (%0)" : : "r" (addr) : "memory");
}

static inline pde_t* get_page_directory(void)
{
	return (pde_t*)(RECURSIVE_ENTRY_START +
					(index_in_pd(RECURSIVE_ENTRY_START) << PAGE_SIZE_SHIFT));
}

static inline pte_t* get_page_table(unsigned int pde_index)
{
	return (pte_t*)((int8_t*)RECURSIVE_ENTRY_START +
					(pde_index << PAGE_SIZE_SHIFT));
}

static inline pde_t* get_temp_page_directory(void)
{
	return (pde_t*)(TEMP_RECURSIVE_ENTRY_START +
					(index_in_pd(RECURSIVE_ENTRY_START) << PAGE_SIZE_SHIFT));
}

static inline pte_t* get_temp_page_table(unsigned int pde_index)
{
	return (pte_t*)((int8_t*)TEMP_RECURSIVE_ENTRY_START +
					(pde_index << PAGE_SIZE_SHIFT));
}

static void make_recursive_entry(pde_t* pd, p_addr_t pd_phys,
								 v_addr_t recursive_entry_addr)
{
	int index = index_in_pd(recursive_entry_addr);
	pde_t* recursive_entry = &pd[index];

	recursive_entry->present = 1;
	recursive_entry->read_write = 1;
	recursive_entry->user = 0;
	recursive_entry->address = p_addr2pd_addr(pd_phys);

	invlpg(recursive_entry_addr);
}

static void clear_recursive_entry(pde_t* pd, v_addr_t recursive_entry_addr)
{
	int index = index_in_pd(recursive_entry_addr);
	pde_t* recursive_entry = &pd[index];

	memset(recursive_entry, 0, sizeof(pde_t));

	// invalidate the 4MB covered by the pd entry
	v_addr_t addr = recursive_entry_addr;
	for (size_t i = 0; i < PAGE_TABLE_ENTRY_COUNT; ++i, addr += PAGE_SIZE)
		invlpg(addr);
}

static void setup_recursive_entry(pde_t* pd, p_addr_t pd_phys)
{
	make_recursive_entry(pd, pd_phys, RECURSIVE_ENTRY_START);
}

static void setup_temp_recursive_entry(p_addr_t pd)
{
	make_recursive_entry(get_page_directory(), pd, TEMP_RECURSIVE_ENTRY_START);
}

static void reset_temp_recursive_entry()
{
	clear_recursive_entry(get_page_directory(), TEMP_RECURSIVE_ENTRY_START);
	invlpg(TEMP_RECURSIVE_ENTRY_START);
}

/**
 * The kernel was loaded at 1MB and the first 4MB of physical memory were
 * mapped to KERNEL_VADDR_SPACE_START in boot.s
 * Unmap the "extra" pages that were mapped so only the frames where the kernel
 * actually lives are mapped.
 */
static void map_to_fit_kernel(void)
{
	pte_t* const pt = &__boot_page_table;

	// 0 to X86_MEMORY_HARDWARE_MAP_BEGIN
	for (v_addr_t low_mem = KERNEL_SPACE_START;
		 low_mem < KERNEL_SPACE_START + X86_MEMORY_HARDWARE_MAP_BEGIN;
		 low_mem += PAGE_SIZE)
	{
		const int pt_index = index_in_pt(low_mem);

		memset(&pt[pt_index], 0, sizeof(pte_t));
		invlpg(low_mem);
	}

	// kernel top virtual page
	const v_addr_t kernel_top = kernel_image_get_top_page();
	const int pt_index = index_in_pt(kernel_top);

	// Clear every entry in the kernel page table starting at the entry for the
	// kernel_top page
	v_addr_t page = kernel_top;
	for (int i = pt_index; i < PAGE_TABLE_ENTRY_COUNT; ++i, page += PAGE_SIZE) {
		memset(&pt[i], 0, sizeof(pte_t));
		invlpg(page);
	}
}

/**
 * The first 4MB of physical memory were identity mapped to avoid a crash,
 * we can now remove this mapping.
 */
static void unmap_identity_map(void)
{
	pde_t* pd = (pde_t*)&__boot_page_directory;
	// clear the page directory entry
	memset(&pd[0], 0, sizeof(pde_t));

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

	setup_recursive_entry(&__boot_page_directory,
						  (p_addr_t)&__boot_page_directory_phys);

	unmap_identity_map();
}

int paging_map(p_addr_t paddr, v_addr_t vaddr, int prot)
{
	pde_t* pd = get_page_directory();
	size_t pdi = index_in_pd(vaddr);
	pte_t* pt = get_page_table(pdi);
	size_t pti = index_in_pt(vaddr);

	// no page table
	if (!pd[pdi].present) {
		p_addr_t page_table = memory_page_frame_alloc();
		if (!page_table)
			return -ENOMEM;

		pd[pdi].present = 1;
		pd[pdi].read_write = 1;
		pd[pdi].user = (prot & VMM_PROT_USER) ? 1 : 0;
		pd[pdi].address = p_addr2pd_addr(page_table);

		invlpg((v_addr_t)pt);

		memset(pt, 0, PAGE_SIZE);

		if (!vmm_is_userspace_address(vaddr))
			vmm_sync_kernel_space(&pdi);
	}

	// a page frame is already mapped for the virtual page
	// (vaddr & 0xfffff000) -> (vaddr | 0xfff)
	if (pt[pti].present)
		return -EFAULT;

	pt[pti].present = 1;
	pt[pti].read_write = (prot & VMM_PROT_WRITE) ? 1 : 0;
	pt[pti].user = (prot & VMM_PROT_USER) ? 1 : 0;
	pt[pti].address = p_addr2pt_addr(paddr);

	invlpg(vaddr);

	return 0;
}

int paging_unmap(v_addr_t vaddr)
{
	pde_t* pd = get_page_directory();
	size_t pdi = index_in_pd(vaddr);
	pte_t* pt = get_page_table(pdi);
	size_t pti = index_in_pt(vaddr);

	if (!pd[pdi].present || !pt[pti].present)
		return -EFAULT;

	// reset the page table entry
	memset(&pt[pti], 0, sizeof(pte_t));

	invlpg(vaddr);

	return 0;
}

void paging_switch_cr3(p_addr_t cr3)
{
	log_printf("Switching cr3 (%p)!\n", (void*)cr3);
	__asm__ volatile ("movl %%eax, %%cr3" : : "a" (cr3) : "memory");
}

int paging_sync_kernel_space(p_addr_t cr3, void* data)
{
	size_t pdi = *(size_t*)data;
	pde_t* pd = get_temp_page_directory();
	pde_t* cur_pd = get_page_directory();

	setup_temp_recursive_entry(cr3);

	memcpy(&pd[pdi], &cur_pd[pdi], sizeof(pde_t));

	reset_temp_recursive_entry();

	return 0;
}

static int init_kernel_space(p_addr_t cr3, v_addr_t addr, size_t nr_pages)
{
	setup_temp_recursive_entry(cr3);

	size_t idx = index_in_pd(addr);
	pde_t* pd = get_temp_page_directory();
	pde_t* cur_pd = get_page_directory();

	size_t nr_pd_entries = (nr_pages + 1023) / 1024;

	for (size_t i = idx; i < idx + nr_pd_entries; ++i) {
		memcpy(&pd[i], &cur_pd[i], sizeof(pde_t));
	}

	reset_temp_recursive_entry();

	return 0;
}

static int init_pd_recursive_entry(p_addr_t pd)
{
	int err;
	v_addr_t pd_map = KERNEL_SPACE_RESERVED;

	err = paging_map(pd, pd_map, VMM_PROT_WRITE);
	if (!err) {
		memset((void*)pd_map, 0, PAGE_SIZE);
		setup_recursive_entry((pde_t*)pd_map, pd);
		err = paging_unmap(pd_map);
	}

	return err;
}

int paging_init_pd(p_addr_t cr3)
{
	int err;

	err = init_pd_recursive_entry(cr3);
	if (err)
		return err;

	err = init_kernel_space(cr3, KERNEL_SPACE_START,
							(TEMP_RECURSIVE_ENTRY_START -
							 KERNEL_SPACE_START) / PAGE_SIZE);

	return err;
}

int paging_clone_current_cow(p_addr_t cr3)
{
	setup_temp_recursive_entry(cr3);

	pde_t* pd = get_temp_page_directory();
	pde_t* cur_pd = get_page_directory();

	for (size_t i = 0; i < USER_SPACE_PD_ENTRIES; ++i) {
		if (cur_pd[i].present) {
			pte_t* pt = get_temp_page_table(i);
			pte_t* cur_pt = get_page_table(i);

			p_addr_t page_table = memory_page_frame_alloc();
			if (!page_table)
				return -ENOMEM;

			pd[i].present = 1;
			pd[i].read_write = 1;
			pd[i].user = 1;
			pd[i].address = p_addr2pd_addr(page_table);

			invlpg((v_addr_t)pt);
			memset(pt, 0, PAGE_SIZE);

			for (size_t j = 0; j < PAGE_TABLE_ENTRY_COUNT; ++j) {
				if (cur_pt[j].present) {
					memcpy(&pt[j], &cur_pt[j], sizeof(pte_t));

					// CoW permissions
					pt[j].read_write = 0;
					cur_pt[j].read_write = 0;

					invlpg(pd_pt_index2v_addr(i, j));
				}
			}
		}
	}

	reset_temp_recursive_entry();

	return 0;
}

int paging_copy_page(v_addr_t src_page, p_addr_t dst_frame)
{
	int err;
	v_addr_t dst_page = KERNEL_SPACE_RESERVED;

	err = paging_map(dst_frame, dst_page, VMM_PROT_WRITE);
	if (err)
		return err;

	memcpy((void*)dst_page, (void*)src_page, PAGE_SIZE);

	paging_unmap(dst_page);

	return 0;
}

int paging_update_prot(v_addr_t page, int prot)
{
	pde_t* pd = get_page_directory();
	size_t pdi = index_in_pd(page);
	pte_t* pt = get_page_table(pdi);
	size_t pti = index_in_pt(page);

	if (!pd[pdi].present || !pt[pti].present)
		return -EINVAL;

	pt[pti].read_write = (prot & VMM_PROT_WRITE) ? 1 : 0;
	pt[pti].user = (prot & VMM_PROT_USER) ? 1 : 0;

	invlpg(page);

	return 0;
}

// free page tables
void paging_clear_userspace(p_addr_t cr3)
{
	setup_temp_recursive_entry(cr3);

	pde_t* pd = get_temp_page_directory();

	for (size_t i = 0; i < USER_SPACE_PD_ENTRIES; ++i) {
		if (pd[i].present) {
			memory_page_frame_free(pd_addr2p_addr(pd[i].address));
			memset(&pd[i], 0, sizeof(pde_t));
		}
	}

	reset_temp_recursive_entry();
}

#if 0
void paging_dump(void)
{
	pde_t* pd = get_page_directory();

	for (size_t i = 0; i < PAGE_DIRECTORY_ENTRY_COUNT; ++i) {
		if (pd[i].present) {
			log_printf("PD[%d]:[u=%d,w=%d](%p)\n", (int)i, pd[i].user, pd[i].read_write, pd_addr2p_addr(pd[i].address));
			pte_t* pt = get_page_table(i);
			for (size_t j = 0; j < PAGE_TABLE_ENTRY_COUNT; ++j) {
				if (pt[j].present) {
					log_printf("[u=%d,w=%d]virt:%p -> phys:%p\n",
							   pt[j].user,
							   pt[j].read_write,
							   (void*)(v_addr_t)pd_pt_index2v_addr(i, j),
							   (void*)(v_addr_t)pt_addr2p_addr(pt[j].address));
				}
			}
		}
	}

}
#endif
