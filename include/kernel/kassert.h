#ifndef _KERNEL_KASSERT_H_
#define _KERNEL_KASSERT_H_

#include <kernel/panic.h>

#ifdef NDEDUG

#define kassert(expr) expr

#else

#define STR(x) STR1(x)
#define STR1(x) #x

#define kassert(expr)						\
	do {							\
		if (expr) (void)0;				\
		else PANIC("Assertion failed: `" #expr "'");	\
	} while (0)

#endif

#endif
