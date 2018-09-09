#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <dummyos/keymap.h>
#include <fs/tty.h>
#include <kernel/interrupt.h>

int keyboard_init(struct tty* tty);

#endif
