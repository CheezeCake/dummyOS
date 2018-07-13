#include <dummyos/errno.h>
#include <kernel/interrupt.h>
#include <kernel/types.h>
#include "irq.h"
#include "i8259.h"
#include "idt.h"

interrupt_handler_t irq_handlers[IRQ_COUNT] = { NULL, };

int irq_set_handler(uint8_t irq, interrupt_handler_t handler)
{
	int err;

	if (irq > IRQ_MAX || !handler)
		return -EINVAL;

	irq_disable();

	err = idt_set_irq_handler_present(irq, INTGATE);

	if (!err) {
		irq_handlers[irq] = handler;
		i8259_irq_enable(irq); // update PIC
	}

	irq_enable();

	return err;
}

int irq_unset_handler(uint8_t irq)
{
	if (irq > IRQ_MAX)
		return -EINVAL;

	irq_disable();

	idt_unset_irq_handler_present(irq);
	irq_handlers[irq] = NULL;
	i8259_irq_disable(irq); // update PIC

	irq_enable();

	return 0;
}

void irq_init(void)
{
	// init i8259 PIC
	i8259_init();
}
