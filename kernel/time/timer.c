#include <kernel/time/timer.h>
#include <kernel/time/time.h>
#include <libk/libk.h>

struct timer* timer_create(unsigned int delay_ms, timer_callback_t cb, void* data)
{
	struct timer* timer = kmalloc(sizeof(struct timer));

	if (!timer)
		return NULL;

	time_get_current(&timer->time);
	time_add_millis(&timer->time, delay_ms);
	timer->cb = cb;
	timer->data = data;

	refcount_init(&timer->refcnt);

	return timer;
}

static void timer_reset(struct timer* timer)
{
	memset(timer, 0, sizeof(struct timer));
}

static void timer_destroy(struct timer* timer)
{
	timer_reset(timer);
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

void timer_ref(struct timer* timer)
{
	refcount_inc(&timer->refcnt);
}

void timer_unref(struct timer* timer)
{
	if (refcount_dec(&timer->refcnt) == 0)
		timer_destroy(timer);
}

int timer_get_ref(const struct timer* timer)
{
	return refcount_get(&timer->refcnt);
}
