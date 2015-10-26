#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>

#define NANOSECS_IN_SEC 1000000000

struct time
{
	uint32_t sec;
	uint32_t nano_sec;
};

void time_init(struct time tick_value);
void time_tick(void);
struct time time_get_current(void);
double time_diff_ms(const struct time* t1, const struct time* t2);

#endif
