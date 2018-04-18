#ifndef _LIBK_UTILS_H_
#define _LIBK_UTILS_H_

#include <kernel/types.h>

#define align_down(v, alignment) \
	((v) & ~((alignment) - 1))

#define align_up(v, alignment) \
	align_down((v) + (alignment) - 1, alignment)

#define is_aligned(v, alignment) \
	(((v) & ((alignment) - 1)) == 0)

#define container_of(ptr, type, member) \
	((type*)((int8_t*)(1 ? (ptr) : &((type*)0)->member) - offsetof(type, member)))

#endif
