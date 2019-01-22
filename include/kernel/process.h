#ifndef _KERNEL_PROCESS_H_
#define _KERNEL_PROCESS_H_

#include <dummyos/const.h>
#include <fs/cache_node.h>
#include <fs/file.h>
#include <fs/tty.h>
#include <kernel/mm/vmm.h>
#include <kernel/signal.h>
#include <kernel/thread.h>
#include <kernel/types.h>
#include <kernel/sched/wait.h>

#define PROCESS_NAME_MAX_LENGTH 16

#define PROCESS_MAX_FD 16

enum process_state
{
	PROC_RUNNABLE,
	PROC_LOCKED,
	PROC_ZOMBIE
};

struct process_image
{
	v_addr_t entry_point;
	v_addr_t brk;
};

/**
 * @brief Process
 */
struct process
{
	pid_t pid;
	int session;
	bool session_leader;
	int pgrp;
	struct tty* ctrl_tty;

	char name[PROCESS_NAME_MAX_LENGTH];
	enum process_state state;
	struct process_image img;

	int exit_status;

	struct vfs_file* fds[PROCESS_MAX_FD];
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

void process_remove_thread(struct process* proc, struct thread* thr);

int process_fork(struct process* proc, const struct thread* fork_thread,
		 struct process** child, struct thread** child_thread);

/**
 * @brief Resets and free() a process object
 */
void process_destroy(struct process* proc);

int process_lock(struct process* proc, const struct thread* cur);

int process_unlock(struct process* proc);

void process_set_vmm(struct process* proc, struct vmm* vmm);

void process_set_name(struct process* proc, const char* name);

void process_set_process_image(struct process* proc,
			       const struct process_image* img);

/**
 * @brief Checks if the process has any pending signals
 */
bool process_signal_pending(const struct process* proc);

/**
 * @brief Sends a signal to a process
 */
int process_kill(pid_t pid, uint32_t sig);

/**
 * @brief Sends a signal to a process group
 */
int process_pgrp_kill(int pgrp, uint32_t sig);

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

/**
 * Prepares the process for execve
 */
void process_exec(struct process* proc);

/**
 * Adds file to the file descriptor array
 *
 * @return the index in the fd array
 *  < 0 on failure
 */
int process_add_file(struct process* proc, struct vfs_file* file);

/**
 * Adds file to the file descriptor array at index fd
 *
 * @return 0 on success
 */
int process_add_file_at(struct process* proc, struct vfs_file* file, int fd);

/**
 * Remove file identified by fd from the file descriptor table
 *
 * @parm removed optional: the removed file
 */
int process_remove_file(struct process* proc, int fd,
			struct vfs_file** removed);

/**
 * @brief Returns the vfs_file identified by the file descriptor
 */
struct vfs_file* process_get_file(struct process* proc, int fd);

/**
 * @brief Returns the process identified by pid
 */
struct process* process_get(pid_t pid);

/*
 * @brief Sets the controling tty for the process group identified by pgrp
 */
void process_set_tty_for_pgrp(int pgrp, struct tty* tty);

#endif
