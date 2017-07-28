#ifndef _SZ_LIST_H_
#define _SZ_LIST_H_

#include <kernel/list.h>

/*
 * **************
 * list with size
 * **************
 */

/*
 * init
 */
#define sz_list_init_null(list) \
	do {						\
		list_init_null(list);	\
		(list)->size = 0;		\
	} while (0)

#define sz_list_init(list, value)			\
	do {									\
		list_init_null_value(list, value);	\
		(list)->size = 0;					\
	} while (0)

/*
 * element access
 */
#define sz_list_front(list) list_front(list)

#define sz_list_back(list) list_back(list)


/*
 * capacity
 */
#define sz_list_empty(list) list_empty(list)


/*
 * modifiers
 */
#define sz_list_push_front(list, value)	\
	do {								\
		list_push_front(list, value);	\
		++(list)->size;					\
	} while (0)

#define sz_list_push_back(list, value)	\
	do {								\
		list_push_back(list, value);	\
		++(list)->size;					\
	} while (0)

#define sz_list_pop_front(list)	\
	do {						\
		list_pop_front(list);	\
		--(list)->size;			\
	} while (0)

#define sz_list_pop_back(list)	\
	do {						\
		list_pop_back(list);	\
		--(list)->size;			\
	} while(0)

#define sz_list_erase(list, value)	\
	do {							\
		list_erase(list, value);	\
		--(list)->size;				\
	} while (0)
	
#endif
