#ifndef _ARCH_IRQ_H_
#define _ARCH_IRQ_H_

#include <kernel/types.h>

#define __irq_disable() __asm__ ("cpsid i")
#define __irq_enable() __asm__ ("cpsie i")

typedef uint32_t irq_state_t;

#define irq_save_state(state) \
	__asm__ ("mrs %0, cpsr" : "=r" (state))

#define irq_restore_state(state) \
	__asm__ ("msr cpsr, %0" : : "r" (state))

// @note defined in arch/arm/machine/*
void machine_irq_handle(void);

#endif
