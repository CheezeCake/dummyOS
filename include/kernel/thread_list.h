#ifndef _KERNEL_THREAD_LIST_H_
#define _KERNEL_THREAD_LIST_H_

#include <kernel/kmalloc.h>
#include <kernel/locking/spinlock.h>
#include <kernel/synced_list.h>
#include <kernel/thread.h>
#include <libk/libk.h>
#include <libk/list.h>

struct thread_list
{
	LIST_CREATE
};

struct thread_list_synced
{
	SYNCED_LIST_CREATE
};

#endif
