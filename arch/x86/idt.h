#ifndef _IDT_H_
#define _IDT_H_

#include <kernel/interrupt.h>
#include <kernel/types.h>
#include "segment.h"

#define EXCEPTION_BASE			0 // base index in IDT
#define EXCEPTION_IDT_INDEX(exception)	(EXCEPTION_BASE + exception)

#define IRQ_BASE		32 // base index in IDT
#define IRQ_IDT_INDEX(irq)	(IRQ_BASE + irq)

#define INTERRUPT_MAX		256
#define INTERRUPTS_DEFINED	IRQ_IDT_INDEX(IRQ_MAX)

#define IDT_SIZE INTERRUPT_MAX

enum gate_type
{
	TASKGATE = 0x5, // 0b101 [unused]
	INTGATE = 0x6, // 0b110
	TRAPGATE = 0x7 // 0b111
};

int idt_set_exception_handler_present(uint8_t exception, enum gate_type type);

int idt_set_irq_handler_present(uint8_t irq, enum gate_type type);

void idt_unset_exception_handler_present(uint8_t exception);

void idt_unset_irq_handler_present(uint8_t irq);

int idt_set_syscall_handler(uint8_t int_number);

void idt_init(void);

#endif
