#include "interrupt.S"

.global cpu_context_preempt
.global cpu_context_switch

/* void cpu_context_preempt(void) */
cpu_context_preempt:
	push {r11, lr}
	ldr r2, =return
	mrs r3, cpsr
	push {r2, r3}
	cpu_context_save

	mov r0, sp
	bl sched_schedule_yield

	bl cpu_context_switch

return:
	/* return to caller */
	pop {r11, pc}

/* void cpu_context_switch(const struct cpu_context* to) */
cpu_context_switch:
	mov sp, r0
	cpu_context_restore
	rfefd sp!
