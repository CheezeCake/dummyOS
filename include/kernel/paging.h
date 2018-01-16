#ifndef _KERNEL_PAGING_H_
#define _KERNEL_PAGING_H_

#include <kernel/types.h>

// read and write: ELF values
#define VM_OPT_WRITE	(1 << 1)
#define VM_OPT_READ		(1 << 2)

#define VM_OPT_USER		(1 << 3)

void paging_init(void);
int paging_map(p_addr_t paddr, v_addr_t vaddr, uint8_t flags);
int paging_unmap(v_addr_t vaddr);

#endif
