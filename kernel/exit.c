#include <kernel/kassert.h>
#include <kernel/process.h>
#include <kernel/sched/sched.h>

#include <kernel/log.h>
#include <kernel/interrupt.h>

void sys_exit(int status)
{
	log_e_printf("\nSYSCALL: _exit(0x%x)\n", status);

	struct thread* cur_thr = sched_get_current_thread();
	struct process* p = cur_thr->process;
	kassert(p != NULL);

	list_node_t* it;
	list_foreach(&p->threads, it) {
		struct thread* thread = list_entry(it, struct thread, p_thr_list);
		if (thread != cur_thr) {
			log_printf("before: refcnt=%d (%s)\n", refcount_get(&thread->refcnt), thread->name);
			thread_set_state(thread, THREAD_DEAD);
			kassert(sched_remove_thread(thread) == 0);
			log_printf("after: refcnt=%d (%s)\n", refcount_get(&thread->refcnt), thread->name);
		}
	}

	p->state = PROC_ZOMBIE;
	// TODO: send SIGCHLD to parent and destroy proc on sig reception

	sched_exit();
}
