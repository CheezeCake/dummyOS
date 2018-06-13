#define ASSEMBLY

#include <kernel/syscall.h>
#include "interrupt.S" /* common macros */

.text

/* see kernel/syscall.c */
.extern syscall_table

.global syscall_handler
.type syscall_handler, @function
/*
	eax =  syscall number
	ebx =  arg1
	ecx =  arg2
	edx =  arg3
	esi =  arg4
	edi =  arg5
	ebp =  arg6
*/
syscall_handler:
	/* check if syscall number is valid */
	cmpl $SYSCALL_NR_TOP, %eax
	jle 1f
	movl $-1, %eax
	iret

1:
	pushl $0 /* error code */
	interrupt_enter

	/* update syscall context */
	movl %esp, %ebx
	call sched_get_current_thread
	pushl %ebx
	pushl %eax /* current_thread */
	call thread_set_syscall_context
	addl $8, %esp

	/* esp = cpu_ctx -> eax, ebx, ecx, edx, esi, edi, ebp */
	popl %eax
	/* esp = cpu_ctx + 4 -> ebx, ecx, edx, esi, edi, ebp  (syscall args) */

	leal syscall_table, %edi
	call *(%edi, %eax, 4) /* call C handler */

	/* syscall return value */
	/* override cpu_ctx.eax and restore esp at the same time */
	pushl %eax
	/* esp = cpu_ctx */

	interrupt_leave


.global __sig_tramp_start
.global __sig_tramp_end

__sig_tramp_start:
	movl $NR_sigreturn, %eax
	int $0x80
__sig_tramp_end:
