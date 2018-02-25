#include <kernel/interrupt.h>
#include <kernel/types.h>
#include "irq.h"
#include "i8259.h"
#include "idt.h"

interrupt_handler_t irq_handlers[IRQ_NB] = { NULL, };

int irq_set_handler(uint8_t irq, interrupt_handler_t handler)
{
	if (irq < 0 || irq > IRQ_MAX || handler == NULL)
		return -1;

	irq_state_t state;
	irq_save_state(state);
	irq_disable();

	int ret = idt_set_handler(IRQ_IDT_INDEX(irq), INTGATE);

	if (ret == 0) {
		irq_handlers[irq] = handler;
		i8259_irq_enable(irq); // update PIC
	}

	irq_restore_state(state);

	return ret;
}

int irq_unset_handler(uint8_t irq)
{
	if (irq < 0 || irq > IRQ_MAX)
		return -1;

	irq_state_t state;
	irq_save_state(state);
	irq_disable();

	idt_unset_handler(IRQ_IDT_INDEX(irq));
	irq_handlers[irq] = NULL;
	i8259_irq_disable(irq); // update PIC

	irq_restore_state(state);

	return 0;
}

void irq_init(void)
{
	// init i8259 PIC
	i8259_init();
}
