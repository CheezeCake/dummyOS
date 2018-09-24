#ifndef _DUMMYOS_TIME_H_
#define _DUMMYOS_TIME_H_

#include <dummyos/types.h>

struct timespec
{
	time_t tv_sec;	/* seconds */
	long tv_nsec;	/* nanoseconds */
};

#endif
