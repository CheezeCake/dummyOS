#ifndef _KERNEL_SCHED_WAIT_H_
#define _KERNEL_SCHED_WAIT_H_

#include <stdbool.h>

#include <kernel/thread_list.h>

typedef struct list_node wait_queue_entry_t;
typedef struct thread_list_synced wait_queue_t;

struct thread;

int wait_create(wait_queue_t* wq);
int wait_wait(wait_queue_t* wq);
int wait_wake(wait_queue_t* wq, unsigned int nb_threads);
int wait_wake_all(wait_queue_t* wq);
bool wait_empty(const wait_queue_t* wq);

#endif
