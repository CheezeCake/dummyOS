#ifndef _THREAD_H_
#define _THREAD_H_

#include <stddef.h>
#include <arch/cpu_context.h>
#include <kernel/types.h>

#define MAX_THREAD_NAME_LENGTH 16

struct thread
{
	char name[MAX_THREAD_NAME_LENGTH];

	struct cpu_context* cpu_context;

	v_addr_t stack;
	size_t stack_size;

	struct thread* prev;
	struct thread* next;
};

int thread_create(struct thread* thread, const char* name, size_t stack_size,
		void (start(void)), void (exit(void)));
void thread_destroy(struct thread* thread);

#endif
