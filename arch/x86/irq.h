#ifndef _IRQ_H_
#define _IRQ_H_

#ifndef ASM_SOURCE
#include <stdint.h>
#include "interrupt.h"
#endif

/*
 * IRQs
 */
#define IRQ_TIMER 0
#define IRQ_KEYBOARD 1
#define IRQ_CASCADE 2
#define IRQ_COM2 3
#define IRQ_COM1 4
#define IRQ_LPT2 5
#define IRQ_FLOPPY 6
#define IRQ_LPT1 7
#define IRQ_RTC 8
#define IRQ_UNDEFINED_1 9
#define IRQ_UNDEFINED_2 10
#define IRQ_UNDEFINED_3 11
#define IRQ_PS2_MOUSE 12
#define IRQ_COPROCESSOR 13
#define IRQ_PRIMARY_HARDDISK 14
#define IRQ_SECONDARY_HARDDISK 15


#ifndef ASM_SOURCE

#define IRQ_IDT_INDEX(irq) (IRQ_BASE + irq)

static inline void disable_irqs(uint32_t* flags)
{
	// save EFLAGS register to flags
	__asm__ __volatile__ (
			"pushfl\n"
			"popl %0\n"
			: "=g" (*flags)
			:
			: "memory");

	__asm__ ("cli\n");
}

static inline void restore_irqs(uint32_t flags)
{
	// restore EFLAGS register
	__asm__ __volatile__ (
			"pushl %0\n"
			"popfl\n"
			:
			: "g" (flags)
			: "memory");
}


int irq_set_handler(uint8_t irq, interrupt_handler_t handler);
int irq_unset_handler(uint8_t irq);
void irq_init(void);

#endif

#endif
