#include <kernel/arch.h>
#include <kernel/terminal.h>
#include <kernel/cpu.h>
#include <kernel/log.h>
#include <kernel/multiboot.h>
#include <kernel/kassert.h>
#include <kernel/kernel.h>
#include <kernel/time.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <kernel/interrupt.h>

void idle_kthread_do(void)
{
	while (1)
		thread_yield();
}

void clock_tick(void)
{
	irq_disable();

	time_tick();
	sched_schedule();

	irq_enable();
}

void kernel_main(multiboot_info_t* mbi)
{
	terminal_init();

	struct cpu_info* cpu = cpu_info();

	terminal_puts("Welcome to dummyOS!\n");
	terminal_printf("CPU: %s\tRAM: %dMB (0x%x)\n", cpu->cpu_vendor,
			(mbi->mem_upper >> 10) + 1, mbi->mem_upper);

	kassert(arch_init() == 0);
	irq_disable(); // XXX: ?
	arch_memory_management_init((mbi->mem_upper << 10) + (1 << 20));

	sched_init(1000);

	struct thread idle_kthread;
	kassert(thread_create(&idle_kthread, "[idle]", 256, idle_kthread_do, NULL) == 0);
	sched_add_thread(&idle_kthread);

	sched_start();

	for (;;)
		;
}
