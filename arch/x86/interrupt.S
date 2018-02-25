#define ASSEMBLY

#include "exception.h"
#include "interrupt.h"
#include "irq.h"
#include "segment.h"

.macro PROLOGUE
	pushl %ebp
	movl %esp, %ebp
	SAVE_CONTEXT()
	/* setup kernel segment register values */
	SET_DATA_SEGMENT_REGISTERS(PRIVILEGE_KERNEL, KDATA)
.endm

.macro EPILOGUE
	RESTORE_CONTEXT()
	popl %ebp
.endm

#define PIC_MASTER	0x20
#define PIC_SLAVE	0xa0

.text

/* see exception.c */
.extern exception_handlers

/* see irq.c */
.extern irq_handlers


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

     asm_exception_handler_\nb:
	.type asm_interrupt_handler_\nb, @function

	PROLOGUE

	pushl $\nb

	leal exception_handlers, %edi
	call *\nb * 4(%edi) /* call C handler */

	addl $4, %esp

	/* EPILOGUE */

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

     asm_exception_handler_\nb:
	.type asm_exception_handler_\nb, @function

	addl $4, %esp /* remove error code from stack */

	PROLOGUE

	pushl $\nb

	leal exception_handlers, %edi
	call *\nb * 4(%edi) /* call C handler */

	addl $4, %esp

	EPILOGUE

	iret
.endr


/* Master PIC IRQ handlers */
.irp nb, IRQ_TIMER,		\
	     IRQ_KEYBOARD	\
	     IRQ_CASCADE	\
	     IRQ_COM2		\
	     IRQ_COM1		\
	     IRQ_LPT2		\
	     IRQ_FLOPPY		\
	     IRQ_LPT1

	asm_irq_handler_\nb:
		.type asm_irq_handler_\nb, @function

		PROLOGUE

		/* send EOI */
		movb $0x20, %al
		outb %al, $PIC_MASTER

		leal irq_handlers, %edi
		call *\nb * 4(%edi) /* call C handler */

		EPILOGUE

		iret
.endr

/* Slave PIC IRQ hanlders */
.irp nb, IRQ_RTC			\
	     IRQ_UNDEFINED_1		\
	     IRQ_UNDEFINED_2		\
	     IRQ_UNDEFINED_3		\
	     IRQ_PS2_MOUSE		\
	     IRQ_COPROCESSOR		\
	     IRQ_PRIMARY_HARDDISK	\
	     IRQ_SECONDARY_HARDDISK

	asm_irq_handler_\nb:
		.type asm_irq_handler_\nb, @function

		PROLOGUE

		/* send EOI */
		movb $0x20, %al
		outb %al, $PIC_SLAVE
		outb %al, $PIC_MASTER

		leal irq_handlers, %edi
		call *\nb * 4(%edi) /* call C handler */

		EPILOGUE

		iret
.endr


.data

.globl asm_interrupt_handlers

asm_interrupt_handlers:
	/* indexes 0 -> 31: asm exception hanlders */
	.irp nb,  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, \
		17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
		.long (asm_exception_handler_\nb)
	.endr
	/* indexes 32 -> 47: asm irq hanlders */
	.irp nb,  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
		.long (asm_irq_handler_\nb)
	.endr
