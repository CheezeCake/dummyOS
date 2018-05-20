#define ASSEMBLY

#include "irq.h" /* IRQ codes */
#include "interrupt.S" /* common macros */

#define PIC_MASTER	0x20
#define PIC_SLAVE	0xa0

/* see irq.c */
.extern irq_handlers

.macro irq_enter
	pushl $0 /* error code */
	interrupt_enter
	incl irq_level
.endm

.macro irq_leave
	decl irq_level
	jnz 1f

	pushl %esp /* saved cpu_context */
	call sched_schedule
	addl $4, %esp

	cmpl %eax, %esp
	je 1f /* same thread */

	movl %eax, %esp /* switch to next thread context */

	/* update tss */
	pushl %esp
	call cpu_context_update_tss
	addl $4, %esp
1:
	interrupt_leave
.endm


.text

/*
	Master PIC IRQ handlers
*/
.irp nr,			\
	IRQ_TIMER,		\
	IRQ_KEYBOARD,	\
	IRQ_CASCADE		\
	IRQ_COM2,		\
	IRQ_COM1,		\
	IRQ_LPT2,		\
	IRQ_FLOPPY,		\
	IRQ_LPT1

.align 4
asm_irq_handler_\nr:
.type asm_irq_handler_\nr, @function
	irq_enter

	/* send EOI */
	movb $0x20, %al
	outb %al, $PIC_MASTER

	interrupt_call_c_handler \nr, irq_handlers

	irq_leave
.endr

/*
	Slave PIC IRQ hanlders
*/
.irp nr,					\
	IRQ_RTC,				\
	IRQ_UNDEFINED_1,		\
	IRQ_UNDEFINED_2,		\
	IRQ_UNDEFINED_3,		\
	IRQ_PS2_MOUSE,			\
	IRQ_COPROCESSOR,		\
	IRQ_PRIMARY_HARDDISK,	\
	IRQ_SECONDARY_HARDDISK

.align 4
asm_irq_handler_\nr:
.type asm_irq_handler_\nr, @function
	irq_enter

	/* send EOI */
	movb $0x20, %al
	outb %al, $PIC_SLAVE
	outb %al, $PIC_MASTER

	interrupt_call_c_handler \nr, irq_handlers

	irq_leave
.endr

.data

irq_level: .long 0


.section .rodata

.global asm_irq_handlers

.align 4
asm_irq_handlers:
	build_handlers_table asm_irq_handler_, IRQ_MIN, IRQ_MAX
