#ifndef _KERNEL_PROCESS_H_
#define _KERNEL_PROCESS_H_

#include <dummyos/const.h>
#include <fs/cache_node.h>
#include <kernel/thread.h>
#include <kernel/vm_context.h>

#define PROCESS_NAME_MAX_LENGTH 16

typedef short int pid_t;

#define PROCESS_INIT_PID		1
#define PROCESS_KTHREADD_PID	2


/**
 * @brief Process
 */
struct process
{
	pid_t pid;
	char name[PROCESS_NAME_MAX_LENGTH];

	struct vfs_cache_node* cwd;
	struct vfs_cache_node* root;
	struct vfs_cache_node* exec;

	struct process* parent;
	list_t children;

	struct vm_context vm_ctx;

	thread_list_t threads;

	list_node_t p_list; /**< Chained in process.c::@ref ::process_list */
};

int process_kprocess_init(void);
struct process* process_get_kprocess(void);

int process_init(struct process* proc, const char* name, start_func_t start,
				 void* start_args);
int process_create(const char* name, start_func_t start, void* start_args,
				   struct process** result);

void process_prepare_exec(struct process* proc, const struct thread* save);
void process_destroy(struct process* proc);

int process_create_uthread(struct process* proc, const char* name,
						  start_func_t __user start, void* __user start_args);

int process_create_kthread(const char* name, start_func_t start,
						   void* start_args, struct thread** result);

struct thread* process_get_initial_thread(const struct process* proc);

#endif
