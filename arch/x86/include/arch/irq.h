#ifndef _AIRQ_H_
#define _AIRQ_H_

#define irq_disable() __asm__ ("cli")
#define irq_enable() __asm__ ("sti")

#endif
