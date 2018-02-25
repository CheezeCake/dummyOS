#include <kernel/context_switch.h>
#include <kernel/vm_context.h>
#include <kernel/process.h>
#include <arch/cpu_context.h>
#include "tss.h"

void context_switch(const struct thread* from, const struct thread* to)
{
	struct cpu_context* from_cpu_ctx = (from) ? from->cpu_context : NULL;

	if (to->type == UTHREAD)
		tss_update(to->kstack.sp + to->kstack.size);

	if (!from || from->process != to->process)
		vm_context_switch(&to->process->vm_ctx);

	cpu_context_switch(from_cpu_ctx, to->cpu_context);
}
