#ifndef _SEGMENT_H_
#define _SEGMENT_H_

#include <stdbool.h>

// indexes in GDT
#define KCODE 1
#define KDATA 2
#define UCODE 3
#define UDATA 4
#define TSS 5

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

// LDT flag in segment selector is always zero
#define make_segment_selector(rpl, index) \
	((index << 3) | (rpl & 3))

#endif
