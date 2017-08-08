#ifndef _KERNEL_TIMER_H_
#define _KERNEL_TIMER_H_

#include <kernel/time/time.h>
#include <kernel/kmalloc.h>
#include <kernel/list.h>

typedef void (*timer_callback_t)(void* data);

struct timer
{
	struct time time;
	timer_callback_t cb;
	void* data;

	LIST_NODE_CREATE(struct timer);
};

struct timer* timer_create(unsigned int delay_ms, timer_callback_t cb, void* data);

static inline void timer_destroy(struct timer* timer)
{
	kfree(timer);
}

static inline void timer_trigger(struct timer* timer)
{
	timer->cb(timer->data);
}

#endif
