#ifndef _KERNEL_MM_UACCESS_H_
#define _KERNEL_MM_UACCESS_H_

#include <dummyos/const.h>
#include <kernel/types.h>

int copy_to_user(void* __user to, const void* from, size_t n);

int copy_from_user(void* to, const void* __user from, size_t n);

ssize_t strnlen_user(const char* __user str, ssize_t n);

ssize_t strlcpy_to_user(char* __user dest, const char* src, ssize_t n);

ssize_t strlcpy_from_user(char* dest, const char* __user src, ssize_t n);

int strndup_from_user(const char* __user str, ssize_t n, char** dup);

int memset_user(void *s, int c, size_t size);

#endif
