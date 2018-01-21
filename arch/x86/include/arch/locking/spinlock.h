#ifndef _ARCH_LOCKING_SPINLOCK_H_
#define _ARCH_LOCKING_SPINLOCK_H_

#include <stdint.h>

typedef __volatile__ uint32_t spinlock_t;

#define SPINLOCK_NULL 0

// %= format string: https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#AssemblerTemplate
#define spinlock_lock(lock)		\
	__asm__ __volatile__ (		\
			"spin%=:\n"			\
			"	bts $0, %0\n"	\
			"	jc spin%="		\
			:					\
			: "m" (lock)		\
			: "memory")

#define spinlock_unlock(lock)	\
	__asm__ __volatile__ (		\
			"btr $0, %0"		\
			:					\
			: "m" (lock)		\
			: "memory")

#define spinlock_locked(lock) (lock == 1)

#endif
