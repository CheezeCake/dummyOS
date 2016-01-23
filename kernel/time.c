#include <kernel/interrupt.h>
#include <kernel/time.h>
#include <kernel/log.h>


static struct time tick_value;

static uint64_t ticks = 0;
static struct time current = { .sec = 0, .nano_sec = 0 };

void time_init(struct time tick_val)
{
	tick_value.sec = tick_val.sec;
	tick_value.nano_sec = tick_val.nano_sec;
}

void time_tick(void)
{
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
		log_printf("sec = %d, nsec = %d\n", (unsigned int)current.sec,
				(unsigned int)current.nano_sec);
#endif

	enable_irqs();
}

struct time time_get_current(void)
{
	return current;
}

double time_diff_ms(const struct time* t1, const struct time* t2)
{
	double sec = (double)t1->sec - t2->sec;
	double ns = (double)t1->nano_sec - t2->nano_sec;

	return (sec * 1000 + ns / 1000000);
}
