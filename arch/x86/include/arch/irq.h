#ifndef _ARCH_IRQ_H_
#define _ARCH_IRQ_H_

#include <kernel/types.h>

#define irq_disable() __asm__ ("cli")
#define irq_enable() __asm__ ("sti")

typedef uint32_t irq_state_t;

#define irq_save_state(state)	\
	__asm__ ("pushfl\n"			\
			 "popl %0\n"		\
			 : "=r" (state))

#define irq_restore_state(state)	\
	__asm__ ("pushl %0\n"			\
			 "popfl\n"				\
			 :						\
			 : "r" (state))

#endif
