#ifndef _KERNEL_THREAD_LIST_H_
#define _KERNEL_THREAD_LIST_H_

#include <kernel/thread.h>
#include <kernel/list.h>
#include <kernel/synced_list.h>
#include <kernel/locking/spinlock.h>
#include <kernel/kmalloc.h>
#include <kernel/libk.h>

struct thread_list_node
{
	struct thread* thread;

	LIST_NODE_CREATE(struct thread_list_node);
};

struct thread_list
{
	LIST_CREATE(struct thread_list_node);
};

struct thread_list_synced
{
	SYNCED_LIST_CREATE(struct thread_list_node);
};

static inline struct thread_list_node* thread_list_node_create(struct thread* thread)
{
	struct thread_list_node* node = kmalloc(sizeof(struct thread_list_node));
	if (!node)
		return NULL;

	memset(node, 0, sizeof(struct thread_list_node));
	node->thread = thread;

	return node;
}

#endif
