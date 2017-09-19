#include <stdint.h>

#include <kernel/atomic.h>
#include <kernel/locking/semaphore.h>
#include <kernel/sched/sched.h>
#include <libk/libk.h>

static int release_thread(struct list_node* n)
{
	int ret = 0;

	struct thread* thread = list_entry(n, struct thread, sem_wq);
	if (thread->state != THREAD_DEAD)
		ret = sched_add_thread(thread);
	thread_unref(thread);

	return ret;
}

int semaphore_create(sem_t* sem, int n)
{
	sem->value = n;
	memset(&sem->wait_queue, 0, sizeof(sem->wait_queue));

	return 0;
}

int semaphore_destroy(sem_t* sem)
{
	struct list_node* it = list_begin(&sem->wait_queue);

	while (it) {
		struct list_node* next = list_it_next(it);
		release_thread(it);
		it = next;
	}

	memset(sem, 0, sizeof(sem_t));

	return 0;
}

int semaphore_up(sem_t* sem)
{
	atomic_inc_int(sem->value);

	if (!list_empty(&sem->wait_queue)) {
		list_lock_synced(&sem->wait_queue);

		struct list_node* first;
		bool dead = false;
		do {
			first = list_front(&sem->wait_queue);
			list_pop_front(&sem->wait_queue);

			const struct thread* thread = list_entry(first, struct thread, sem_wq);
			dead = (thread->state == THREAD_DEAD);
			release_thread(first); // TODO: check for error
		} while (dead);

		list_unlock_synced(&sem->wait_queue);
	}

	return 0;
}

int semaphore_down(sem_t* sem)
{
	atomic_dec_int(sem->value);

	if (sem->value < 0) {
		struct thread* current_thread = sched_get_current_thread();

		thread_ref(current_thread);
		list_push_front_synced(&sem->wait_queue, &current_thread->sem_wq);

		sched_block_current_thread();
	}

	return 0;
}

int semaphore_get_value(const sem_t* sem)
{
	int val;
	atomic_get_int(sem->value, val);
	return val;
}
