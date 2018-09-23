#ifndef _KERNEL_SCHED_REAPER_H_
#define _KERNEL_SCHED_REAPER_H_

struct thread;

void reaper_init(void);

void reaper_reap(struct thread* thr);

#endif
