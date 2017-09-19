#include "gdt.h"
#include "segment.h"
#include <kernel/cpu_context.h>
#include <libk/libk.h>

void cpu_context_create(struct cpu_context* cpu_context, v_addr_t stack_top,
		v_addr_t ip)
{
	memset(cpu_context, 0, sizeof(struct cpu_context));
	cpu_context->esp = stack_top;
	cpu_context->ebp = stack_top + 1;
	cpu_context->eip = ip;

	const uint16_t data_segment = make_segment_selector(PRIVILEGE_KERNEL, KDATA);
	cpu_context->cs = make_segment_selector(PRIVILEGE_KERNEL, KCODE);
	cpu_context->ds = data_segment;
	cpu_context->es = data_segment;
	cpu_context->fs = data_segment;
	cpu_context->gs = data_segment;
	cpu_context->ss = data_segment;

	cpu_context->eflags = (1 << 9) | (1 << 1); // IF = 1, reserved_1 = 1
}

void cpu_context_pass_arg(struct cpu_context* cpu_context, unsigned int arg_nb,
		v_addr_t arg)
{
	v_addr_t* arg_stack_addr = (v_addr_t*)cpu_context->esp - 1;
	*arg_stack_addr = arg;
	cpu_context->esp = (v_addr_t)arg_stack_addr;
}
