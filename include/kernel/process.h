#ifndef _KERNEL_PROCESS_H_
#define _KERNEL_PROCESS_H_

#include <dummyos/const.h>
#include <fs/cache_node.h>
#include <kernel/mm/vmm.h>
#include <kernel/signal.h>
#include <kernel/thread.h>
#include <kernel/types.h>
#include <kernel/sched/wait.h>

#define PROCESS_NAME_MAX_LENGTH 16


enum process_state
{
	PROC_RUNNABLE,
	PROC_STOPPED,
	PROC_LOCKED,
	PROC_ZOMBIE
};

/**
 * @brief Process
 */
struct process
{
	pid_t pid;
	char name[PROCESS_NAME_MAX_LENGTH];
	enum process_state state;
	int exit_status;

	struct vfs_cache_node* cwd;
	struct vfs_cache_node* root;

	struct process* parent;
	list_t children;

	struct vmm* vmm;

	struct signal_manager* signals;
	wait_queue_t wait_wq;

	thread_list_t threads;

	list_node_t p_child; /**< Chained in process::children */
};

/**
 * @brief Creates a process object
 *
 * Creates a new process and places it in the process table
 *
 * @param result the created process
 */
int process_create(const char* name, struct process** result);

pid_t process_register(struct process* proc);

pid_t process_register_pid(struct process* proc, pid_t pid);

int process_add_thread(struct process* proc, struct thread* thr);

int process_fork(struct process* proc, const struct thread* fork_thread,
				 struct process** child, struct thread** child_thread);

/**
 * @brief Resets and free() a process object
 */
void process_destroy(struct process* proc);

int process_lock(struct process* proc, const struct thread* cur);

int process_unlock(struct process* proc);

void process_set_vmm(struct process* proc, struct vmm* vmm);

/**
 * @brief Checks if the process has any pending signals
 */
bool process_signal_pending(const struct process* proc);

/**
 * @brief Sends a signal to a process
 */
int process_kill(pid_t pid, uint32_t sig);

/**
 * @brief Exits without sending SIGCHLD
 */
int process_exit_quiet(struct process* proc);

/**
 * @brief Exit
 *
 * Sends SIGCHLD to parent and wakes up the parents wait(2)'ing threads
 */
int process_exit(struct process* proc, int status);

#endif
