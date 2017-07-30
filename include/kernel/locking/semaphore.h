#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

typedef int sem_t;

int semaphore_create(sem_t* sem, int n);
int semaphore_destroy(sem_t* sem);

int semaphore_up(const sem_t* sem);
int semaphore_down(const sem_t* sem);
int semaphore_getvalue(const sem_t* sem, int* val);

#endif
