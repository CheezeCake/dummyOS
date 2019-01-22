#ifndef _KERNEL_KTHREAD_H_
#define _KERNEL_KTHREAD_H_

#include <dummyos/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/thread.h>
#include <libk/libk.h>

static inline int kthread_create(const char* name, void (*start)(void* data),
				 void* data, struct thread** result)
{
	char* name_dup;
	int err;

	name_dup = strdup(name);
	if (!name_dup)
		return -ENOMEM;

	err = __kthread_create(name_dup, (v_addr_t)start, result);
	if (!err)
		(*result)->kthr_data = data;
	else
		kfree(name_dup);

	return err;
}

#endif
