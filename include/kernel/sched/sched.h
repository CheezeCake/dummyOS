#ifndef _KERNEL_SCHED_SCHED_H_
#define _KERNEL_SCHED_SCHED_H_

#include <kernel/process.h>
#include <kernel/thread.h>
#include <kernel/thread_list.h>
#include <kernel/types.h>


#define SCHED_PRIORITY_LEVELS 5
#define SCHED_PRIORITY_LEVEL_MAX (SCHED_PRIORITY_LEVELS - 1)
#define SCHED_PRIORITY_LEVEL_MIN 0
#define SCHED_PRIORITY_LEVEL_DEFAULT 3

void sched_init(void);
void sched_start(void);

struct cpu_context* sched_schedule_yield(struct cpu_context* cpu_ctx);
struct cpu_context* sched_schedule(struct cpu_context* cpu_ctx);

int sched_add_thread(struct thread* thread);
int sched_add_process(struct process* proc);

int sched_remove_thread(struct thread* thread);
int sched_remove_process(struct process* proc);

struct thread* sched_get_current_thread(void);
struct process* sched_get_current_process(void);

void sched_exit(void);

void sched_yield(void);

#if 0
#define sched_sleep(...) _Generic((__VA_ARGS__+0), \
								  unsigned int: sched_sleep_millis, \
								  default: sched_sleep_event)(__VA_ARGS__)
#endif
void sched_sleep_event(void);

void sched_sleep_millis(unsigned int millis);

#endif
