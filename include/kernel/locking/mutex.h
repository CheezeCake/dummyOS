#ifndef _KERNEL_LOCKING_MUTEX_H_
#define _KERNEL_LOCKING_MUTEX_H_

#include <kernel/locking/semaphore.h>

typedef sem_t mutex_t;

static inline int mutex_create(mutex_t* mutex)
{
	return semaphore_create(mutex, 1);
}

static inline int mutex_destroy(mutex_t* mutex)
{
	return semaphore_destroy(mutex);
}

static inline int mutex_lock(mutex_t* mutex)
{
	return semaphore_down(mutex);
}

static inline int mutex_trylock(mutex_t* mutex)
{
	return (semaphore_get_value(mutex) == 1) ? mutex_lock(mutex) : 0;
}

static inline int mutex_unlock(mutex_t* mutex)
{
	return semaphore_up(mutex);
}

#endif
