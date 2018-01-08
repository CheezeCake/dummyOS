#include <kernel/kassert.h>
#include <kernel/process.h>
#include <kernel/sched/sched.h>

#include <kernel/log.h>

void sys_exit(int status)
{
	log_e_printf("\nSYSCALL: _exit(0x%x)\n", status);

	struct process* p = sched_get_current_process();
	kassert(p != NULL);

	struct list_node* it;
	list_foreach(&p->threads, it) {
		struct thread* thread = list_entry(it, struct thread, p_thr_list);
		thread->state = THREAD_ZOMBIE;
	}

	// TODO: add process to a "zombie" list and send SIGCHLD to parent

	sched_remove_current_thread();
}
