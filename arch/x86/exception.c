#include <kernel/errno.h>
#include <kernel/interrupt.h>
#include <kernel/panic.h>
#include <kernel/terminal.h>
#include <kernel/types.h>
#include "exception.h"
#include "idt.h"
#include "irq.h"

interrupt_handler_t exception_handlers[EXCEPTION_COUNT] = { NULL, };

int exception_set_handler(uint8_t exception, interrupt_handler_t handler)
{
	int err;

	if (exception > EXCEPTION_MAX || !handler)
		return -EINVAL;

	irq_disable();

	err = idt_set_exception_handler_present(exception, INTGATE);
	if (!err)
		exception_handlers[exception] = handler;

	irq_enable();

	return err;
}

int exception_unset_handler(uint8_t exception)
{
	if (exception > EXCEPTION_MAX)
		return -EINVAL;

	irq_disable();

	idt_unset_exception_handler_present(exception);
	exception_handlers[exception] = NULL;

	irq_enable();

	return 0;
}

static void doublefault_handler(int nr, struct cpu_context* interrupted_ctx)
{
	PANIC("double fault");
}

int exception_init(void)
{
	// setup double fault handler
	int err = exception_set_handler(EXCEPTION_DOUBLE_FAULT, doublefault_handler);

	return err;
}
