#ifndef _LIBK_DEQUE_H_
#define _LIBK_DEQUE_H_

#include <dummyos/errno.h>
#include <kernel/types.h>

typedef struct deque
{
	uint8_t* buffer;
	size_t head;
	size_t tail;
	size_t size;
} deque_t;

#define DEQUE_DEFINE(name, size)		\
	uint8_t name##_buf[size];		\
	deque_t name = {			\
		.buffer = name##_buf,		\
		.head = 0,			\
		.tail = 0,			\
		.size = size			\
	}

#define DEQUE_DEFINE1(name, size)		\
	uint8_t name##_buf[size];		\
	deque_t name

#define deque_buffer(name) name##_buf


static inline void deque_init(deque_t* dq, uint8_t* buffer, size_t size)
{
	dq->buffer = buffer;
	dq->head = 0;
	dq->tail = 0;
	dq->size = size;
}

static inline void deque_flush(deque_t* dq)
{
	deque_init(dq, dq->buffer, dq->size);
}

static inline bool deque_empty(const deque_t* dq)
{
	return (dq->head == dq->tail);
}

static inline bool deque_full(const deque_t* dq)
{
	return ((dq->tail + 1) % dq->size == dq->head);
}

static inline int deque_push_front(deque_t* dq, uint8_t data)
{
	if (deque_full(dq))
		return -ENOSPC;

	dq->head = (dq->head == 0) ? dq->size - 1 : dq->head - 1;
	dq->buffer[dq->head] = data;

	return 0;
}

static inline int deque_push_back(deque_t* dq, uint8_t data)
{
	if (deque_full(dq))
		return -ENOSPC;

	dq->buffer[dq->tail] = data;
	dq->tail = (dq->tail + 1) % dq->size;

	return 0;
}

static inline int deque_pop_front(deque_t* dq, uint8_t* data)
{
	if (deque_empty(dq))
		return -ESPIPE;

	if (data)
		*data = dq->buffer[dq->head];
	dq->head = (dq->head + 1) % dq->size;

	return 0;
}

static inline int deque_pop_back(deque_t* dq, uint8_t* data)
{
	if (deque_empty(dq))
		return -ESPIPE;

	dq->tail = (dq->tail == 0) ? dq->size - 1 : dq->tail - 1;
	if (data)
		*data = dq->buffer[dq->tail];

	return 0;
}

static inline int deque_front(const deque_t* dq, uint8_t* data)
{
	if (deque_empty(dq))
		return -ESPIPE;

	*data = dq->buffer[dq->head];

	return 0;
}

static inline int deque_back(const deque_t* dq, uint8_t* data)
{
	if (deque_empty(dq))
		return -ESPIPE;

	const size_t tail = (dq->tail == 0) ? dq->size - 1 : dq->tail - 1;
	*data = dq->buffer[tail];

	return 0;
}

#endif
