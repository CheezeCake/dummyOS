#include <kernel/interrupt.h>
#include <kernel/kassert.h>
#include <kernel/sched/sched.h>
#include <kernel/time/time.h>
#include <kernel/time/timer.h>

#include <kernel/log.h>

static struct time tick_value;
static struct time current = { .sec = 0, .milli_sec = 0, .nano_sec = 0 };

static list_t timer_list;

static void time_update_timer_list(void);

void time_init(struct time tick_val)
{
	kassert(tick_val.milli_sec < TIME_SEC_IN_MS	&& tick_val.nano_sec < TIME_SEC_IN_NANOSEC);

	tick_value.sec = tick_val.sec;
	tick_value.milli_sec = tick_val.milli_sec;
	tick_value.nano_sec = tick_val.nano_sec;

	list_init(&timer_list);
}

void time_tick(void)
{
#ifndef NDEBUG
	unsigned long old_sec = current.sec;
#endif

	time_add_time(&current, &tick_value);

#ifndef NDEBUG
	// should be printed every second
	if (current.sec != old_sec)
		log_printf("sec = %llu, msec = %d, nsec = %u\n", current.sec,
				current.milli_sec, current.nano_sec);
#endif

	time_update_timer_list();
}

void time_get_current(struct time* time)
{
	time->sec = current.sec;
	time->milli_sec = current.milli_sec;
	time->nano_sec = current.nano_sec;
}

int time_cmp(const struct time* t1, const struct time* t2)
{
	const int sec_cmp = t1->sec - t2->sec;
	if (sec_cmp == 0) {
		const int milli_sec_cmp = t1->milli_sec - t2->milli_sec;
		return (milli_sec_cmp == 0) ? (t1->nano_sec - t2->nano_sec) : milli_sec_cmp;
	}
	else {
		return sec_cmp;
	}
}

static void release_timer(list_node_t* n)
{
	struct timer* timer = list_entry(n, struct timer, t_list);
	timer_trigger(timer);
}

static void time_update_timer_list(void)
{
	list_node_t* it;
	list_node_t* next;

	irq_disable();

	list_foreach_safe(&timer_list, it, next) {
		const struct timer* timer_it = list_entry(it, struct timer, t_list);
		if (time_cmp(&current, &timer_it->time) >= 0) {
			list_erase(it);
			release_timer(it);
		}
	}

	irq_enable();
}

void time_add_timer(struct timer* timer)
{
	if (timer) {
		const struct thread* thr = container_of(timer, struct thread, timer);
		log_printf("sleep add %s\n", thr->name);

		irq_disable();
		list_push_back(&timer_list, &timer->t_list);
		irq_enable();
	}
}
