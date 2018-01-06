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

static struct process kthreadd;

static void idle_kthread_do(void* args)
{
	while (1)
		thread_yield();
}

void idle_init(void)
{
	kassert(process_kprocess_create(&kthreadd, "[idle]", idle_kthread_do) == 0);

	struct thread* test_thread;
	kassert((test_thread = thread_kthread_create("test", &kthreadd, 1024, test, NULL, exit)) != NULL);
	process_add_thread(&kthreadd, test_thread);

	sched_add_process(&kthreadd);
	thread_unref(test_thread);
}
