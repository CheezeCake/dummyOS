#ifndef _AIRQ_H_
#define _AIRQ_H_

#define disable_irqs() __asm__ ("cli")
#define enable_irqs() __asm__ ("sti")

#endif
