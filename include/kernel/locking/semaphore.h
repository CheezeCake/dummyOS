#ifndef _KERNEL_LOCKING_SEMAPHORE_H_
#define _KERNEL_LOCKING_SEMAPHORE_H_

#include <kernel/atomic.h>
#include <kernel/sched/wait.h>

typedef struct sem_t
{
	atomic_int_t value;
	wait_queue_t wait_queue;
} sem_t;


/**
 * @brief Initializes the semaphore
 */
int semaphore_init(sem_t* sem, int n);

/**
 * @brief Destroys the semaphore
 *
 * Wakes up all the threads blocked by the semaphore
 */
int semaphore_destroy(sem_t* sem);

int semaphore_up(sem_t* sem);

int semaphore_down(sem_t* sem);

int semaphore_get_value(const sem_t* sem);

#endif
