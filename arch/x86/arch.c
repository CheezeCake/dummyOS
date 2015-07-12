#include <kernel/arch.h>
#include <kernel/memory.h>
#include <kernel/panic.h>
#include <kernel/paging.h>
#include <kernel/kmalloc.h>
#include "gdt.h"
#include "idt.h"
#include "exception.h"
#include "irq.h"

void page_fault(void)
{
	PANIC("page_fault");
}

void arch_init(void)
{
	gdt_init();

	idt_init();

	exception_init();

	irq_init();

	exception_set_handler(EXCEPTION_PAGE_FAULT, page_fault);
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
