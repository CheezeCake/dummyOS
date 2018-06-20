#include <kernel/kassert.h>
#include <kernel/kthread.h>
#include <kernel/sched/idle.h>
#include <kernel/sched/sched.h>

static void idle_kthread_do(void* data)
{
	while (1)
		sched_yield();
}

void idle_init(void)
{
	struct thread* idle;

	kassert(kthread_create("[idle]", idle_kthread_do, NULL, &idle) == 0);
	idle->priority = SCHED_PRIORITY_LEVEL_MIN;
	kassert(sched_add_thread(idle) == 0);
}
