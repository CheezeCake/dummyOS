#ifndef _KERNEL_LOG_H_
#define _KERNEL_LOG_H_

#include <kernel/terminal.h>

struct log_ops
{
	int (*puts)(const char*);
	void (*putchar)(char);
};

void log_register(const struct log_ops* ops);


int log_printf(const char* format, ...)
	__attribute__((format(printf, 1, 2)));
int log_puts(const char* s);
void log_putchar(char c);

#ifndef LOG_DISABLE

#define	log_print(str) log_puts(str)

#define log_e_printf(format, ...) do {			\
		log_printf(format, __VA_ARGS__);	\
		terminal_printf(format, __VA_ARGS__);	\
	} while (0)
#define log_w_printf(format, ...) log_printf(format, __VA_ARGS__)
#define log_i_printf(format, ...) log_printf(format, __VA_ARGS__)

#define log_e_puts(str) do {			\
		log_puts(str);			\
		terminal_puts(str);		\
	} while (0)
#define log_e_print(str)	log_e_puts(str)

#define log_w_puts(str)		log_puts(str)
#define log_w_print(str)	log_puts(str)

#define log_i_puts(str)		log_puts(str)
#define log_i_print(str)	log_puts(str)


#define log_e_putchar(c) do {			\
		log_putchar(c);			\
		terminal_putchar(c);		\
	} while (0)
#define log_w_putchar(c)	log_putchar(c)
#define log_i_putchar(c)	log_putchar(c)

#else

#define	log_print(str)

#define log_e_printf(format, ...)
#define log_w_printf(format, ...)
#define log_i_printf(format, ...)

#define log_e_puts(str)
#define log_e_print(str)

#define log_w_puts(str)
#define log_w_print(str)

#define log_i_puts(str)
#define log_i_print(str)


#define log_e_putchar(c)
#define log_w_putchar(c)

#endif /*  ifndef LOG_DISABLE */

#endif
