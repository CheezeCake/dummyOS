#ifndef _ARCH_LOCKING_SPINLOCK_H_
#define _ARCH_LOCKING_SPINLOCK_H_

#include <kernel/types.h>

typedef volatile uint32_t spinlock_t;

#define SPINLOCK_NULL 0

static inline void spinlock_lock(spinlock_t* lock)
{
// %= format string:
// https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#AssemblerTemplate
	__asm__ volatile ("spin%=:\n"
					  "	bts $0, (%0)\n"
					  "	jc spin%="
					  :
					  : "r" (lock)
					  : "memory");
}

static inline bool spinlock_try(spinlock_t* lock)
{
	spinlock_t was_locked = 1;
	__asm__ volatile ("bts $0, (%0)\n"
					  "jc 1f\n"
					  "movl $0, %1\n"
					  "1:"
					  : "=r" (was_locked)
					  : "r" (lock)
					  : "memory");

	return !was_locked;
}

static inline void spinlock_unlock(spinlock_t* lock)
{
	__asm__ volatile ("btr $0, (%0)\n"
					  :
					  : "r" (lock)
					  : "memory");
}

static inline bool spinlock_locked(const spinlock_t* lock)
{
	spinlock_t s;
	__asm__ volatile ("movl (%0), %%eax\n"
					  "movl %%eax, %1"
					  : "=r" (s)
					  : "r" (lock)
					  : "%eax", "memory");

	return s;
}

#endif
