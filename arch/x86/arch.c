#include <kernel/arch.h>
#include <kernel/console.h>
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
#include "drivers/keyboard.h"
#include "exception.h"
#include "gdt.h"
#include "i8254.h"
#include "idt.h"
#include "irq.h"
#include "mm/memory.h"
#include "mm/paging.h"
#include "mm/vmm.h"
#include "tss.h"

#include "drivers/ioport_0xe9.h"

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
	log_printf("initrd copied to %p\n", (void*)initrd);

	memcpy((void*)initrd, (void*)initrd_start, initrd_size);

	return initrd;
}

static size_t mem_size_bytes = 0;

size_t arch_get_mem_size(void)
{
	return mem_size_bytes;
}

static const struct log_ops x86_log_ops = {
	.puts		= ioport_0xe9_puts,
	.putchar	= ioport_0xe9_putchar,
};

void __kernel_main(const multiboot_info_t* mbi)
{
	kassert(mbi->mods_count > 0);
	void* initrd = (void*)setup_initrd((const multiboot_module_t*)mbi->mods_addr);

	log_register(&x86_log_ops);

	mem_size_bytes = (mbi->mem_upper << 10) + (1 << 20);
	kernel_main((void*)initrd);
}

static void clock_tick(int nr, struct cpu_context* interrupted_ctx)
{
	time_tick();
}

int arch_console_init(void)
{
	int err;
	struct tty* console_tty;

	err = console_init(terminal_putchar, &console_tty);
	if (!err)
		err = keyboard_init(console_tty);

	return err;
}

static const mem_area_t mem_layout[] = {
	MEMORY_AREA(X86_MEMORY_HARDWARE_MAP_START,
		    X86_MEMORY_HARDWARE_MAP_SIZE,
		    0,
		    "MMIO"),
};

static int arch_mm_init()
{
	kassert(mem_size_bytes > 0);

	paging_init();

	__memory_early_init();

	x86_vmm_register();

	if (kheap_init(kernel_image_get_top_page()) < KHEAP_INITIAL_SIZE)
		PANIC("Not enough memory for kernel heap!");

	memory_init(mem_size_bytes, mem_layout, ARRAY_SIZE(mem_layout));

	return 0;
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

	kassert(i8254_set_tick_interval(TICK_INTERVAL_IN_MS) == 0);

	return arch_mm_init();
}
