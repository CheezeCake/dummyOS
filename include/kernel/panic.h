#ifndef _KERNEL_PANIC_H_
#define _KERNEL_PANIC_H_

#include <kernel/terminal.h>
#include <arch/halt.h>
#include <kernel/debug.h>
#include <kernel/log.h>

#define PANIC(message) { \
	log_e_printf("\nPANIC : "message" (%s:%s:%d)\n", __func__, __FILE__, __LINE__); \
	debug_dump(); \
	HALT(); \
}

#endif
