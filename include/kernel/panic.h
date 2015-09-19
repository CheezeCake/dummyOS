#ifndef _KPANIC_H_
#define _KPANIC_H_

#include <kernel/terminal.h>
#include <arch/halt.h>

#define PANIC(message) { \
	terminal_printf("\nPANIC : "message" (%s:%s:%d)\n", __func__, __FILE__, __LINE__); \
	HALT(); \
}

#endif
