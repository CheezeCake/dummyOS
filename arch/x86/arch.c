#include <kernel/arch.h>
#include <kernel/memory.h>
#include <kernel/panic.h>
#include <kernel/paging.h>
#include <kernel/kmalloc.h>
#include <kernel/kernel.h>
#include <kernel/time.h>
#include <kernel/kernel_image.h>
#include "gdt.h"
#include "idt.h"
#include "exception.h"
#include "irq.h"
#include "i8254.h"

#include <kernel/log.h>

#define TICK_INTERVAL_IN_MS 10
#define MS_IN_NANOSEC 1000000

void page_fault(void)
{
	PANIC("page_fault");
}

void inv(void)
{
	PANIC("inv");
}

void kbd(void)
{
	log_puts("kbd()\n");
}

int arch_init(void)
{
	gdt_init();

	idt_init();

	exception_init();

	irq_init();

	exception_set_handler(EXCEPTION_PAGE_FAULT, page_fault);
	exception_set_handler(EXCEPTION_INVALID_OPCODE, inv);
	irq_set_handler(IRQ_TIMER, clock_tick);
	irq_set_handler(IRQ_KEYBOARD, kbd);

	time_init((struct time) { .sec = 0,
			.nano_sec = MS_IN_NANOSEC * TICK_INTERVAL_IN_MS });

	return i8254_set_tick_interval(TICK_INTERVAL_IN_MS);
}

void arch_memory_management_init(size_t ram_size_bytes)
{
	memory_init(ram_size_bytes);

	p_addr_t kernel_top = kernel_image_get_top_page_frame();

	paging_init(kernel_image_get_base_page_frame(), kernel_top);

	kmalloc_init(kernel_top);
}
