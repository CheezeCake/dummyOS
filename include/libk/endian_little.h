#ifndef _LIBK_ENDIAN_LITTLE_H_
#define _LIBK_ENDIAN_LITTLE_H_

#include <kernel/types.h>

static inline uint32_t le2h32(uint32_t x)
{
	return x;
}

static inline uint16_t le2h16(uint16_t x)
{
	return x;
}

#endif
