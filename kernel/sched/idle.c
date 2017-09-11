#include <kernel/kassert.h>
#include <kernel/process.h>
#include <kernel/sched/idle.h>
#include <kernel/sched/sched.h>

static struct process kthreadd;

static void idle_kthread_do(void* args)
{
	while (1)
		thread_yield();
}

void idle_init(void)
{
	kassert(process_kprocess_create(&kthreadd, "[idle]", idle_kthread_do) == 0);
	sched_add_process(&kthreadd);
}
