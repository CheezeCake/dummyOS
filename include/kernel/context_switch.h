#ifndef _KERNEL_CONTEXT_SWITCH_H_
#define _KERNEL_CONTEXT_SWITCH_H_

#include <kernel/thread.h>

void context_switch(const struct thread* from, const struct thread* to);

#endif

