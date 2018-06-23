#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/mm/uaccess.h>
#include <kernel/sched/sched.h>

/** @see kernel/mm/vmm.c */
extern v_addr_t __fixup_addr;

// this is used to avoid the compiler optimizing away the fixup label
int __false = 0;

#define uaccess_setup(fixup_label)				\
	do {										\
		__fixup_addr = (v_addr_t)&&fixup_label; \
		vmm_uaccess_setup();					\
		if (__false)							\
			goto fixup;							\
	} while (0)

#define uaccess_reset() \
	__fixup_addr = 0


static int uaccess_memcpy(void* to, const void* from, size_t n)
{
	char* dst = to;
	const char* s = from;

	uaccess_setup(fixup);

	for ( ; n > 0; ++dst, ++s, --n)
		*dst = *s;

	uaccess_reset();

	return 0;

fixup:
	uaccess_reset();
	return -EFAULT;
}

int copy_to_user(void* __user to, const void* from, size_t n)
{
	return uaccess_memcpy(to, from, n);
}

int copy_from_user(void* to, const void* __user from, size_t n)
{
	return uaccess_memcpy(to, from, n);
}

ssize_t strnlen_user(const char* __user str, ssize_t n)
{
	const char* s = str;

	uaccess_setup(fixup);

	while (n > 0 && *s) {
		++s;
		--n;
	}

	uaccess_reset();

	return (s - str);

fixup:
	uaccess_reset();
	return -EFAULT;
}

ssize_t uaccess_strlcpy(char* dest, const char* src, ssize_t n)
{
	const char* src0 = src;
	size_t s = n;

	uaccess_setup(fixup);

	for ( ; n > 1 && *src; ++dest, ++src, --n)
		*dest = *src;

	if (n == 0 || n == 1) {
		if (s != 0)
			*dest = '\0';

		while (*src)
			++src;
	}

	uaccess_reset();

	return (src - src0);

fixup:
	uaccess_reset();
	return -EFAULT;
}

ssize_t strlcpy_to_user(char* __user dest, const char* src, ssize_t n)
{
	return uaccess_strlcpy(dest, src, n);
}

ssize_t strlcpy_from_user(char* dest, const char* __user src, ssize_t n)
{
	return uaccess_strlcpy(dest, src, n);
}

int strndup_from_user(const char* __user str, ssize_t n, char** dup)
{
	char* copy;
	ssize_t size;

	size = strnlen_user(str, n);
	if (size <= 0)
		return size;

	copy = kmalloc(size + 1);
	if (!copy)
		return -ENOMEM;

	size = strlcpy_from_user(copy, str, size + 1);
	if (size < 0) {
		kfree(copy);
		return size;
	}

	*dup = copy;

	return 0;
}

int memset_user(void *s, int c, size_t size)
{
	char* s0 = s;

	uaccess_setup(fixup);

	for ( ; size > 0; ++s0, --size)
		*s0 = c;

	return 0;

fixup:
	uaccess_reset();
	return -EFAULT;
}
