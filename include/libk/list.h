#ifndef _LIBK_LIST_H_
#define _LIBK_LIST_H_

/*
 * struct node {
 *		struct node* next;
 *		struct node* prev;
 * };
 *
 * struct list {
 *		struct node head;
 *		struct node tail;
 * };
 */

#define LIST_NODE_CREATE(node_type)	\
	node_type* prev;				\
	node_type* next

#define LIST_CREATE(node_type)	\
	node_type* head;			\
	node_type* tail


/*
 * init
 */
#define list_init_null(list)	\
	do {						\
		(list)->head = NULL;	\
		(list)->tail = NULL;	\
	} while (0)

#define list_init(list, value)	\
	do {						\
		value->prev = NULL;		\
		value->next = NULL;		\
		(list)->head = value;	\
		(list)->tail = value;	\
	} while (0)


/*
 * element access
 */
#define list_front(list) ((list)->head)

#define list_back(list) ((list)->tail)

/*
 * iterators
 */
#define list_begin(list) list_front(list)

#define list_end(list) list_back(list)

#define list_it_next(it) ((it)->next)

#define list_it_prev(it) ((it)->pev)


/*
 * capacity
 */
#define list_empty(list) ((list)->head == NULL)


/*
 * modifiers
 */
#define list_push_front(list, value)	\
	do {								\
		if (list_empty(list)) {			\
			list_init(list, value);		\
		}								\
		else {							\
			value->prev = NULL;			\
			value->next = (list)->head;	\
			(list)->head->prev = value;	\
			(list)->head = value;		\
		}								\
	} while (0)

#define list_push_back(list, value)		\
	do {								\
		if (list_empty(list)) {			\
			list_init(list, value);		\
		}								\
		else {							\
			value->prev = (list)->tail;	\
			value->next = NULL;			\
			(list)->tail->next = value;	\
			(list)->tail = value;		\
		}								\
	} while (0)

#define list_pop_front(list)				\
	do {									\
		if ((list)->head == (list)->tail)	\
			(list)->tail = NULL;			\
		(list)->head = (list)->head->next;	\
	} while (0)

#define list_pop_back(list)					\
	do {									\
		if ((list)->tail == (list)->head)	\
			(list)->head = NULL;			\
		(list)->tail = (list)->tail->prev;	\
	} while(0)

#define list_erase(list, value)					\
	do {										\
		if ((list)->head == value)				\
			(list)->head = (list)->head->next;	\
		if ((list)->tail == value)				\
			(list)->tail = (list)->tail->prev;	\
		if (value->prev)						\
			value->prev->next = value->next;	\
		if (value->next)						\
			value->next->prev = value->prev;	\
	} while (0)

#define list_clear(list) list_init_null(list)


/*
 * iterators
 */
#define list_foreach(list, it) for (it = (list)->head; it; it = it->next)

#define list_foreach_reverse(list, it) for (it = (list)->tail; it; it = it->prev)

#endif
