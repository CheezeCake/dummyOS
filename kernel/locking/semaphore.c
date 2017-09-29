#include <kernel/atomic.h>
#include <kernel/locking/semaphore.h>
#include <kernel/sched/sched.h>
#include <libk/libk.h>

int semaphore_create(sem_t* sem, int n)
{
	sem->value = n;
	wait_create(&sem->wait_queue);

	return 0;
}

int semaphore_destroy(sem_t* sem)
{
	wait_wake_all(&sem->wait_queue);
	memset(sem, 0, sizeof(sem_t));

	return 0;
}

int semaphore_up(sem_t* sem)
{
	atomic_inc_int(sem->value);

	while (!wait_empty(&sem->wait_queue) &&
			wait_wake(&sem->wait_queue, 1) != 1)
		;

	return 0;
}

int semaphore_down(sem_t* sem)
{
	atomic_dec_int(sem->value);

	if (sem->value < 0)
		wait_wait(&sem->wait_queue);

	return 0;
}

int semaphore_get_value(const sem_t* sem)
{
	int val;
	atomic_get_int(sem->value, val);
	return val;
}
