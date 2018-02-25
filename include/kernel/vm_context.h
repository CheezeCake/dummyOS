#ifndef _KERNEL_VM_CONTEXT_H_
#define _KERNEL_VM_CONTEXT_H_

#include <arch/vm_context.h>

int vm_context_create(struct vm_context* vm_context);
int vm_context_init(struct vm_context* vm_context);
void vm_context_destroy(struct vm_context* vm_context);
void vm_context_switch(struct vm_context* vm_context);
int vm_context_clear_userspace(struct vm_context* vm_context);

#endif
