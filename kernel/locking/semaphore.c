#include <kernel/locking/semaphore.h>
#include <kernel/sched/sched.h>
#include <libk/libk.h>

int semaphore_init(sem_t* sem, int n)
{
	atomic_int_init(&sem->value, n);
	wait_init(&sem->wait_queue);

	return 0;
}

int semaphore_destroy(sem_t* sem)
{
	int ret;

	ret = wait_wake_all(&sem->wait_queue);
	wait_reset(&sem->wait_queue);
	memset(sem, 0, sizeof(sem_t));

	return ret;
}

int semaphore_up(sem_t* sem)
{
	int v = atomic_int_inc_return(&sem->value);

	return (v >= 0) ? wait_wake(&sem->wait_queue, 1): 0;
}

int semaphore_down(sem_t* sem)
{
	int v = atomic_int_dec_return(&sem->value);

	return (v < 0) ? wait_wait(&sem->wait_queue) : 0;
}

int semaphore_get_value(const sem_t* sem)
{
	return atomic_int_load(&sem->value);
}
