#ifndef _LIBK_UTILS_H_
#define _LIBK_UTILS_H_

#include <arch/utils.h>

#define container_of(ptr, type, member) \
	((type*)((char*)(1 ? (ptr) : &((type*)0)->member) - offsetof(type, member)))

#define __MIN_MAX(a, b, op) (((a) op (b)) ? (a) : (b))
#define MIN(a, b) __MIN_MAX(a, b, <)
#define MAX(a, b) __MIN_MAX(a, b, >)

#endif
