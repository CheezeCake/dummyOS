#include <stdbool.h>
#include <kernel/cpu_context.h>
#include <kernel/libk.h>
#include "gdt.h"
#include "segment.h"

void cpu_context_create(struct cpu_context* cpu_context, v_addr_t stack_top, v_addr_t ip)
{
	memset(cpu_context, 0, sizeof(struct cpu_context));
	cpu_context->esp = stack_top;
	cpu_context->ebp = stack_top;
	cpu_context->eip = ip;

	cpu_context->cs = make_segment_register(0, false, KCODE);
	cpu_context->ds = make_segment_register(0, false, KDATA);
	cpu_context->es = make_segment_register(0, false, KDATA);
	cpu_context->fs = make_segment_register(0, false, KDATA);
	cpu_context->gs = make_segment_register(0, false, KDATA);
	cpu_context->ss = make_segment_register(0, false, KDATA);

	cpu_context->eflags = (1 << 9); // IF = 1
}
