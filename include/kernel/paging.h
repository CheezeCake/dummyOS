#ifndef _PAGING_H_
#define _PAGING_H_

#include <kernel/types.h>

void paging_init(p_addr_t kernel_top_page_frame);
int paging_map(p_addr_t paddr, v_addr_t vaddr, bool user_page);
int paging_unmap(v_addr_t vaddr);

#endif
