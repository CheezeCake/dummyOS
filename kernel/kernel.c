#include <fs/vfs.h>
#include <kernel/arch.h>
#include <kernel/cpu.h>
#include <kernel/init.h>
#include <kernel/interrupt.h>
#include <kernel/kassert.h>
#include <kernel/kernel.h>
#include <kernel/kernel_image.h>
#include <kernel/kheap.h>
#include <kernel/log.h>
#include <kernel/process.h>
#include <kernel/sched/idle.h>
#include <kernel/sched/sched.h>
#include <kernel/terminal.h>
#include <kernel/thread.h>
#include <kernel/tty.h>
#include <kernel/time/time.h>

#include <fs/ramfs/ramfs.h>

void clock_tick(int nr, struct cpu_context* interrupted_ctx)
{
	time_tick();
	sched_tick(interrupted_ctx);
}

static void mm_init(size_t mem)
{
	kassert(arch_mm_init(mem) == 0);

	if (kheap_init(kernel_image_get_top_page()) < KHEAP_INITIAL_SIZE)
		PANIC("Not enough memory for kernel heap!");
}

static void register_filesystems(void)
{
	kassert(ramfs_init_and_register() == 0);
}

void kernel_main(size_t mem_size)
{
	terminal_init();

	struct cpu_info* cpu = cpu_info();

	terminal_puts("Welcome to dummyOS!\n");
	terminal_printf("CPU: %s\tRAM: %dMB (%p)\n", cpu->cpu_vendor,
			(unsigned int)(mem_size >> 20), (void*)mem_size);


	kassert(arch_init() == 0);
	mm_init(mem_size);

	register_filesystems();
	kassert(vfs_init() == 0);

	sched_init();

	kassert(tty_chardev_init() == 0);
	arch_console_init();

	kassert(init_process_init("/init") == 0); // create init process first (pid 1)
	idle_init(); // then create the idle thread (pid 2)

	sched_start();

	for (;;)
		;
}
