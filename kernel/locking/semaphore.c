#include <stdint.h>

#include <kernel/locking/semaphore.h>
#include <kernel/atomic.h>
#include <kernel/sched.h>
#include <kernel/libk.h>

int semaphore_create(sem_t* sem, int n)
{
	sem->value = n;
	memset(&sem->wait_queue, 0, sizeof(sem->wait_queue));

	return 0;
}

int semaphore_destroy(sem_t* sem)
{
	struct thread* it;
	list_foreach(&sem->wait_queue, it) {
		sched_add_thread(it);
	}

	memset(sem, 0, sizeof(sem_t));

	return 0;
}

int semaphore_up(sem_t* sem)
{
	atomic_inc_int(sem->value);

	if (!list_empty(&sem->wait_queue)) {
		list_lock_synced(&sem->wait_queue);

		struct thread* first = list_front(&sem->wait_queue);
		list_pop_front(&sem->wait_queue);

		list_unlock_synced(&sem->wait_queue);

		sched_add_thread(first);
	}

	return 0;
}

int semaphore_down(sem_t* sem)
{
	atomic_dec_int(sem->value);

	if (sem->value < 0) {
		list_push_front_synced(&sem->wait_queue, sched_get_current_thread());
		sched_block_current_thread();
	}

	return 0;
}

int semaphore_get_value(const sem_t* sem)
{
	return sem->value;
}
