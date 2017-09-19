#include <kernel/kassert.h>
#include <kernel/log.h>
#include <kernel/sched/sched.h>
#include <kernel/synced_list.h>
#include <kernel/time/time.h>
#include <kernel/time/timer.h>


static struct time tick_value;
static struct time current = { .sec = 0, .milli_sec = 0, .nano_sec = 0 };
static struct
{
	SYNCED_LIST_CREATE
} timer_list;

static void time_update_timer_list(void);

void time_init(struct time tick_val)
{
	kassert(tick_val.milli_sec < TIME_SEC_IN_MS	&& tick_val.nano_sec < TIME_SEC_IN_NANOSEC);

	tick_value.sec = tick_val.sec;
	tick_value.milli_sec = tick_val.milli_sec;
	tick_value.nano_sec = tick_val.nano_sec;

	list_init_null_synced(&timer_list);
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

static void release_timer(struct list_node* n)
{
	struct timer* timer = list_entry(n, struct timer, t_list);
	timer_trigger(timer);
	timer_unref(timer);
}

static void time_update_timer_list(void)
{
	struct list_node* erase = NULL;
	struct list_node* it;

	if (list_empty(&timer_list))
		return;

	list_lock_synced(&timer_list); // lock list
	list_foreach(&timer_list, it) {
		if (erase) {
			list_erase(&timer_list, erase); // normal erase
			release_timer(erase);
			erase = NULL;
		}

		const struct timer* timer_it = list_entry(it, struct timer, t_list);
		if (time_cmp(&current, &timer_it->time) >= 0)
			erase = it;
	}
	list_unlock_synced(&timer_list); // unlock

	if (erase) {
		list_erase_synced(&timer_list, erase); // synced erase
		release_timer(erase);
	}
}

// TODO: keep list sorted
void time_add_timer(struct timer* timer)
{
	if (timer) {
		log_printf("sleep add %s\n", ((struct thread*)timer->data)->name);
		list_push_back_synced(&timer_list, &timer->t_list);
	}
}
