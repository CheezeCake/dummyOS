#include <kernel/arch.h>
#include <kernel/cpu.h>
#include <kernel/interrupt.h>
#include <kernel/kassert.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/multiboot.h>
#include <kernel/process.h>
#include <kernel/sched/idle.h>
#include <kernel/sched/sched.h>
#include <kernel/terminal.h>
#include <kernel/thread.h>
#include <kernel/time/time.h>

void clock_tick(void)
{
	time_tick();
	sched_schedule();
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

	sched_init();
	idle_init(); // create idle thread

	struct process user;
	kassert(process_init(&user, "user") == 0);
	sched_add_process(&user);

	sched_start();

	for (;;)
		;
}
