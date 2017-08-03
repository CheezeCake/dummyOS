#ifndef _KERNEL_SCHED_H_
#define _KERNEL_SCHED_H_

#include <stddef.h>
#include <kernel/thread.h>

void sched_init(unsigned int quantum_in_ms);
void sched_add_thread(struct thread* thread);
struct thread* sched_get_current_thread(void);
void sched_remove_current_thread(void);
void sched_schedule(void);
void sched_start(void);
void sched_yield_current_thread(void);
void sched_sleep_current_thread(unsigned int millis);
void sched_block_current_thread(void);

#endif
