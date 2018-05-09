#include <limits.h>

#include <kernel/interrupt.h>
#include <kernel/sched/sched.h>
#include <kernel/sched/wait.h>

int wait_init(wait_queue_t* wq)
{
	list_init(&wq->threads);

	return 0;
}

void wait_reset(wait_queue_t* wq)
{
	list_node_t* it;
	list_node_t* next;

	list_foreach_safe(&wq->threads, it, next)
		thread_unref(list_entry(it, struct thread, wqe));
}

int wait_wait(wait_queue_t* wq)
{
	struct thread* thread = sched_get_current_thread();

	list_push_back(&wq->threads, &thread->wqe);
	thread_ref(thread);
	sched_sleep_event();

	return 0;
}

int wait_wake(wait_queue_t* wq, unsigned int nb_threads)
{
	unsigned int n = 0;

	irq_disable();

	while (!list_empty(&wq->threads) && n < nb_threads) {
		struct thread* thread = list_entry(list_front(&wq->threads),
										   struct thread, wqe);
		list_pop_front(&wq->threads);

		if (sched_add_thread(thread) == 0)
			++n;

		thread_unref(thread);
	}

	irq_enable();

	return n;
}

int wait_wake_all(wait_queue_t* wq)
{
	return wait_wake(wq, UINT_MAX);
}

bool wait_empty(const wait_queue_t* wq)
{
	return list_empty(&wq->threads);
}
