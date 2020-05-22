#ifndef _KERNEL_CONSOLE_H_
#define _KERNEL_CONSOLE_H_

#include <fs/tty.h>

int console_init(void (*putchar)(char), struct tty** tty);

void console_reset(void);

#endif
