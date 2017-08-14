#ifndef _KERNEL_THREAD_H_
#define _KERNEL_THREAD_H_

#include <stddef.h>
#include <arch/cpu_context.h>
#include <kernel/types.h>
#include <kernel/time/time.h>
#include <kernel/list.h>

#define MAX_THREAD_NAME_LENGTH 16

enum thread_state { THREAD_RUNNING, THREAD_READY, THREAD_BLOCKED };

struct thread
{
	char name[MAX_THREAD_NAME_LENGTH];

	struct cpu_context* cpu_context;

	v_addr_t stack;
	size_t stack_size;

	enum thread_state state;

	LIST_NODE_CREATE(struct thread);
};

int thread_create(struct thread* thread, const char* name, size_t stack_size,
		void (*start)(void), void (*exit)(void));
void thread_destroy(struct thread* thread);

void thread_yield(void);
void thread_sleep(unsigned int millis);

#endif
