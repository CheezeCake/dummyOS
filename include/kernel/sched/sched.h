#ifndef _KERNEL_SCHED_SCHED_H_
#define _KERNEL_SCHED_SCHED_H_

#include <stddef.h>
#include <kernel/thread.h>
#include <kernel/process.h>
#include <kernel/thread_list.h>

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

#endif
