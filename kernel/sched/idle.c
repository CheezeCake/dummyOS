#include <kernel/kassert.h>
#include <kernel/process.h>
#include <kernel/sched/idle.h>
#include <kernel/sched/sched.h>

static void test(void* args)
{
	/* return; */
	while (1) {
		terminal_putchar('t');
		thread_sleep(500);
	}
}

static void exit(void)
{
	log_e_print("EXIT\n");
	sched_remove_current_thread();
}

static void idle_kthread_do(void* args)
{
	while (1)
		thread_yield();
}

void idle_init(void)
{
	int err;

	process_kprocess_init();

	struct thread* idle;
	err = thread_kthread_create("[idle]", 1024, idle_kthread_do, NULL, NULL,
								&idle);
	kassert(err == 0);
	kassert(process_add_kthread(idle) == 0);
	thread_unref(idle);

	// test
	struct thread* test_thread;
	err = thread_kthread_create("[test]", 1024, test, NULL, exit, &test_thread);
	kassert(err == 0);
	kassert(process_add_kthread(test_thread) == 0);
	thread_unref(test_thread);
	//

	kassert(sched_add_process(process_get_kprocess()) == 0);
}
