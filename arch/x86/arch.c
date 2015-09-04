#include <kernel/arch.h>
#include <kernel/memory.h>
#include <kernel/panic.h>
#include <kernel/paging.h>
#include <kernel/kmalloc.h>
#include <kernel/kernel.h>
#include <kernel/time.h>
#include "gdt.h"
#include "idt.h"
#include "exception.h"
#include "irq.h"
#include "i8254.h"

#define CLOCK_FREQUENCY 100 // 100Hz
#define NANOSEC_PER_TICK 10000000

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

	time_init((struct time) { .sec = 0, .nano_sec = NANOSEC_PER_TICK});

	return i8254_set_frequency(CLOCK_FREQUENCY);
}

void arch_memory_management_init(size_t ram_size_bytes)
{
	p_addr_t kernel_top =
		get_kernel_top_page_frame(ram_size_bytes >> PAGE_SIZE_SHIFT);

	if (kernel_top > ram_size_bytes)
		PANIC("insufficient amount of RAM");

	memory_init(ram_size_bytes);

	paging_init(kernel_top);

	kmalloc_init(kernel_top);
}
