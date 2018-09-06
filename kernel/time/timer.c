#include <kernel/time/timer.h>
#include <kernel/time/time.h>
#include <libk/libk.h>

int timer_init(struct timer* timer, unsigned delay_ms, timer_callback_t cb)
{
	time_get_current(&timer->time);
	time_add_millis(&timer->time, delay_ms);
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
