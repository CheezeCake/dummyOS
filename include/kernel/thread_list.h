#ifndef _THREAD_LIST_H_
#define _THREAD_LIST_H_

#include <kernel/thread.h>
#include <kernel/list.h>
#include <kernel/synced_list.h>
#include <kernel/locking/spinlock.h>

struct thread_list
{
	LIST_CREATE(struct thread);
};

struct thread_list_synced
{
	SYNCED_LIST_CREATE(struct thread);
};

#endif
