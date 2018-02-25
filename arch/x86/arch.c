#include <kernel/arch.h>
#include <kernel/memory.h>
#include <kernel/panic.h>
#include <kernel/paging.h>
#include <kernel/kheap.h>
#include <kernel/kernel.h>
#include <kernel/kernel_image.h>
#include <kernel/time/time.h>
#include <kernel/kassert.h>
#include "gdt.h"
#include "idt.h"
#include "exception.h"
#include "irq.h"
#include "i8254.h"
#include "tss.h"

#include <kernel/log.h>

#define TICK_INTERVAL_IN_MS 10

extern uint32_t syscall_handler;

static void default_exception_handler(unsigned int exception)
{
	log_e_printf("\nexception : %d", exception);
	PANIC("exception");
}

int arch_init(void)
{
	idt_init();

	exception_init();

	irq_init();

	gdt_init();

	tss_init();

	for (unsigned int e = 0; e <= EXCEPTION_MAX; e++)
		exception_set_handler_generic(e, default_exception_handler);

	irq_set_handler(IRQ_TIMER, clock_tick);

	time_init((struct time) { .sec = 0, .milli_sec = TICK_INTERVAL_IN_MS,
			.nano_sec = 0 });

	kassert(idt_set_syscall_handler(0x80, (interrupt_handler_t)&syscall_handler) == 0);

	return i8254_set_tick_interval(TICK_INTERVAL_IN_MS);
}

int arch_mm_init(size_t ram_size_bytes)
{
	memory_init(ram_size_bytes);

	paging_init();

	return 0;
}
