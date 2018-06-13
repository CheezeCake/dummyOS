#ifndef _KERNEL_LOG_H_
#define _KERNEL_LOG_H_

#include <arch/log.h>
#include <kernel/terminal.h>

#define log_e_printf(format, ...) { \
	log_printf(format, __VA_ARGS__); \
}
	/* terminal_printf(format, __VA_ARGS__); \ */
/* } */
#define log_w_printf(format, ...) log_printf(format, __VA_ARGS__)
#define log_i_printf(format, ...) log_printf(format, __VA_ARGS__)

#define log_e_puts(str) { \
	log_puts(str); \
}
	/* terminal_puts(str); \ */
/* } */
#define log_e_print(str) log_e_puts(str)

#define log_w_puts(str) log_puts(str)
#define log_w_print(str) log_puts(str)

#define log_i_puts(str) log_puts(str)
#define log_i_print(str) log_puts(str)

#define log_e_putchar(c) { \
	log_putchar(c); \
	terminal_putchar(c); \
}
#define log_w_putchar(c) log_putchar(c)
#define log_i_putchar(c) log_putchar(c)

#endif
