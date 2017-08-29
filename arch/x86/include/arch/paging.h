#ifndef _ARCH_PAGING_H_
#define _ARCH_PAGING_H_

#include <stdbool.h>

#include <kernel/types.h>
#include <arch/virtual_memory.h>

#define KERNEL_ADDR_SPACE_PAGE_DIRECTORY_ENTRIES \
	(MIRRORING_VADDR_BEGIN / VM_COVERED_PER_PD_ENTRY) // 256

void paging_switch_cr3(p_addr_t cr3, bool init_userspace);

#endif