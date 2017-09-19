#ifndef _LIBK_UTILS_H_
#define _LIBK_UTILS_H_

#define container_of(ptr, type, member) \
	((type*)((char*)(1 ? (ptr) : &((type*)0)->member) - offsetof(type, member)))

#endif
