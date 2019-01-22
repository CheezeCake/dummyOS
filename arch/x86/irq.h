#ifndef _IRQ_H_
#define _IRQ_H_

#ifndef ASSEMBLY
#include <kernel/interrupt.h>
#include <kernel/types.h>
#endif // ! ASSEMBLY


/*
 * IRQs
 */
#define IRQ_TIMER		0
#define IRQ_KEYBOARD		1
#define IRQ_CASCADE		2
#define IRQ_COM2		3
#define IRQ_COM1		4
#define IRQ_LPT2		5
#define IRQ_FLOPPY		6
#define IRQ_LPT1		7
#define IRQ_RTC			8
#define IRQ_UNDEFINED_1		9
#define IRQ_UNDEFINED_2		10
#define IRQ_UNDEFINED_3		11
#define IRQ_PS2_MOUSE		12
#define IRQ_COPROCESSOR		13
#define IRQ_PRIMARY_HARDDISK	14
#define IRQ_SECONDARY_HARDDISK	15



#define IRQ_MIN		0
#define IRQ_MAX		15
#define IRQ_COUNT	(IRQ_MAX + 1)

#ifndef ASSEMBLY

int irq_set_handler(uint8_t irq, interrupt_handler_t handler);

int irq_unset_handler(uint8_t irq);

void irq_init(void);

#endif // ! ASSEMBLY

#endif
