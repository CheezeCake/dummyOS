#ifndef _LIBK_LIST_H_
#define _LIBK_LIST_H_

#include <kernel/types.h>
#include <libk/utils.h>

typedef struct list_node {
	struct list_node* prev;
	struct list_node* next;
} list_node_t;

typedef struct list_node list_t;


/*
 * init
 */
#define LIST_DEFINE(list) list_t list = { .prev = &list, .next = &list }

static inline void list_init(list_t* list)
{
	list->prev = list;
	list->next = list;
}


/*
 * element access
 */
static inline list_node_t* list_front(const list_t* list)
{
	return list->next;
}

static inline list_node_t* list_back(const list_t* list)
{
	return list->prev;
}


/*
 * get enclosing struct
 */
#define list_entry(ptr, type, member) \
		container_of(ptr, type, member)


/*
 * iterators
 */
static inline list_node_t* list_begin(list_t* list)
{
	return list_front(list);
}

static inline list_node_t* list_end(list_t* list)
{
	return list;
}

static inline list_node_t* list_it_next(list_node_t* it)
{
	return it->next;
}


/*
 * capacity
 */
static inline bool list_empty(const list_t* list)
{
	return (list->next == list);
}


/*
 * modifiers
 */
static inline void list_insert_after(list_node_t* it, list_node_t* value)
{
	value->prev = it;
	value->next = it->next;
	it->next->prev = value;
	it->next = value;
}

static inline void list_push_front(list_t* list, list_node_t* value)
{
	list_insert_after(list, value);
}

static inline void list_insert_before(list_node_t* it, list_node_t* value)
{
	value->prev = it->prev;
	value->next = it;
	it->prev->next = value;
	it->prev = value;
}

static inline void list_push_back(list_t* list, list_node_t* value)
{
	list_insert_before(list, value);
}

static inline void list_pop_front(list_t* list)
{
	list_node_t* del = list->next;

	list->next = del->next;
	list->next->prev = list;
	del->prev = NULL;
	del->next = NULL;
}

static inline void list_pop_back(list_t* list)
{
	list_node_t* del = list->prev;

	list->prev = del->prev;
	list->prev->next = list;
	del->prev = NULL;
	del->next = NULL;
}

static inline void list_erase(list_t* list, list_node_t* del)
{
	del->prev->next = del->next;
	del->next->prev = del->prev;
	del->prev = NULL;
	del->next = NULL;
}

static inline void list_clear(list_t* list)
{
	list_init(list);
}


/*
 * foreach
 */
#define list_foreach(list, it) \
	for (it = (list)->next; it != (list); it = it->next)

#define list_foreach_reverse(list, it) \
	for (it = (list)->prev; it != (list); it = it->prev)

#define list_foreach_safe(list, it, next)		\
	for (it = (list)->next, next = it->next;	\
		 it != (list);							\
		 it = next, next = next->next)

#endif
