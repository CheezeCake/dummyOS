#ifndef _KERNEL_TIME_H_
#define _KERNEL_TIME_H_

#include <dummyos/time.h>
#include <libk/libk.h>

#define TIME_SEC_IN_MS 1000
#define TIME_SEC_IN_NS 1000000000

#define TIME_MS_IN_NS 1000000

struct timer;
struct thread;

static inline void timespec_init(struct timespec* time, unsigned millis)
{
	time->tv_nsec = millis * TIME_MS_IN_NS;

	time->tv_sec = time->tv_nsec / TIME_SEC_IN_NS;
	time->tv_nsec %= TIME_SEC_IN_NS;
}

/**
 * a += b
 */
static inline void timespec_add(struct timespec* a, const struct timespec* b)
{
	const int64_t sum = a->tv_nsec + b->tv_nsec;

	a->tv_nsec = sum % TIME_SEC_IN_NS;
	a->tv_sec += b->tv_sec + (sum / TIME_SEC_IN_NS);
}

static inline void timespec_add_millis(struct timespec* time, unsigned millis)
{
	struct timespec t;
	timespec_init(&t, millis);
	timespec_add(time, &t);
}

/**
 * a -= b
 */
static inline void timespec_diff(struct timespec* a, const struct timespec* b)
{
	const int64_t diff = a->tv_nsec - b->tv_nsec;

	a->tv_sec -= b->tv_sec;

	if (diff < 0) {
		--a->tv_sec;
		a->tv_nsec = TIME_SEC_IN_NS + diff;
	}
	else {
		a->tv_nsec = diff;
	}
}

static inline int64_t timespec_diff_ms(const struct timespec* t1,
				       const struct timespec* t2)
{
	const int64_t sec_in_ms = ((int64_t)t1->tv_sec - t2->tv_sec) * TIME_SEC_IN_MS;
	const int64_t ns_in_ms = ((int64_t)t1->tv_nsec - t2->tv_nsec) / TIME_MS_IN_NS;

	return (sec_in_ms + ns_in_ms);
}


void time_init(struct timespec tick_value);

void time_tick(void);

void time_get_current(struct timespec* time);

int64_t time_cmp(const struct timespec* t1, const struct timespec* t2);

void time_add_timer(struct timer* timer);

void time_nanosleep_intr(struct thread* thr);

#endif
