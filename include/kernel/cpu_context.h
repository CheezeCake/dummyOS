#ifndef _KERNEL_CPU_CONTEXT_H_
#define _KERNEL_CPU_CONTEXT_H_

#include <arch/cpu_context.h>
#include <kernel/types.h>

void cpu_context_create(struct cpu_context* cpu_context, v_addr_t stack_top,
		v_addr_t ip);
void cpu_context_pass_arg(struct cpu_context* cpu_context, unsigned int arg_nb,
		v_addr_t arg);
void cpu_context_set_ret_ip(struct cpu_context* cpu_context, v_addr_t ret_ip);

#endif
