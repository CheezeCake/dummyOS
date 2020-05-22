#ifndef _DUMMYOS_COMPILER_H_
#define _DUMMYOS_COMPILER_H_

#ifndef static_assert
#define static_assert _Static_assert
#endif

#define static_assert_type_size(type, size) \
	static_assert(sizeof(type) == size, #type " == " #size)

#endif
