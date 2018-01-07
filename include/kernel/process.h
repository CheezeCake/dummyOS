#ifndef _KERNEL_PROCESS_H_
#define _KERNEL_PROCESS_H_

#include <kernel/thread.h>
#include <kernel/vm_context.h>

#define PROCESS_NAME_MAX_LENGTH 16

typedef int pid_t;

/**
 * @brief Process
 */
struct process
{
	pid_t pid;
	char name[PROCESS_NAME_MAX_LENGTH];

	struct process* parent;
	list_t children;

	struct vm_context vm_ctx;

	struct thread_list threads;

	struct list_node p_list; /**< Chained in sched::@ref ::process_list */
};

int process_kprocess_init(void);
struct process* process_get_kprocess(void);

struct process* process_create(const char* name);
int process_init(struct process* proc, const char* name);
void process_destroy(struct process* proc);

int process_add_thread(struct process* proc, struct thread* thread);
int process_add_kthread(struct thread* kthread);

#endif
