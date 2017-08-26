#ifndef _KERNEL_VM_CONTEXT_H_
#define _KERNEL_VM_CONTEXT_H_

#include <arch/vm_context.h>

int vm_context_create(struct vm_context* vm_context);

void vm_context_switch(struct vm_context* vm_context);

#endif
