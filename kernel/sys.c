#include <kernel/process.h>
#include <kernel/sched/sched.h>

pid_t sys_getpid(void)
{
	return sched_get_current_process()->pid;
}

pid_t sys_getppid(void)
{
	const struct process* parent = sched_get_current_process()->parent;
	return (parent) ? parent->pid : 0;
}
