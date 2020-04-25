#ifndef _DUMMYOS_TYPES_H_
#define _DUMMYOS_TYPES_H_

#include <stddef.h>
#include <stdint.h>

typedef long blkcnt_t;

typedef long blksize_t;

typedef uint64_t fsblkcnt_t;

typedef uint32_t fsfilcnt_t;

typedef long off_t;

typedef int pid_t;

typedef int16_t dev_t;

typedef unsigned short uid_t;

typedef unsigned short gid_t;

typedef uint32_t id_t;

typedef unsigned long ino_t;

typedef uint32_t mode_t;

typedef long ssize_t;

typedef int64_t time_t;

typedef unsigned short nlink_t;

#endif
