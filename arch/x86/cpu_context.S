#define ASSEMBLY

#include "interrupt.S" /* common macros */

.text

/* void cpu_context_preempt(void) */
.global cpu_context_preempt
.type cpu_context_preempt, @function
cpu_context_preempt:
	pushfl
	pushl %cs
	pushl $return
	pushl $0 /* error code */
	cpu_context_save

	pushl %esp /* saved cpu_context */
	call sched_schedule_yield

	pushl %eax
	call cpu_context_switch

return:
	/* return to caller */
	ret

/* void cpu_context_switc(const struct cpu_context* to) */
.global cpu_context_switch
.type cpu_context_switch, @function
cpu_context_switch:
	movl 4(%esp), %esp /* switch to next thread context */

	/* update tss */
	pushl %esp
	call cpu_context_update_tss
	addl $4, %esp

	cpu_context_restore
	iret
