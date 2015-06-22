#include <kernel/arch.h>
#include "gdt.h"
#include "idt.h"
#include "exception.h"
#include "irq.h"

void arch_init(void)
{
	gdt_init();

	idt_init();

	exception_init();

	irq_init();
}
