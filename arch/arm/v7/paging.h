#ifndef _PAGING_H_
#define _PAGING_H_

#define LV1_ENTRIES	4
#define LV2_ENTRIES	512
#define LV3_ENTRIES	512

#define DESC_BLOCK	1
#define DESC_TABLE	3
#define DESC_PAGE	3

// memory attributes
#define AF		(1 << 10) // access flag

// access permissions [7:6]
#define AP_KERNEL	(0 << 6)
#define AP_USER		(1 << 6)
#define AP_RW		(0 << 7)
#define AP_RO		(1 << 7)

// attributes index
#define ATTR_MEM	(0 << 2)
#define ATTR_DEV	(1 << 2)


#ifndef ASSEMBLY
#include <arch/paging.h>
#include <kernel/types.h>

int paging_init(void);

void paging_switch_ttbr0(p_addr_t ttbr0);

int paging_clone_current_cow(uint64_t* dst_lv1, uint64_t* src_lv1);

int paging_clear_userspace(uint64_t* lv1_table);

int paging_copy_page(v_addr_t src_page, p_addr_t dst_frame);

int paging_update_kernel_prot(v_addr_t page, int prot);

int paging_kernel_map(p_addr_t paddr, v_addr_t vaddr, int prot);

int paging_kernel_unmap(v_addr_t vaddr);

int paging_update_user_prot(v_addr_t page, int prot, uint64_t* ttbr0);

int paging_user_map(p_addr_t paddr, v_addr_t vaddr, int prot, uint64_t* ttbr0);

int paging_user_unmap(v_addr_t vaddr, uint64_t* ttbr0);

#endif // !ASSEMBLY

#endif
