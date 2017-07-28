#include <kernel/time.h>
#include <kernel/sched.h>
#include <kernel/kassert.h>
#include <kernel/log.h>


static struct time tick_value;
static struct time current = { .sec = 0, .milli_sec = 0, .nano_sec = 0 };

static struct thread_list wait_list;

static void time_update_thread_wait_list(void);


void time_init(struct time tick_val)
{
	kassert(tick_val.milli_sec < TIME_SEC_IN_MS	&& tick_val.nano_sec < TIME_SEC_IN_NANOSEC);

	tick_value.sec = tick_val.sec;
	tick_value.milli_sec = tick_val.milli_sec;
	tick_value.nano_sec = tick_val.nano_sec;

	list_init_null(&wait_list);
}

void time_tick(void)
{
#ifndef NDEBUG
	unsigned long old_sec = current.sec;
#endif

	current.nano_sec += tick_value.nano_sec;
	if (current.nano_sec >= TIME_MS_IN_NANOSEC) {
		++current.milli_sec;
		current.nano_sec -= TIME_MS_IN_NANOSEC;
	}

	current.milli_sec += tick_value.milli_sec;
	if (current.milli_sec >= TIME_SEC_IN_MS) {
		++current.sec;
		current.milli_sec -= TIME_SEC_IN_MS;
	}

	current.sec += tick_value.sec;

#ifndef NDEBUG
	// should be printed every second
	if (current.sec != old_sec)
		log_printf("sec = %lu, msec = %d, nsec = %lu\n", current.sec,
				current.milli_sec, current.nano_sec);
#endif

	time_update_thread_wait_list();
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

static void time_update_thread_wait_list(void)
{
	struct thread* erase = NULL;
	struct thread* it;

	if (list_empty(&wait_list))
		return;

	list_foreach(&wait_list, it) {
		if (erase) {
			list_erase(&wait_list, erase);
			sched_add_thread(erase);
			erase = NULL;
		}

		if (time_cmp(&current, &it->waiting_for.until) >= 0)
			erase = it;
	}

	if (erase) {
		list_erase(&wait_list, erase);
		sched_add_thread(erase);
	}
}

// TODO: keep list sorted
void time_add_waiting_thread(struct thread* thread)
{
	if (thread)
		list_push_back(&wait_list, thread);
}
