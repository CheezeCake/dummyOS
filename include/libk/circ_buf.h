#ifndef _LIBK_CIRC_BUF_H_
#define _LIBK_CIRC_BUF_H_

#include <dummyos/errno.h>
#include <kernel/types.h>

typedef struct circ_buf
{
	uint8_t* buffer;
	size_t head;
	size_t tail;
	size_t size;
} circ_buf_t;

#define CIRC_BUF_DEFINE(name, size) \
	uint8_t name##_buf[size];		\
	circ_buf_t name = {				\
		.buffer = name##_buf,		\
		.head = 0,					\
		.tail = 0,					\
		.size = size				\
	}

#define CIRC_BUF_DEFINE1(name, size)	\
	uint8_t name##_buf[size];			\
	circ_buf_t name

#define circ_buf_buffer(name) name##_buf


static inline void circ_buf_init(circ_buf_t* cb, uint8_t* buffer, size_t size)
{
	cb->buffer = buffer;
	cb->head = 0;
	cb->tail = 0;
	cb->size = size;
}

static inline bool circ_buf_empty(const circ_buf_t* cb)
{
	return (cb->head == cb->tail);
}

static inline bool circ_buf_full(const circ_buf_t* cb)
{
	size_t next = cb->head + 1;
	if (next >= cb->size)
		next = 0;

	return (next == cb->tail);
}

static inline int circ_buf_push(circ_buf_t* cb, uint8_t data)
{
	size_t next = cb->head + 1;
	if (next >= cb->size)
		next = 0;

	if (next == cb->tail) // full
		return -ENOSPC;

	cb->buffer[cb->head] = data;
	cb->head = next;

	return 0;
}

static inline int circ_buf_pop(circ_buf_t* cb, uint8_t* data)
{
	if (circ_buf_empty(cb))
		return -ESPIPE;

	*data = cb->buffer[cb->tail++];
	if (cb->tail >= cb->size)
		cb->tail = 0;

	return 0;
}

static inline int circ_buf_peek_last(const circ_buf_t* cb, uint8_t* data)
{
	size_t last;

	if (circ_buf_empty(cb))
		return -ESPIPE;

	last = cb->head - 1;
	if (last < 0)
		last = cb->size - 1;

	*data = cb->buffer[last];

	return 0;
}

static inline int circ_buf_remove_last(circ_buf_t* cb)
{
	size_t previous;

	if (circ_buf_empty(cb))
		return -ESPIPE;

	previous = cb->head - 1;
	if (previous < 0)
		previous = cb->size - 1;

	cb->head = previous;

	return 0;
}

#endif
