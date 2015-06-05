#ifndef _SEGMENT_H_
#define _SEGMENT_H_

#include <stdint.h>
#include <stdbool.h>

#define KCODE 1
#define KDATA 2
#define UCODE 3
#define UDATA 4

#define CODE_SEGMENT 0xb
#define DATA_SEGMENT 0x3

static inline uint16_t make_segment_register(uint8_t privilege,
		bool in_ldt, uint16_t index)
{
	return ((privilege & 0x3) | ((in_ldt ? 1 : 0) << 2) | (index << 3));
}

#endif
