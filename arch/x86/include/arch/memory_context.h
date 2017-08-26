#ifndef _ARCH_VM_CONTEXT_H_
#define _ARCH_VM_CONTEXT_H_

#include <kernel/types.h>

struct vm_context
{
	p_addr_t cr3;
};

#endif
