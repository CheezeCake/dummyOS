#ifndef _ARCH_HALT_H_
#define _ARCH_HALT_H_

#define HALT()			\
	__asm__ ("cli\n"	\
		  "hlt");	\
		  for (;;)

#endif
