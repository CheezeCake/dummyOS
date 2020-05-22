#ifndef _ARCH_PAGING_H_
#define _ARCH_PAGING_H_

#include <kernel/types.h>

p_addr_t paging_virt_to_phys(v_addr_t vaddr);

#endif
