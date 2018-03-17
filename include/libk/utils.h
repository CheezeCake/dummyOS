#ifndef _LIBK_UTILS_H_
#define _LIBK_UTILS_H_

#include <kernel/types.h>

#define ALIGN_DOWN(v, alignment) \
	((v) & ~((alignment) - 1))

#define ALIGN_UP(v, alignment) \
	ALIGN_DOWN((v) + (alignment) - 1, alignment)

#define IS_ALIGNED(v, alignment) \
	(((v) & ((alignment) - 1)) == 0)

#define container_of(ptr, type, member) \
	((type*)((void*)(1 ? (ptr) : &((type*)0)->member) - offsetof(type, member)))

#define member_size(type, member) sizeof(((type *)0)->member)

#endif
