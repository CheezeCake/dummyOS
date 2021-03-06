#ifndef _SEGMENT_H_
#define _SEGMENT_H_

// indexes in GDT
#define KCODE 1
#define KDATA 2
#define UCODE 3
#define UDATA 4
#define TSS 5

// Intel Architecture Software Developer’s Manual Volume 3, section 3.4.3
#define SYSTEM_SEGMENT 0
#define CODE_DATA_SEGMENT 1

#ifndef ASSEMBLY

#include <kernel/types.h>

// Intel Architecture Software Developer’s Manual Volume 3, section 3.4.3.1
enum code_data_segment_types
{
	CODE_SEGMENT = 11, // Read/Write, accessed
	DATA_SEGMENT = 3 // Execute/Read, accessed
};

// Intel Architecture Software Developer’s Manual Volume 3, section 3.5
enum system_segment_types
{
	TSS_32BIT = 9
};

enum privilege_level
{
	PRIVILEGE_KERNEL = 0,
	PRIVILEGE_USER = 3
};

static inline bool segment_is_user(uint16_t segment)
{
	return ((segment & 3) == PRIVILEGE_USER);
}

#else
// mirror enum privilege_level values for use in assembly code
# define PRIVILEGE_KERNEL	0
# define PRIVILEGE_USER		3
#endif // ! ASSEMBLY

// LDT flag in segment selector is always zero
#define make_segment_selector(rpl, index) \
	((index << 3) | (rpl & 3))

#endif
