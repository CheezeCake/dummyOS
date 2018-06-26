#include <dummyos/ioctl/ioctls.h>
#include <dummyos/keymap.h>
#include <dummyos/termios.h>
#include <fs/chardev.h>
#include <fs/file.h>
#include <kernel/kmalloc.h>
#include <kernel/process.h>
#include <kernel/tty.h>
#include <kernel/sched/sched.h>
#include <kernel/sched/wait.h>
#include <kernel/signal.h>
#include <libk/libk.h>

#define TTY_BUFFER_SIZE	256

struct tty
{
	enum device_minor minor;
	struct termios termios;

	CIRC_BUF_DEFINE1(buffer, TTY_BUFFER_SIZE);
	int lines;

	void (*putchar)(char c);

	mutex_t lock;
	wait_queue_t wq;

	int pgrp;

	list_node_t tty_list;
};

static void termios_init(struct termios* termios)
{
	memset(termios, 0, sizeof(struct termios));

	termios->c_lflag = ICANON | ECHO | ISIG;

	termios->c_cc[VINTR] = CTRL('C');
	termios->c_cc[VEOF] = CTRL('D');
	termios->c_cc[VERASE] = '\b';
}

static inline bool is_intr(const struct tty* tty, char c)
{
	return (c == tty->termios.c_cc[VINTR]);
}

static inline bool is_eof(const struct tty* tty, char c)
{
	return (c == tty->termios.c_cc[VEOF]);
}

static inline bool is_erase(const struct tty* tty, char c)
{
	return (c == tty->termios.c_cc[VERASE]);
}

static inline bool l_flag(const struct tty* tty, int flag)
{
	return (tty->termios.c_lflag & flag);
}

#define l_echo(tty)		l_flag(tty, ECHO)
#define l_canon(tty)	l_flag(tty, ICANON)

static LIST_DEFINE(tty_list);

static void tty_intr(const struct tty* tty, uint32_t sig)
{
	if (tty->pgrp > 0)
		process_pgrp_kill(tty->pgrp, sig);
}

static void* tty_get_device(enum device_minor minor)
{
	list_node_t* it;

	list_foreach(&tty_list, it) {
		struct tty* tty = list_entry(it, struct tty, tty_list);
		if (tty->minor == minor)
			return tty;
	}

	return NULL;
}

static int tty_init(struct tty* tty, enum device_minor minor,
					void (*putchar)(char))
{
	tty->minor = minor;
	termios_init(&tty->termios);

	circ_buf_init(&tty->buffer, tty->circ_buf_buffer(buffer), TTY_BUFFER_SIZE);
	tty->lines = 0;

	tty->putchar = putchar;

	mutex_init(&tty->lock);
	wait_init(&tty->wq);

	tty->pgrp = 0;

	list_node_init(&tty->tty_list);

	return 0;
}

int tty_create(enum device_minor minor, void (*putchar)(char),
			   struct tty** result)
{
	struct tty* tty = kmalloc(sizeof(struct tty));
	if (!tty)
		return -ENOMEM;

	tty_init(tty, minor, putchar);

	list_push_back(&tty_list, &tty->tty_list);
	*result = tty;

	return 0;
}

void tty_reset_pgrp(struct tty* tty)
{
	tty->pgrp = 0;
}

static int tty_input_c(struct tty* tty, char c)
{
	uint8_t peek;
	int err;

	if (l_flag(tty, ISIG) && is_intr(tty, c)) {
		tty_intr(tty, SIGINT);
		return 0;
	}

	if (l_canon(tty)) {
		if (is_erase(tty, c) && !circ_buf_empty(&tty->buffer)) {
			circ_buf_peek_last(&tty->buffer, &peek);
			if (peek != '\n') {
				circ_buf_remove_last(&tty->buffer);
				if (l_echo(tty))
					tty->putchar('\b');
			}
			return 0;
		}
	}

	err = circ_buf_push(&tty->buffer, c);
	if (err)
		return err;

	if (!l_canon(tty) || c == '\n') {
		++tty->lines;
		wait_wake_all(&tty->wq);
	}

	if (l_echo(tty))
		tty->putchar(c);

	return 0;
}

int tty_input(struct tty* tty, uint16_t c)
{
	char byte;

	if (!tty)
		return -EINVAL;

	mutex_lock(&tty->lock);

	byte = c >> 8;
	if (byte)
		tty_input_c(tty, byte);

	byte = c & 0xff;
	if (byte)
		tty_input_c(tty, byte);

	mutex_unlock(&tty->lock);

	return 0;
}

static inline void tty_reset(struct tty* tty)
{
	mutex_destroy(&tty->lock);
}

void tty_destroy(struct tty* tty)
{
	tty_reset(tty);
	kfree(tty);
}

static int tty_open(struct vfs_inode* inode, int flags, struct vfs_file* file)
{
	return 0;
}

static int tty_close(struct vfs_file* this)
{
	return 0;
}


static ssize_t tty_read(struct vfs_file* this, void* buf, size_t count)
{
	int err = 0;
	struct tty* tty = vfs_file_get_inode(this)->private_data;
	uint8_t* buffer = (uint8_t*)buf;
	uint8_t c;
	size_t n = 0;
	bool done = false;

	if (!tty)
		return -EIO;

	while (!done) {
		mutex_lock(&tty->lock);

		if (circ_buf_empty(&tty->buffer)) {
			mutex_unlock(&tty->lock);
			wait_wait(&tty->wq);
		}
		else {
			err = circ_buf_pop(&tty->buffer, &c);
			mutex_unlock(&tty->lock);

			if (err)
				return n;

			buffer[n++] = c;

			if (c == '\n')
				--tty->lines;
			if (n == count ||
				(l_canon(tty) && (c == '\n' || is_eof(tty, c))))
				done = true;
		}
	}

	return n;
}

static ssize_t tty_write(struct vfs_file* this, void* buf, size_t count)
{
	struct tty* tty = vfs_file_get_inode(this)->private_data;
	char* buffer = (char*)buf;

	if (!tty)
		return -EIO;

	for (size_t i = 0; i < count; ++i) {
		mutex_lock(&tty->lock);

		tty->putchar(buffer[i]);

		mutex_unlock(&tty->lock);
	}

	return count;
}

static int tty_ioctl(struct vfs_file* this, int request, intptr_t arg)
{
	struct tty* tty = vfs_file_get_inode(this)->private_data;
	struct process* current;

	if (request != TIOCSCTTY)
		return -EINVAL;

	current = sched_get_current_process();

	if (!current->session_leader) // not the session leader
		return -ENXIO;
	if (current->ctrl_tty) // already has a controling tty
		return -ENXIO;

	if (tty->pgrp != 0)
		process_set_tty_for_pgrp(tty->pgrp, NULL); // steal the tty
	current->ctrl_tty = tty;
	tty->pgrp = current->pgrp;

	return 0;
}

static struct vfs_file_operations tty_fops = {
	.open	= tty_open,
	.close	= tty_close,
	.lseek	= NULL,
	.read	= tty_read,
	.write	= tty_write,
	.ioctl	= tty_ioctl,
};

int tty_chardev_init(void)
{
	return chardev_register(DEVICE_CHAR_TTY_MAJOR, &tty_fops, tty_get_device);
}
