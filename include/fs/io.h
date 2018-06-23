#ifndef _FS_IO_H_
#define _FS_IO_H_

#include <kernel/types.h>

ssize_t _dirent_init(struct dirent* __user dirp, uint32_t d_ino,
					 uint8_t d_type, uint8_t d_namlen, char d_name[]);

#endif
