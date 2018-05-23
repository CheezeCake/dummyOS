#include <kernel/kassert.h>
#include <kernel/process.h>
#include <kernel/mm/vmm.h>
#include <kernel/sched/sched.h>

pid_t sys_fork(void)
{
	struct thread* current_thr = sched_get_current_thread();
	struct process* current_proc = current_thr->process;
	struct process* child;
	struct thread* child_thr;
	pid_t child_pid;
	int err;
	int n;

	err = process_fork(current_proc, current_thr, &child, &child_thr);
	if (err)
		return err;

	child_pid = process_register(child);
	if (child_pid < 0) {
		err = child_pid;
		goto fail;
	}

	err = vmm_clone(current_proc->vmm, child->vmm);
	if (err)
		goto fail;

	n = sched_add_process(child);
	if (n < 0) {
		err = n;
		goto fail_sched;
	}

	cpu_context_set_syscall_return_value(child_thr->cpu_context, 0);

	return child_pid;

fail_sched:
	kassert(sched_remove_process(child) == n);
fail:
	process_destroy(child);

	return err;
}
