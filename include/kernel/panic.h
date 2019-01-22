#ifndef _KERNEL_PANIC_H_
#define _KERNEL_PANIC_H_

#include <kernel/terminal.h>
#include <kernel/halt.h>
#include <kernel/debug.h>
#include <kernel/log.h>

#define PANIC(message) do {								\
	log_e_printf("\nPANIC : "message" (%s:%s:%d)\n", __func__, __FILE__, __LINE__); \
	debug_dump();									\
	halt();										\
} while (0)

#endif
