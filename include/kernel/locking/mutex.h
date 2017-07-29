#ifndef _MUTEX_H_
#define _MUTEX_H_

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

static inline int mutex_lock(const mutex_t* mutex)
{
	return semaphore_down(mutex);
}

static inline int mutex_trylock(const mutex_t* mutex)
{
	int val;
	if (semaphore_getvalue(mutex, &val) != 0)
		return -1;

	return (val == 1) ? mutex_lock(mutex) : 0;
}

static inline int mutex_unlock(const mutex_t* mutex)
{
	return semaphore_up(mutex);
}

#endif
