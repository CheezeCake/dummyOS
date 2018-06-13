#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <dummyos/keymap.h>
#include <kernel/interrupt.h>
#include <kernel/tty.h>

int keyboard_init(struct tty* tty);

#endif
