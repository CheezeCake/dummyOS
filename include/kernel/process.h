#ifndef _KERNEL_PROCESS_H_
#define _KERNEL_PROCESS_H_

#include <kernel/cpu_context.h>
#include <kernel/thread.h>
#include <kernel/thread_list.h>
#include <kernel/vm_context.h>
#include <kernel/list.h>

#define PROCESS_NAME_MAX_LENGTH 16

typedef int pid_t;

struct process
{
	pid_t pid;
	char name[PROCESS_NAME_MAX_LENGTH];
	struct process* parent;

	struct vm_context vm_ctx;

	struct thread_list threads;

	LIST_NODE_CREATE(struct process);
};

void process_init(void);
int process_create(struct process* proc, const char* name);
int process_kprocess_create(struct process* proc, const char* name,
		start_func_t start);
void process_destroy(struct process* proc);

int process_add_thread(struct process* proc, struct thread* thread);

#endif
