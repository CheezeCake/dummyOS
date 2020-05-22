#include <fs/tty.h>
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
#include <kernel/sched/reaper.h>
#include <kernel/sched/sched.h>
#include <kernel/terminal.h>
#include <kernel/thread.h>
#include <kernel/time/time.h>

#include <fs/ramfs/ramfs.h>

static void register_filesystems(void)
{
	kassert(ramfs_init_and_register() == 0);
}

static int mount_initrd(void* initrd)
{
	return vfs_mount(NULL, vfs_cache_node_get_root(), initrd, "ramfs");
}

void kernel_main(void* initrd)
{
	kassert(arch_init() == 0);

	struct cpu_info* cpu = cpu_info();
	size_t mem_size = arch_get_mem_size();

	terminal_init();

	terminal_puts("Welcome to dummyOS!\n");
	terminal_printf("CPU: %s\tRAM: %dMB (%p)\n", cpu->cpu_vendor,
			(unsigned int)(mem_size >> 20), (void*)mem_size);


	register_filesystems();
	kassert(vfs_init() == 0);
	kassert(mount_initrd(initrd) == 0);

	kassert(tty_chardev_init() == 0);
	arch_console_init();

	sched_init();
	reaper_init();
	idle_init();
	kassert(init_process_init("/init") == 0); // create init process first (pid 1)

	sched_start();

	for (;;)
		;
}
