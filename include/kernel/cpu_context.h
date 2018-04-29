#ifndef _KERNEL_CPU_CONTEXT_H_
#define _KERNEL_CPU_CONTEXT_H_

#include <kernel/types.h>

struct cpu_context;

void cpu_context_kernel_init(struct cpu_context* cpu_context, v_addr_t pc);

void cpu_context_user_init(struct cpu_context* cpu_context, v_addr_t user_pc,
						   v_addr_t user_stack);

bool cpu_context_is_usermode(const struct cpu_context* cpu_context);

size_t cpu_context_sizeof(void);

void cpu_context_switch(const struct cpu_context* to);

void cpu_context_preempt(void);

void cpu_context_set_syscall_return_value(struct cpu_context* cpu_context,
										  int ret);

#endif
