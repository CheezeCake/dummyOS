#ifndef _ARCH_HALT_H_
#define _ARCH_HALT_H_

static inline void __halt(void)
{
	__asm__ ("hlt");
}

#endif
