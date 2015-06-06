#define ASM_SOURCE

#include "exception.h"
/* #include "irq.h" */

.macro SAVE_CONTEXT
	pushal
	subl $2,%esp /* keep the stack aligned */
	pushw %ss
	pushw %ds
	pushw %es
	pushw %fs
	pushw %gs

	movl %esp,%ebp
.endm

.macro RESTORE_CONTEXT
	popw %gs
	popw %fs
	popw %es
	popw %ds
	popw %ss
	addl $2,%esp
	popal
.endm

.set NOP, 0x90

.text

/* defined in exception.c */
.extern exception_handlers

/* defined in irq.c */
/* .extern irq_handlers */


/* No error code exception handlers */
.irp nb, EXCEPTION_DIVIDE_ERROR,			\
	     EXCEPTION_DEBUG,				\
	     EXCEPTION_NMI_INTERRUPT,			\
	     EXCEPTION_BREAKPOINT,			\
	     EXCEPTION_OVERFLOW,			\
	     EXCEPTION_BOUND_RANGE_EXCEDEED,		\
	     EXCEPTION_INVALID_OPCODE,			\
	     EXCEPTION_DEVICE_NOT_AVAILABLE,		\
	     EXCEPTION_COPROCESSOR_SEGMENT_OVERRUN,	\
	     EXCEPTION_RESERVED_1,			\
	     EXCEPTION_FLOATING_POINT_ERROR,		\
	     EXCEPTION_MACHINE_CHECK,			\
	     EXCEPTION_RESERVED_2,			\
	     EXCEPTION_RESERVED_3,			\
	     EXCEPTION_RESERVED_4,			\
	     EXCEPTION_RESERVED_5,			\
	     EXCEPTION_RESERVED_6,			\
	     EXCEPTION_RESERVED_7,			\
	     EXCEPTION_RESERVED_8, 			\
	     EXCEPTION_RESERVED_9, 			\
	     EXCEPTION_RESERVED_10,			\
	     EXCEPTION_RESERVED_11, 			\
	     EXCEPTION_RESERVED_12, 			\
	     EXCEPTION_RESERVED_13, 			\
	     EXCEPTION_RESERVED_14

     .p2align 2, NOP /* align on a 4 byte boundary, fill empty space with NOP */
     asm_exception_handler_\nb:
	.type asm_interrupt_handler_\nb, @function

	SAVE_CONTEXT

	leal exception_handlers, %edi
	call *\nb * 4(%edi) /* call C handler */

	RESTORE_CONTEXT

	iret
.endr

/* Error code exception handlers */
.irp nb, EXCEPTION_DOUBLE_FAULT,		\
	     EXCEPTION_INVALID_TSS,		\
	     EXCEPTION_SEGMENT_NOT_PRESENT,	\
	     EXCEPTION_STACK_SEGMENT_FAULT,	\
	     EXCEPTION_GENERAL_PROTECTION,	\
	     EXCEPTION_PAGE_FAULT,		\
	     EXCEPTION_ALIGNEMENT_CHECK

     .p2align 2, NOP /* align on a 4 byte boundary, fill empty space with NOP */
     asm_exception_handler_\nb:
	.type asm_exception_handler_\nb, @function

	SAVE_CONTEXT

	leal exception_handlers, %edi
	call *\nb * 4(%edi) /* call C handler */

	RESTORE_CONTEXT

	addl $4, %esp /* remove error code from stack */
	iret
.endr


.data

.globl asm_interrupt_handlers

.p2align 5, 0x0
asm_interrupt_handlers:
	.irp nb,  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, \
		17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 29, 30, 31
		.long (asm_exception_handler_\nb)
	.endr
	/* .irp nb, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 */
	/* 	.long (asm_irq_handler_\nb) */
	/* .endr */
