#include <kernel/arch.h>
#include <kernel/memory.h>
#include <kernel/panic.h>
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

void arch_memory_management_init(size_t ram_size_bytes)
{
	p_addr_t kernel_top =
		get_kernel_top_page_frame(ram_size_bytes >> PAGE_SIZE_SHIFT);

	if (kernel_top > ram_size_bytes)
		PANIC("insufficient amount of RAM");

	memory_init(ram_size_bytes);
}
