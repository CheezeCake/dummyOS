#include <fs/tty.h>
#include <kernel/terminal.h>
#include "console.h"
#include "drivers/keyboard.h"

static struct tty* console_tty;

int console_init(void)
{
	int err;

	err = tty_create(DEVICE_CHAR_CONSOLE_MINOR, terminal_putchar, &console_tty);
	if (!err)
		err = keyboard_init(console_tty);

	return err;
}

void console_reset(void)
{
	if (console_tty)
		tty_destroy(console_tty);
}
