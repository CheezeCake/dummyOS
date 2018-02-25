#ifndef _LIBK_UTILS_H_
#define _LIBK_UTILS_H_

#include <kernel/types.h>

/**
 * @brief Little-endian to host 32 bits
 */
static inline uint32_t le2h32(uint32_t x);

/**
 * @brief Little-endian to host 16 bits
 */
static inline uint16_t le2h16(uint16_t x);

/*
 * Include platform specific implementation.
 */
#include <arch/utils.h>


#define ALIGN_DOWN(v, alignment) \
	((v) & ~((alignment) - 1))

#define ALIGN_UP(v, alignment) \
	ALIGN_DOWN((v) + (alignment) - 1, alignment)

#define IS_ALIGNED(v, alignment) \
	(((v) & ((alignment) - 1)) == 0)

#define container_of(ptr, type, member) \
	((type*)((void*)(1 ? (ptr) : &((type*)0)->member) - offsetof(type, member)))

#endif
