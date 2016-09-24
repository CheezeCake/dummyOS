#include <stddef.h>
#include <stdint.h>
#include <kernel/terminal.h>
#include <kernel/interrupt.h>
#include <kernel/panic.h>
#include "exception.h"
#include "idt.h"
#include "irq.h"

interrupt_handler_t exception_handlers[EXCEPTION_NB] = { NULL, };

int exception_set_handler(unsigned int exception, interrupt_handler_t handler)
{
	if (exception < 0 || exception > EXCEPTION_MAX ||
			handler == NULL)
		return -1;

	// do not change doublefault handler
	if (exception == EXCEPTION_DOUBLE_FAULT)
		return -1;

	disable_irqs();

	int ret = idt_set_handler(EXCEPTION_IDT_INDEX(exception), INTGATE);
	if (ret == 0)
		exception_handlers[exception] = handler;

	enable_irqs();

	return ret;
}

int exception_unset_handler(unsigned int exception)
{
	if (exception < 0 || exception > EXCEPTION_MAX)
		return -1;

	disable_irqs();

	int ret = idt_unset_handler(EXCEPTION_IDT_INDEX(exception));
	if (ret == 0)
		exception_handlers[exception] = NULL;

	enable_irqs();

	return ret;
}

static void doublefault_handler(void)
{
	PANIC("double fault");
}

int exception_init(void)
{
	// setup double fault handler
	return exception_set_handler(EXCEPTION_DOUBLE_FAULT, doublefault_handler);
}
