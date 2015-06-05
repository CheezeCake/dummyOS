#include <stddef.h>
#include <stdint.h>
#include <kernel/terminal.h>
#include "exception.h"
#include "interrupt.h"
#include "idt.h"

interrupt_handler_t exception_handlers[EXCEPTION_NB] = { NULL, };

int exception_set_handler(unsigned int exception, interrupt_handler_t handler)
{
	if (exception < 0 || exception > EXCEPTION_MAX ||
			!handler)
		return -1;

	// return -1 for double fault

	// disable IRQs

	exception_handlers[exception] = handler;
	int ret = idt_set_handler(EXCEPTION_BASE + exception, INTGATE);

	if (ret == -1)
		exception_handlers[exception] = NULL;

	// restorre IRQs

	return ret;
}

int exception_unset_handler(unsigned int exception)
{
	if (exception < 0 || exception > EXCEPTION_MAX)
		return -1;

	int ret = idt_unset_handler(EXCEPTION_BASE + exception);
	if (ret == 0)
		exception_handlers[exception] = NULL;

	return ret;
}

void doublefault_handler(void)
{
	terminal_puts("PANIC: double fault");
	__asm__ ("l: hlt\n"
			"jmp l\n");
}

int init_exception(void)
{
	// setup double fault handler
	return exception_set_handler(EXCEPTION_DOUBLE_FAULT, doublefault_handler);
}
