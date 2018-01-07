#include <limits.h>

#include <kernel/sched/wait.h>
#include <kernel/sched/sched.h>

int wait_create(wait_queue_t* wq)
{
	list_init_null_synced(wq);
	return 0;
}

int wait_wait(wait_queue_t* wq)
{
	struct thread* thread = sched_get_current_thread();

	list_push_back_synced(wq, &thread->wqe);
	thread_ref(thread);
	sched_block_current_thread();

	return 0;
}

int wait_wake(wait_queue_t* wq, unsigned int nb_threads)
{
	unsigned int n = 0;

	list_lock_synced(wq);

	while (!list_empty(wq) && n < nb_threads) {
		struct thread* thread = list_entry(list_front(wq), struct thread, wqe);
		list_pop_front(wq);

		if (thread->state != THREAD_ZOMBIE && sched_add_thread(thread) == 0)
			++n;

		thread_unref(thread);
	}

	list_unlock_synced(wq);

	return n;
}

int wait_wake_all(wait_queue_t* wq)
{
	return wait_wake(wq, UINT_MAX);
}

bool wait_empty(const wait_queue_t* wq)
{
	return list_empty(wq);
}
