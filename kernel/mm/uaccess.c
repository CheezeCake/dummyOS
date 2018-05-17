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

ssize_t strncpy_from_user(char* dest, const char* __user src, ssize_t n)
{
	char* dst = dest;

	uaccess_setup(fixup);

	for ( ; *src && n > 0; ++dest, ++src, --n)
		*dest = *src;

	// null pad
	for (; n > 0; ++dest, --n)
		*dest = '\0';

	uaccess_reset();

	return (dest - dst);

fixup:
	uaccess_reset();
	return -EFAULT;
}

char* strndup_from_user(const char* __user str, ssize_t n)
{
	char* copy;
	ssize_t size;

	size = strnlen_user(str, n);
	if (size <= 0)
		return NULL;

	copy = kmalloc(size);
	if (!copy)
		return NULL;

	size = strncpy_from_user(copy, str, size);
	if (size < 0) {
		kfree(copy);
		return NULL;
	}

	return copy;
}
