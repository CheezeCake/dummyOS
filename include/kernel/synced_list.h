#ifndef _KSYNCED_LIST_H_
#define _KSYNCED_LIST_H_

#include <kernel/list.h>
#include <kernel/locking/spinlock.h>

#define SYNCED_LIST_CREATE(node_type)	\
	LIST_CREATE(node_type);				\
	spinlock_t lock

#define list_lock_synced(list) spinlock_lock((list)->lock)

#define list_unlock_synced(list) spinlock_unlock((list)->lock)

#define list_locked(list) spinlock_locked((list)->lock)

#define __synced_list_(list, op)	\
	do {							\
		list_lock_synced(list);		\
		list_##op(list);			\
		list_unlock_synced(list);	\
	} while (0)

#define __synced_list_value_(list, op, value)	\
	do {										\
		list_lock_synced(list);					\
		list_##op(list, value);					\
		list_unlock_synced(list);				\
	} while (0)


/*
 * init
 */
#define list_init_null_synced(list)	\
	do {							\
		list_init_null(list);		\
		(list)->lock = 0;			\
	} while (0)

#define list_init_synced(list, value)	\
	do {								\
		list_init(list, value);			\
		(list)->lock = 0;				\
	} while (0)


/*
 * modifiers
 */
#define list_push_front_synced(list, value) __synced_list_value_(list, push_front, value)

#define list_push_back_synced(list, value) __synced_list_value_(list, push_back, value)

#define list_pop_front_synced(list) __synced_list_(list, pop_front)

#define list_pop_back_synced(list) __synced_list_(list, pop_back)

#define list_erase_synced(list, value) __synced_list_value_(list, erase, value)

#define list_clear_synced(list) __synced_list_(list, clear)

#endif
