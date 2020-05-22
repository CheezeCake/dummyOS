#define ASSEMBLY

#include <dummyos/syscall.h>
#include "exception.h"
#include "interrupt.S" /* common macros */

.text

/* see kernel/syscall.c */
.extern syscall_table

.global __syscall_restart

.global syscall_handler
/*
	r0 =  syscall number
	r1 =  arg1
	r2 =  arg2
	r3 =  arg3
	r4 =  arg4
	r5 =  arg5
*/
syscall_handler:
	srsfd sp!, #ARM_MODE_SVC /* save lr and spsr to svc stack */

	cmp r0, #_SYSCALL_NR_TOP
	ble 1f
	mov r0, #-1
	rfefd sp!
1:
	cpu_context_save

__syscall_restart:
	/* update contexts */
	bl sched_get_current_thread
	/* r0 = current_thread */
	mov r1, sp /* r1 = cpu_context */
	mov r4, r0
	mov r5, r1
	bl thread_set_cpu_context
	mov r0, r4 /* current_thread */
	mov r1, r5 /* cpu_context */
	bl thread_set_syscall_context

	/* sp = cpu_ctx -> r0, r1, r2, r3, r4, r5 (syscall args) */

	/* reload syscall arguments */
	add r0, sp, #4 /* r0 -> r1, r2, r3, r4, r5 */
	ldmfd r0, {r0-r5}
	ldr r7, [sp] /* r7 = cpu_ctx->r0 */

	ldr r6, =syscall_table
	ldr r6, [r6, r7, lsl #2]
	blx r6 /* call C handler */

	/* syscall return value */
	str r0, [sp]

	/* set syscall context to NULL */
	bl sched_get_current_thread
	/* r0 = current_thread */
	mov r1, #0 /* cpu_context */
	bl thread_set_syscall_context

	cpu_context_restore
	rfefd sp!


.global __sig_tramp_start
.global __sig_tramp_end

__sig_tramp_start:
	mov r0, #SYS_sigreturn
	swi 0x0
__sig_tramp_end:
