#include "keyboard.h"
#include "../io_ports.h"
#include "../irq.h"
#include <libk/libk.h>

#include <kernel/log.h>

/*
 * keymap
 */
#define DEFINE_KEYMAP const keyboard_map_t keymap
#include "keymaps/fr.src"


#define KEYBOARD_DATA_PORT 0x60

#define LEFT_SHIFT		0x2a
#define RIGHT_SHIFT		0x36

#define LEFT_CTRL		0x1d
#define RIGHT_CTRL_EXT	0x1d

#define BREAKCODE_LIMIT		0x80
#define EXTENDED_KEYCODE	0xe0


static struct tty* console_tty = NULL;

static inline char breakcode2makecode(char breakcode)
{
	return (breakcode ^ BREAKCODE_LIMIT);
}

static void keyboard_intr(int nr, struct cpu_context* interrupted_ctx)
{
	static bool extended = false;
	static enum keyboard_state state = KBD_REGULAR;

	int sc = inb(KEYBOARD_DATA_PORT);
	uint16_t chars = 0;

	if (sc == EXTENDED_KEYCODE) {
		extended = true;
	}
	else if (extended) {
		extended = false;

		if (sc < BREAKCODE_LIMIT) {
			if (sc == RIGHT_CTRL_EXT)
				state = KBD_CTRL;
		}
		else {
			if (breakcode2makecode(sc) == RIGHT_CTRL_EXT)
				state = KBD_REGULAR;
		}
	}
	else {
		if (sc < BREAKCODE_LIMIT) {
			if (sc == LEFT_SHIFT || sc == RIGHT_SHIFT)
				state = KBD_SHIFT;
			else if (sc == LEFT_CTRL)
				state = KBD_CTRL;
			else if (keymap[sc][state])
				chars = keymap[sc][state];
		}
		else {
			char make = breakcode2makecode(sc);
			if ((make == LEFT_SHIFT || make == RIGHT_SHIFT) &&
			    state == KBD_SHIFT) {
				state = KBD_REGULAR;
			}
			else if (make == LEFT_CTRL &&
				 state == KBD_CTRL) {
				state = KBD_REGULAR;
			}
		}
	}

	if (chars)
		tty_input(console_tty, chars);
}

int keyboard_init(struct tty* tty)
{
	console_tty = tty;

	return irq_set_handler(IRQ_KEYBOARD, keyboard_intr);
}
