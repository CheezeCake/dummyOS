#include <kernel/kassert.h>
#include <kernel/process.h>
#include <kernel/sched/idle.h>
#include <kernel/sched/sched.h>

static int test(void* args)
{
	/* return; */
	while (1) {
		terminal_putchar('t');
		thread_sleep(500);
	}

	/* log_e_print("EXIT\n"); */
	/* sched_remove_current_thread(); */
	return 0;
}

static int idle_kthread_do(void* args)
{
	while (1)
		thread_yield();

	return 0;
}

void idle_init(void)
{
	struct thread* idle;

	kassert(process_kprocess_init() == 0);

	kassert(process_create_kthread("[idle]", idle_kthread_do, NULL, &idle) == 0);
	kassert(sched_add_thread(idle) == 0);
	// test
	kassert(process_create_kthread("[test]", test, NULL, &idle) == 0)
	kassert(sched_add_thread(idle) == 0);
}
