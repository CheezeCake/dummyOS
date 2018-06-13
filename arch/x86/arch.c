#include <kernel/arch.h>
#include <kernel/kassert.h>
#include <kernel/kernel.h>
#include <kernel/kernel_image.h>
#include <kernel/kheap.h>
#include <kernel/mm/memory.h>
#include <kernel/multiboot.h>
#include <kernel/panic.h>
#include <kernel/time/time.h>
#include "console.h"
#include "exception.h"
#include "gdt.h"
#include "i8254.h"
#include "idt.h"
#include "irq.h"
#include "tss.h"
#include "mm/paging.h"
#include "mm/vmm.h"

#include <kernel/log.h>

#define TICK_INTERVAL_IN_MS 10

void handle_page_fault(int exception, struct cpu_context* ctx);

static void default_exception_handler(int exception, struct cpu_context* ctx)
{
	log_e_printf("\nexception : %d", exception);
	PANIC("exception");
}

void __kernel_main(const multiboot_info_t* mbi)
{
	size_t mem_size = (mbi->mem_upper << 10) + (1 << 20);
	log_printf("MEM:%p\n", (void*)(mbi->mem_upper));

	kernel_main(mem_size);
}

int arch_init(void)
{
	idt_init();

	exception_init();

	irq_init();

	gdt_init();

	tss_init();

	for (unsigned int e = 0; e <= EXCEPTION_MAX; e++)
		exception_set_handler(e, default_exception_handler);

	irq_set_handler(IRQ_TIMER, clock_tick);
	exception_set_handler(EXCEPTION_PAGE_FAULT, handle_page_fault);

	time_init((struct time) { .sec = 0, .milli_sec = TICK_INTERVAL_IN_MS,
			.nano_sec = 0 });

	kassert(idt_set_syscall_handler(0x80) == 0);

	return i8254_set_tick_interval(TICK_INTERVAL_IN_MS);
}

int arch_mm_init(size_t ram_size_bytes)
{
	memory_init(ram_size_bytes);

	vmm_register();

	paging_init();

	return 0;
}

int arch_console_init(void)
{
	return console_init();
}
