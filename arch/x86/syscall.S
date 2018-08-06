#define ASSEMBLY

#include <dummyos/syscall.h>
#include "interrupt.S" /* common macros */

.text

/* see kernel/syscall.c */
.extern syscall_table

.global __syscall_restart

.global syscall_handler
.type syscall_handler, @function
/*
	eax =  syscall number
	ebx =  arg1
	ecx =  arg2
	edx =  arg3
	esi =  arg4
	edi =  arg5
*/
syscall_handler:
	/* check if syscall number is valid */
	cmpl $_SYSCALL_NR_TOP, %eax
	jle 1f
	movl $-1, %eax
	iret

1:
	pushl $0 /* error code */
	interrupt_enter

__syscall_restart:
	/* update contexts */
	call sched_get_current_thread
	pushl %esp /* cpu_context */
	pushl %eax /* current_thread */
	call thread_set_cpu_context
	call thread_set_syscall_context
	addl $8, %esp

	/* esp = cpu_ctx -> ebx, ecx, edx, esi, edi (syscall args) */

	movl 24(%esp), %eax /* eax = cpu_ctx->eax */

	leal syscall_table, %edi
	call *(%edi, %eax, 4) /* call C handler */

	/* syscall return value */
	movl %eax, 24(%esp) /* cpu_ctx->eax = eax */

	/* set syscall context to NULL */
	call sched_get_current_thread
	pushl $0 /* cpu_context */
	pushl %eax /* current_thread */
	call thread_set_syscall_context
	addl $8, %esp

	/* update tss */
	pushl %esp
	call cpu_context_update_tss
	addl $4, %esp

	interrupt_leave


.global __sig_tramp_start
.global __sig_tramp_end

__sig_tramp_start:
	movl $SYS_sigreturn, %eax
	int $0x80
__sig_tramp_end:
