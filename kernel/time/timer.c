#include <stdbool.h>

#include <kernel/time/timer.h>
#include <kernel/time/time.h>

struct timer* timer_create(unsigned int delay_ms, timer_callback_t cb, void* data)
{
	struct timer* timer = kmalloc(sizeof(struct timer));

	if (!timer)
		return NULL;

	time_get_current(&timer->time);
	time_add_millis(&timer->time, delay_ms);
	timer->cb = cb;
	timer->data = data;

	return timer;
}

void timer_destroy(struct timer* timer)
{
	kfree(timer);
}

void timer_register(struct timer* timer)
{
	time_add_timer(timer);
}

void timer_trigger(struct timer* timer)
{
	timer->cb(timer->data);
}
