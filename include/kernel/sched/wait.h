#ifndef _KERNEL_SCHED_WAIT_H_
#define _KERNEL_SCHED_WAIT_H_

#include <kernel/thread_list.h>
#include <kernel/types.h>

typedef list_node_t wait_queue_entry_t;
typedef struct wait_queue
{
	thread_list_t threads;
} wait_queue_t;

struct thread;

int wait_init(wait_queue_t* wq);

void wait_reset(wait_queue_t* wq);

int wait_wait(wait_queue_t* wq);

int wait_wake(wait_queue_t* wq, unsigned int nb_threads);

int wait_wake_all(wait_queue_t* wq);

bool wait_empty(const wait_queue_t* wq);

#endif
