#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#define EXCEPTION_BASE 0 // base index in IDT
#define EXCEPTION_MAX 31
#define EXCEPTION_NB (EXCEPTION_MAX + 1)

#define IRQ_BASE 32 // base index in IDT
#define IRQ_MAX 15
#define IRQ_NB (IRQ_MAX + 1)

#define INTERRUPT_MAX 256

typedef void (*interrupt_handler_t)(void);

#endif
