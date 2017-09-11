#ifndef _KERNEL_SCHED_SCHED_H_
#define _KERNEL_SCHED_SCHED_H_

#include <stddef.h>
#include <kernel/thread.h>
#include <kernel/process.h>
#include <kernel/thread_list.h>

void sched_init(unsigned int quantum_in_ms);
void sched_start(void);

void sched_schedule(void);

void sched_add_process(struct process* proc);
int sched_add_thread(struct thread* thread);
void sched_add_thread_node(struct thread_list_node* node);

struct thread* sched_get_current_thread(void);
struct process* sched_get_current_process(void);
struct thread_list_node* sched_get_current_thread_node(void);

void sched_remove_current_thread(void);
void sched_yield_current_thread(void);
void sched_sleep_current_thread(unsigned int millis);
void sched_block_current_thread(void);

#endif
