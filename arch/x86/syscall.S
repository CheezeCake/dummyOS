#define ASSEMBLY

#include <kernel/syscall.h>

#include "interrupt.h"
#include "segment.h"

/* see kernel/syscall.c */
.extern syscall_table

.globl syscall_handler

.text

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
	SAVE_CONTEXT()

	/* pass the arguments */
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx

	pushl %eax /* save eax */
	SET_DATA_SEGMENT_REGISTERS(PRIVILEGE_KERNEL, KDATA)
	popl %eax

	leal syscall_table, %edi
	call *(%edi, %eax, 4)

	/* pop the arguments */
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp

	RESTORE_CONTEXT()

	iret
