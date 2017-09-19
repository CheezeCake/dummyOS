#ifndef _KERNEL_SCHED_SCHED_H_
#define _KERNEL_SCHED_SCHED_H_

#include <stddef.h>
#include <kernel/thread.h>
#include <kernel/process.h>
#include <kernel/thread_list.h>


#define SCHED_PRIORITY_LEVELS 5
#define SCHED_PRIORITY_LEVEL_MAX (SCHED_PRIORITY_LEVELS - 1)
#define SCHED_PRIORITY_LEVEL_MIN 0
#define SCHED_PRIORITY_LEVEL_DEFAULT 3

void sched_init(void);
void sched_start(void);

void sched_schedule(void);

int sched_add_process(const struct process* proc);
int sched_add_thread(struct thread* thread);

int sched_remove_thread(struct thread* thread);

struct thread* sched_get_current_thread(void);
struct process* sched_get_current_process(void);

void sched_remove_current_thread(void);
void sched_yield_current_thread(void);
void sched_sleep_current_thread(unsigned int millis);
void sched_block_current_thread(void);

thread_priority_t sched_get_priority(void);
int sched_set_priority(thread_priority_t priority);

#endif
