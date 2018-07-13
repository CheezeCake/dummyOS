#include <dummyos/errno.h>
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

pid_t sys_setsid(void)
{
	struct process* current = sched_get_current_process();

	if (current->session_leader)
		return -EPERM;

	current->session = current->pid;
	current->session_leader = true;
	current->pgrp = current->pid;
	current->ctrl_tty = NULL;

	return current->session;
}

int sys_setpgid(pid_t pid, pid_t pgid)
{
	struct process* current = sched_get_current_process();
	struct process* proc = (pid == 0) ? current : process_get(pid);

	if (!proc)
		return -ESRCH;

	if (proc->session_leader)
		return -EPERM;

	if (proc->session != current->session)
		return -EPERM;

	proc->pgrp = (pgid == 0) ? proc->pid : pgid;

	return 0;
}

pid_t sys_getpgid(pid_t pid)
{
	struct process* proc = (pid == 0) ?
		sched_get_current_process() : process_get(pid);

	return (proc) ? proc->pgrp : -ESRCH;
}

int sys_setpgrp(void)
{
	return sys_setpgid(0, 0);
}

pid_t sys_getpgrp(void)
{
	return sched_get_current_process()->pgrp;
}
