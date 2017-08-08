#ifndef _KERNEL_TIME_H_
#define _KERNEL_TIME_H_

#include <stdint.h>

#define TIME_SEC_IN_MS 1000
#define TIME_SEC_IN_NANOSEC 1000000000

#define TIME_MS_IN_NANOSEC 1000000

struct thread;

struct time
{
	unsigned long long sec;
	unsigned short milli_sec;
	unsigned int nano_sec;
};

static inline void time_add_millis(struct time* time,
		unsigned int millis)
{
	millis += time->milli_sec;

	time->sec += millis / TIME_SEC_IN_MS;
	time->milli_sec = millis % TIME_SEC_IN_MS;
}

static inline void time_add_time(struct time* a, const struct time* b)
{
	unsigned long long sum = 0;

	sum = a->nano_sec + b->nano_sec;
	a->nano_sec = sum % TIME_MS_IN_NANOSEC;

	sum /= TIME_MS_IN_NANOSEC;
	sum += a->milli_sec + b->milli_sec;
	a->milli_sec = sum % TIME_SEC_IN_MS;

	sum /= TIME_SEC_IN_MS;
	a->sec += sum + b->sec;
}

static inline int64_t time_diff_ms(const struct time* t1, const struct time* t2)
{
	const int64_t sec = ((int64_t)t1->sec - t2->sec) * TIME_SEC_IN_MS;
	const int64_t ms = (int64_t)t1->milli_sec - t2->milli_sec;

	return (sec + ms);
}


void time_init(struct time tick_value);
void time_tick(void);
void time_get_current(struct time* time);
int time_cmp(const struct time* t1, const struct time* t2);
void time_add_waiting_thread(struct thread* thread);

#endif
