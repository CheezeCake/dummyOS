#ifndef _KPANIC_H_
#define _KPANIC_H_

#include <kernel/terminal.h>
#include <arch/halt.h>

#define PANIC(message) { \
	terminal_printf("\nPANIC : "message" (%s)\n", __func__); \
	HALT(); \
}

#endif
