#ifndef _KERNEL_CPU_CONTEXT_H_
#define _KERNEL_CPU_CONTEXT_H_

#include <arch/cpu_context.h>
#include <kernel/types.h>

void cpu_context_create(struct cpu_context* cpu_context, v_addr_t stack_top, v_addr_t ip);

#endif
