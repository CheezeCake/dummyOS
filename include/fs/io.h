#ifndef _FS_IO_H_
#define _FS_IO_H_

#include <kernel/types.h>

ssize_t _dirent_init(struct dirent* __user dirp, long d_ino,
					 int d_type, size_t d_namlen, const char d_name[]);

#endif
