#include <kernel/time/timer.h>
#include <kernel/time/time.h>
#include <libk/libk.h>

int timer_init(struct timer* timer, const struct timespec* delay,
	       timer_callback_t cb)
{
	time_get_current(&timer->time);
	timespec_add(&timer->time, delay);
	timer->cb = cb;

	return 0;
}

void timer_reset(struct timer* timer)
{
	memset(timer, 0, sizeof(struct timer));
}

void timer_register(struct timer* timer)
{
	time_add_timer(timer);
}

void timer_trigger(struct timer* timer)
{
	timer->cb(timer);
}
