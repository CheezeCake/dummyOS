#ifndef _KLIST_H_
#define _KLIST_H_

#define list_init(list, value) _list_init(list, value, root, prev, next)
#define _list_init(list, value, root, prev, next) { \
	(list)->root = (value); \
	if ((value)) { \
		(value)->prev = (value); \
		(value)->next = (value); \
	} \
}

#define list_push_back(list, value) _list_push_back(list, value, root, prev, next)
#define _list_push_back(list, value, root, prev, next) { \
	if ((list)->root) { \
		(value)->next = (list)->root; \
		(value)->prev = (list)->root->prev; \
		(list)->root->prev->next = (value); \
		(list)->root->prev = (value); \
	} \
	else { \
		_list_init(list, value, root, prev, next); \
	} \
}

#define list_push_front(list, value) _list_push_front(list, value, root, prev, next)
#define _list_push_front(list, value, root, prev, next) { \
	if ((list)->root) { \
		(value)->next = (list)->root; \
		(value)->prev = (list)->root->prev; \
		(list)->root->prev->next = (value); \
		(list)->root->prev = (value); \
		(list)->root = (value); \
	} \
	else { \
		_list_init(list, value, root, prev, next) \
	} \
}

#define list_pop_back(list) _list_pop_back(list, root, prev, next)
#define _list_pop_back(list, root, prev, next) { \
	if ((list)->root->prev == (list)->root) { \
		(list)->root = NULL; \
	} \
	else { \
		(list)->root->prev = (list)->root->prev->prev; \
		(list)->root->prev->next = (list)->root; \
	} \
}

#define list_pop_front(list) _list_pop_front(list, root, prev, next)
#define _list_pop_front(list, root, prev, next) { \
	if ((list)->root->next == (list)->root) { \
		(list)->root = NULL; \
	} \
	else { \
		(list)->root->prev->next = (list)->root->next; \
		(list)->root->next->prev = (list)->root->prev; \
		(list)->root = (list)->root->next; \
	} \
}

#define list_front(list) _list_front(list, root)
#define _list_front(list, root) ((list)->root)

#define list_back(list) _list_back(list, root, prev)
#define _list_back(list, root, prev) ((list)->root->prev)


// list with size
#define slist_init(list, value) _slist_init(list, value, root, prev, next, size)
#define _slist_init(list, value, root, prev, next, size) { \
	(list)->root = (value); \
	if ((value)) { \
		(value)->prev = (value); \
		(value)->next = (value); \
		(list)->size = 0; \
	} \
	else { \
		(list)->size = 1; \
	} \
}

#define slist_push_back(list, value) _slist_push_back(list, value, root, prev, next, size)
#define _slist_push_back(list, value, root, prev, next, size) {\
	_list_push_back(list, (value), root, prev, next); \
	++(list)->size; }

#define slist_push_front(list, value) _slist_push_back(list, value, root, prev, next, size)
#define _slist_push_font(list, value, root, prev, next, size) { \
	_list_push_font(list, (value), root, prev, next); \
	++(list)->size; }

#define slist_pop_front(list) _slist_pop_front(list, root, prev, next, size)
#define _slist_pop_front(list, root, prev, next, size) { \
	_list_pop_front(list, root, prev, next); \
	--(list)->size; }

#define slist_pop_back(list) _slist_pop_back(list, root, prev, next, size)
#define _slist_pop_back(list, root, prev, next, size) { \
	_list_pop_back(list, root, prev, next); \
	--(list)->size; }

#define list_erase(list, iterator) _list_erase(list, iterator, root, prev, next)
#define _list_erase(list, iterator, root, prev, next) \
	if ((list)->root == iterator && (list)->root->next == iterator) { \
			(list)->root = NULL; \
	} \
	else { \
		if (iterator->prev) iterator->prev->next = iterator->next; \
		if (iterator->next) iterator->next->prev = iterator->prev; \
		if ((list)->root == iterator) (list)->root = iterator->next; \
	}

#define list_empty(list) (!(list)->root)

#define list_foreach(list, it, nb) _list_foreach(list, it, next, nb)
#define _list_foreach(list, it, next, nb) \
	for (nb = 0, (it) = (list)->root; \
			(it) != (list)->root || nb == 0; \
			(it) = (it)->next, ++nb)

#endif
