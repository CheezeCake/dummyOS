#ifndef _ARCH_VM_CONTEXT_H_
#define _ARCH_VM_CONTEXT_H_

#include <stdbool.h>

#include <kernel/types.h>
#include <arch/virtual_memory.h>

#define USER_ADDR_SPACE_PAGE_DIRECTORY_ENTRIES \
	(USER_VADDR_SPACE_SIZE / VM_COVERED_PER_PD_ENTRY) // 767

struct vm_context
{
	p_addr_t cr3;
	bool init;

	unsigned int page_tables_page_frame_refs[USER_ADDR_SPACE_PAGE_DIRECTORY_ENTRIES];
};

#endif
