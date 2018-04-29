#ifndef _PAGING_H_
#define _PAGING_H_

#include <kernel/types.h>
#include <arch/vm.h>

#define KERNEL_ADDR_SPACE_PAGE_DIRECTORY_ENTRIES \
	(MIRRORING_VADDR_BEGIN / VM_COVERED_PER_PD_ENTRY) // 256

void paging_switch_cr3(p_addr_t cr3);

void paging_init(void);

int paging_init_pd(p_addr_t cr3);

int paging_sync_kernel_space(p_addr_t cr3, void* data);

int paging_clone_current_cow(p_addr_t cr3);

int paging_copy_page(v_addr_t src_page, p_addr_t dst_frame);

int paging_update_prot(v_addr_t page, int prot);


int paging_map(p_addr_t paddr, v_addr_t vaddr, int prot);

int paging_unmap(v_addr_t vaddr);

#endif
