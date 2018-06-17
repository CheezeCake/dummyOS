#ifndef _FS_DEVICE_H_
#define _FS_DEVICE_H_

#include <kernel/types.h>

enum device_major
{
	DEVICE_CHAR_TTY_MAJOR,
	DEVICE_CHAR_MAJOR_SENTINEL
};

enum device_minor
{
	DEVICE_CHAR_CONSOLE_MINOR,
	DEVICE_CHAR_SERIAL_CONSOLE_MINOR,
};

struct device
{
	enum device_major major;
	enum device_minor minor;
};

typedef int16_t dev_t;
typedef uint32_t mode_t;

#endif
