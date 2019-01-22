#ifndef _KERNEL_INTERRUPT_H_
#define _KERNEL_INTERRUPT_H_

#include <arch/irq.h> // __{enable,disable}_irqs()
#include <kernel/cpu_context.h>

typedef void (*interrupt_handler_t)(int nr,
				    struct cpu_context* interrupted_ctx);

#define irq_disable()			\
{					\
	irq_state_t __state;		\
	irq_save_state(__state);	\
	__irq_disable()

#define irq_enable()			\
	irq_restore_state(__state);	\
}

#endif
