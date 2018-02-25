#ifndef _KERNEL_TIMER_H_
#define _KERNEL_TIMER_H_

#include <kernel/time/time.h>
#include <kernel/kmalloc.h>
#include <libk/list.h>
#include <libk/refcount.h>

typedef int (*timer_callback_t)(void* data);

struct timer
{
	struct time time;
	timer_callback_t cb;
	void* data;

	refcount_t refcnt;

	list_node_t t_list; // timer_list node
};

struct timer* timer_create(unsigned int delay_ms, timer_callback_t cb, void* data);
void timer_ref(struct timer* timer);
void timer_unref(struct timer* timer);
int timer_get_ref(const struct timer* timer);

void timer_register(struct timer* timer);
void timer_trigger(struct timer* timer);

#endif
