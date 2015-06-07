#include <stddef.h>
#include <stdint.h>
#include "irq.h"
#include "i8259.h"
#include "idt.h"
#include "interrupt.h"

interrupt_handler_t irq_handlers[IRQ_NB] = { NULL, };

int irq_set_handler(uint8_t irq, interrupt_handler_t handler)
{
	if (irq < 0 || irq > IRQ_MAX ||
			handler == 0)
		return -1;

	disable_irqs();

	int ret = idt_set_handler(IRQ_BASE + irq, INTGATE);

	if (ret == 0) {
		irq_handlers[irq] = handler;
		i8259_irq_enable(irq); // update PIC
	}

	enable_irqs();

	return ret;
}

int irq_unset_handler(uint8_t irq)
{
	if (irq < 0 || irq > IRQ_MAX)
		return -1;

	disable_irqs();

	int ret = idt_unset_handler(IRQ_BASE + irq);

	if (ret == 0) {
		irq_handlers[irq] = NULL;
		i8259_irq_disable(irq); // update PIC
	}

	enable_irqs();

	return ret;
}

void irq_init(void)
{
	// init i8259 PIC
	i8259_init();
}
