#include <kernel/console.h>

static struct tty* console_tty;

int console_init(void (*putchar)(char), struct tty** tty)
{
	if (!putchar)
		return -EINVAL;

	int err = tty_create(DEVICE_CHAR_CONSOLE_MINOR, putchar, &console_tty);
	if (err)
		return err;

	if (tty)
		*tty = console_tty;

	return 0;
}

void console_reset(void)
{
	if (console_tty)
		tty_destroy(console_tty);
}
