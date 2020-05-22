#ifndef _UART_H_
#define _UART_H_

#include <fs/tty.h>

#if 0

#define __UART 0

#if __UART == 1
# define uart_init	uart1_init
# define uart_putchar	uart1_putchar
# define uart_puts	uart1_puts
# define uart_getchar	uart1_getchar
#else
# define uart_init	uart0_init
# define uart_putchar	uart0_putchar
# define uart_puts	uart0_puts
# define uart_getchar	uart0_getchar
#endif

#endif

void uart0_init(void);

void uart0_putchar(char c);

int uart0_puts(const char* str);

int uart0_getchar(void);

void uart0_set_tty(struct tty* tty);


void uart1_init(void);

void uart1_putchar(char c);

int uart1_puts(const char* str);

int uart1_getchar(void);

void uart1_set_tty(struct tty* tty);

#endif
