#include <kernel/interrupt.h>
#include <kernel/time.h>

#include <kernel/log.h>

static struct time tick_value;

static struct time current;

void time_init(struct time tick_val)
{
	tick_value.sec = tick_val.sec;
	tick_value.nano_sec = tick_val.nano_sec;

	current.sec = 0;
	current.nano_sec = 0;
}

void time_tick(void)
{
	static uint32_t ticks = 0;

	disable_irqs();

	++ticks;

	uint32_t nano_sec = current.nano_sec + tick_value.nano_sec;
#ifndef NDEBUG
	uint32_t old_sec = current.sec;
#endif
	current.sec += tick_value.sec + (nano_sec / NANOSECS_IN_SEC);
	current.nano_sec = nano_sec % NANOSECS_IN_SEC;

#ifndef NDEBUG
	// should be printed every second
	if (current.sec != old_sec)
		log_printf("sec = %u, nsec = %d\n", current.sec, current.nano_sec);
#endif

	enable_irqs();
}

struct time time_get_current(void)
{
	return current;
}
