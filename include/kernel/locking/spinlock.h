#ifndef _KERNEL_LOCKING_SPINLOCK_H_
#define _KERNEL_LOCKING_SPINLOCK_H_

#include <kernel/types.h>

typedef volatile uint32_t spinlock_t;

#define SPINLOCK_NULL 0

static inline void spinlock_lock(spinlock_t* lock);

static inline bool spinlock_try(spinlock_t* lock);

static inline void spinlock_unlock(spinlock_t* lock);

static inline bool spinlock_locked(const spinlock_t* lock)
{
	return (*lock == 1);
}

/*
 * Include platform specific implementation.
 */
#include <arch/locking/spinlock.h>

#endif
