#include <dummyos/const.h>
#include <fs/device.h>

int mknodat(int dirfd, const char* __user pathname, mode_t mode, dev_t dev)
{
	enum device_major major = dev >> 16;
	enum device_major minor = dev & 0xffff;

	return 0;
}
