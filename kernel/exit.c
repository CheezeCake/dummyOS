#include <dummyos/compiler.h>
#include <dummyos/errno.h>
#include <kernel/kassert.h>
#include <kernel/mm/uaccess.h>
#include <kernel/process.h>
#include <kernel/sched/sched.h>
#include <kernel/signal.h>

#include <kernel/log.h>
#include <kernel/interrupt.h>

void sys_exit(int status)
{
	struct thread* cur_thr = sched_get_current_thread();
	struct process* p = cur_thr->process;
	kassert(p != NULL);

	log_e_printf("\nSYSCALL: _exit(%d), pid=%d\n", status, p->pid);

	process_exit(p, status);
	sched_exit();
}

static pid_t _wait(int* __user status, const struct process* p, pid_t pid)
{
	list_node_t* it;
	list_node_t* next;
	int err;

	list_foreach_safe(&p->children, it, next) {
		struct process* child = list_entry(it, struct process, p_child);
		pid_t child_pid = child->pid;

		if (child->state == PROC_ZOMBIE &&
			(!pid || pid == child_pid))
		{
			if (status) {
				err = copy_to_user(status, &child->exit_status, sizeof(*status));
				if (err)
					return -EFAULT;
			}

			process_destroy(child);
			return child_pid;
		}
	}

	return -ECHILD;
}

pid_t sys_wait(int* __user status)
{
	struct process* p = sched_get_current_process();

	while (!list_empty(&p->children)) {
		pid_t child_pid = _wait(status, p, 0);
		if (child_pid > 0)
			return child_pid;

		wait_wait(&p->wait_wq); // wait
	}

	return -ECHILD;
}

#define WNOHANG 1
pid_t sys_waitpid(pid_t pid, int* __user status, int options)
{
	if (pid == -1)
		return sys_wait(status);

	if (pid <= 0)
		return -EINVAL;

	struct process* p = sched_get_current_process();

	do {
		pid_t child_pid = _wait(status, p, pid);
		if (child_pid > 0)
			return child_pid;

		if (options & WNOHANG)
			return -ECHILD;

		wait_wait(&p->wait_wq); // wait
	} while (!list_empty(&p->children));

	return -ECHILD;
}
