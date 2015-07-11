#ifndef _AHALT_H_
#define _AHALT_H_

#define HALT() \
	__asm__ ( \
			"cli\n" \
			"hlt"); \
	for (;;)
#endif
