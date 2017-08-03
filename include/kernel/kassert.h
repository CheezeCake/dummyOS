#ifndef _KERNEL_KASSERT_H_
#define _KERNEL_KASSERT_H_

#include <kernel/panic.h>

#define STR(x) STR1(x)
#define STR1(x) #x

#define kassert(expr) {\
	if (expr) (void)0; \
	else PANIC("Assertion failed: `" #expr "'"); \
}

#endif
