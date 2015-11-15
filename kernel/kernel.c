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

void onche(void)
{
	unsigned int i = 200 * 1000;
	while (i--)
		terminal_puts("o");
}

void ponche(void)
{
	/* while (1) */
	/* 	terminal_puts("p"); */
	unsigned int i = 500 * 1000;
	while (i--)
		terminal_puts("p");
}

void exit(void)
{
	log_puts("########## EXIT ##########\n");
	sched_remove_current_thread();
}

void idle_kthread_do(void)
{
	while (1)
		;
}

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
	terminal_printf("CPU: %s\tRAM: %dMB (0x%x)", cpu->cpu_vendor,
			(mbi->mem_upper >> 10) + 1, mbi->mem_upper);

	kassert(arch_init() == 0);
	arch_memory_management_init((mbi->mem_upper << 10) + (1 << 20));

	sched_init(1000);

	struct thread idle_kthread;
	kassert(thread_create(&idle_kthread, "[idle]", 256, idle_kthread_do, NULL) == 0);
	sched_add_thread(&idle_kthread);

	struct thread t1;
	thread_create(&t1, "[onche]", 1024, onche, exit);
	struct thread t2;
	thread_create(&t2, "[ponche]", 1024, ponche, exit);

	sched_add_thread(&t1);
	sched_add_thread(&t2);

	sched_start();

	for (;;)
		;
}
