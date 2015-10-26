#ifndef _SCHED_H_
#define _SCHED_H_

#include <stddef.h>
#include <kernel/list.h>
#include <kernel/thread.h>

struct thread_list
{
	struct thread* root;
};

void sched_init(unsigned int quantum_in_ms);
void sched_add_thread(struct thread* thread);
void sched_remove_current_thread(void);
void sched_schedule(void);
void sched_start(void);

#endif
