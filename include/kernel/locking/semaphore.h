#ifndef _KERNEL_LOCKING_SEMAPHORE_H_
#define _KERNEL_LOCKING_SEMAPHORE_H_

#include <kernel/thread_list.h>

typedef struct thread_list_synced thread_queue;

struct sem_t;
typedef struct sem_t sem_t;

int semaphore_create(sem_t* sem, int n);
int semaphore_destroy(sem_t* sem);

int semaphore_up(sem_t* sem);
int semaphore_down(sem_t* sem);
int semaphore_get_value(const sem_t* sem);

#endif
