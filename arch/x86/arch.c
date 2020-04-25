#include <kernel/arch.h>
#include <kernel/kassert.h>
#include <kernel/kernel.h>
#include <kernel/kernel_image.h>
#include <kernel/kheap.h>
#include <kernel/kmalloc.h>
#include <kernel/mm/memory.h>
#include <kernel/multiboot.h>
#include <kernel/panic.h>
#include <kernel/time/time.h>
#include <libk/libk.h>
#include "console.h"
#include "exception.h"
#include "gdt.h"
#include "i8254.h"
#include "idt.h"
#include "irq.h"
#include "mm/paging.h"
#include "mm/vmm.h"
#include "tss.h"

#include <kernel/log.h>

#define TICK_INTERVAL_IN_MS 10

void handle_page_fault(int exception, struct cpu_context* ctx);

static void default_exception_handler(int exception, struct cpu_context* ctx)
{
	log_e_printf("\nexception : %d", exception);
	PANIC("exception");
}

static v_addr_t setup_initrd(const multiboot_module_t* mod)
{
	v_addr_t initrd_start = mod->mod_start;
	v_addr_t initrd_end = mod->mod_end;
	size_t initrd_size = initrd_end - initrd_start;
	log_printf("initrd: %p -> %p (%ld)\n", (void*)initrd_start,
		   (void*)initrd_end, initrd_size);

	kassert(is_aligned(initrd_start, PAGE_SIZE));

	v_addr_t initrd = kmalloc_early(align_up(initrd_size, PAGE_SIZE));
	kassert(is_aligned(initrd, PAGE_SIZE));

	memcpy((void*)initrd, (void*)initrd_start, initrd_size);

	return initrd;
}

void __kernel_main(const multiboot_info_t* mbi)
{
	kassert(mbi->mods_count > 0);
	void* initrd = (void*)setup_initrd((const multiboot_module_t*)mbi->mods_addr);

	size_t mem_size = (mbi->mem_upper << 10) + (1 << 20);
	kernel_main(mem_size, (void*)initrd);
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

	struct timespec tick;
	timespec_init(&tick, TICK_INTERVAL_IN_MS);
	time_init(tick);

	kassert(idt_set_syscall_handler(0x80) == 0);

	return i8254_set_tick_interval(TICK_INTERVAL_IN_MS);
}

static const mem_area_t mem_layout[] = {
	MEMORY_AREA(0xa0000, 0x100000, "MMIO"),
};

int arch_mm_init(size_t ram_size_bytes)
{
	memory_init(ram_size_bytes, mem_layout, ARRAY_SIZE(mem_layout));

	vmm_register();

	paging_init();

	return 0;
}

int arch_console_init(void)
{
	return console_init();
}
