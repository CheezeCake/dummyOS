#ifndef _FS_TTY_H_
#define _FS_TTY_H_

#include <fs/device.h>
#include <kernel/locking/mutex.h>
#include <libk/circ_buf.h>

struct tty;

int tty_chardev_init(void);

int tty_create(enum device_minor minor, void (*putchar)(char),
			   struct tty** result);

void tty_set_lflag(struct tty* tty, int flag);

void tty_reset_pgrp(struct tty* tty);

int tty_input(struct tty* tty, uint16_t c);

void tty_destroy(struct tty* tty);

#endif
