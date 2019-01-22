#ifndef _KERNEL_HALT_H_
#define _KERNEL_HALT_H_

#include <arch/halt.h>
#include <arch/irq.h>

static inline void halt(void)
{
	__irq_disable();
	__halt();
	for (;;)
		;
}

#endif
