#include <kernel/kassert.h>
#include <kernel/kthread.h>
#include <kernel/process.h>
#include <kernel/sched/idle.h>
#include <kernel/sched/sched.h>

#include <kernel/signal.h>
int sys_kill(pid_t pid, uint32_t sig);

static void test(void* data)
{
	sched_sleep_millis(3*1000);
	log_e_print("kill pid 2\n");
	sys_kill(2, SIGINT);
	while (1) {
		/* terminal_putchar('t'); */
		sched_sleep_millis(500);
	}
}

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
	// test
	kassert(kthread_create("[test]", test, NULL, &idle) == 0);
	kassert(sched_add_thread(idle) == 0);
}
