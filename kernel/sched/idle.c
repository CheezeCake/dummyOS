#include <kernel/kassert.h>
#include <kernel/process.h>
#include <kernel/sched/idle.h>
#include <kernel/sched/sched.h>


static void idle_kthread_do(void* args)
{
	while (1)
		thread_yield();
}

void idle_init(void)
{
	process_kprocess_init();

	struct thread* idle = thread_kthread_create("[idle]", 1024, idle_kthread_do, NULL, NULL);
	kassert(idle != NULL);
	kassert(process_add_kthread(idle) == 0);
	thread_unref(idle);

	kassert(sched_add_process(process_get_kprocess()) == 0);
}
