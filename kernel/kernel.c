#include <fs/vfs.h>
#include <kernel/arch.h>
#include <kernel/cpu.h>
#include <kernel/interrupt.h>
#include <kernel/kassert.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/multiboot.h>
#include <kernel/process.h>
#include <kernel/sched/sched.h>
#include <kernel/terminal.h>
#include <kernel/thread.h>
#include <kernel/time/time.h>

#include <fs/ramfs/ramfs.h>

void clock_tick(void)
{
	time_tick();
	sched_schedule();
}

static void register_filesystems(void)
{
	kassert(ramfs_init_and_register() == 0);
}

void kernel_main(multiboot_info_t* mbi)
{
	terminal_init();

	struct cpu_info* cpu = cpu_info();

	terminal_puts("Welcome to dummyOS!\n");
	terminal_printf("CPU: %s\tRAM: %dMB (0x%x)\n", cpu->cpu_vendor,
			(mbi->mem_upper >> 10) + 1, mbi->mem_upper);

	kassert(arch_init() == 0);
	arch_memory_management_init((mbi->mem_upper << 10) + (1 << 20));

	register_filesystems();
	kassert(vfs_init() == 0);

	process_init();
	sched_init();

	struct process user;
	kassert(process_uprocess_create(&user, "user") == 0);
	sched_add_process(&user);

	sched_start();

	for (;;)
		;
}
