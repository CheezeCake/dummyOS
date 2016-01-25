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

void page_fault(void)
{
	PANIC("page_fault");
}

int arch_init(void)
{
	gdt_init();

	idt_init();

	exception_init();

	irq_init();

	exception_set_handler(EXCEPTION_PAGE_FAULT, page_fault);
	irq_set_handler(IRQ_TIMER, clock_tick);

	time_init((struct time) { .sec = 0, .milli_sec = TICK_INTERVAL_IN_MS,
			.nano_sec = 0 });

	return i8254_set_tick_interval(TICK_INTERVAL_IN_MS);
}

void arch_memory_management_init(size_t ram_size_bytes)
{
	memory_init(ram_size_bytes);

	p_addr_t kernel_top = kernel_image_get_top_page_frame();

	paging_init(kernel_image_get_base_page_frame(), kernel_top);

	kmalloc_init(kernel_top);
}
