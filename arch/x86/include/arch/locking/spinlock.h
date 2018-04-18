#ifndef _ARCH_LOCKING_SPINLOCK_H_
#define _ARCH_LOCKING_SPINLOCK_H_

static inline void spinlock_lock(spinlock_t* lock)
{
	__asm__ volatile ("acquire:\n"
					  "	lock bts $0, (%0)\n"
					  "	jnc end\n"
					  ".spin:\n"
					  " cmp $0, (%0)\n"
					  " je acquire\n"
					  " jmp .spin\n"
					  "end:"
					  :
					  : "r" (lock)
					  : "memory");
}

static inline bool spinlock_try(spinlock_t* lock)
{
	spinlock_t was_locked = 1;
	__asm__ volatile ("lock bts $0, (%0)\n"
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
	__asm__ volatile ("lock btr $0, (%0)\n"
					  :
					  : "r" (lock)
					  : "memory");
}

#endif
