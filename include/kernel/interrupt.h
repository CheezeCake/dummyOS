#ifndef _KERNEL_INTERRUPT_H_
#define _KERNEL_INTERRUPT_H_

#include <arch/irq.h> // {enable,disable}_irqs()

typedef void (*interrupt_handler_t)(void);

#endif

