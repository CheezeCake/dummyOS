#include <fs/vfs.h>
#include <kernel/errno.h>
#include <kernel/interrupt.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <kernel/process.h>
#include <kernel/sched/sched.h>
#include <libk/libk.h>

#define INVALID_PID	0
#define PID_MIN		PROCESS_INIT_PID

#define PROCESS_TABLE_SIZE 64
static struct process* process_table[PROCESS_TABLE_SIZE + 1] = { NULL, };


static pid_t find_free_pid(void)
{
	for (pid_t pid = 1; pid <= PROCESS_TABLE_SIZE; ++pid) {
		if (!process_table[pid])
			return pid;
	}

	return -1;
}

pid_t process_register_pid(struct process* proc, pid_t pid)
{
	if (pid > 0 && pid <= PROCESS_TABLE_SIZE && !process_table[pid]) {
		process_table[pid] = proc;
		proc->pid = pid;
		return pid;
	}

	return -ENOSPC;
}

pid_t process_register(struct process* proc)
{
	pid_t pid;

	irq_disable();

	pid = process_register_pid(proc, find_free_pid());

	irq_enable();

	return pid;
}

static inline void unregister_process(struct process* proc)
{
	process_table[proc->pid] = NULL;
}

static int process_init(struct process* proc, const char* name)
{
	int err;

	memset(proc, 0, sizeof(struct process));

	err = vmm_create(&proc->vmm);
	if (err)
		return err;
	log_printf("%s() proc=%s vmm=%p\n", __func__, name, (void*)proc->vmm);

	err = signal_create(&proc->signals);
	if (err)
		return err;
	wait_init(&proc->wait_wq);

	process_set_name(proc, name);
	proc->state = PROC_RUNNABLE;
	proc->root = vfs_cache_node_get_root();
	proc->parent = NULL;

	list_init(&proc->children);
	list_init(&proc->threads);

	return err;
}

int process_create(const char* name, struct process** result)
{
	struct process* proc;
	int err;

	proc = kmalloc(sizeof(struct process));
	if (!proc)
		return -ENOMEM;

	err = process_init(proc, name);
	if (err) {
		kfree(proc);
		proc = NULL;
	}

	if (result)
		*result = proc;

	return err;
}

static int copy_fds(const struct process* proc, struct process* child)
{
	struct vfs_file* file;
	struct vfs_file* copy;
	int err;

	for (size_t i = 0; i < PROCESS_MAX_FD; ++i) {
		file = proc->fds[i];

		if (file) {
			copy = kmalloc(sizeof(struct vfs_file));
			if (!copy)
				return -ENOMEM;

			err = vfs_inode_open(file->inode, file->flags, copy);
			if (err) {
				kfree(copy);
				return err;
			}

			child->fds[i] = copy;
		}
	}

	return 0;
}

int process_fork(struct process* proc, const struct thread* fork_thread,
				 struct process** child, struct thread** child_thread)
{
	struct process* new;
	int err;

	err = process_create(proc->name, &new);
	if (err)
		return err;

	err = thread_clone(fork_thread, new->name, child_thread);
	if (err)
		goto fail;
	process_add_thread(new, *child_thread);
	thread_unref(*child_thread);

	err = copy_fds(proc, new);
	if (err)
		goto fail;

	new->root = proc->root;
	vfs_cache_node_ref(new->root);
	new->cwd = proc->cwd;
	vfs_cache_node_ref(new->cwd);

	new->session = proc->session;
	new->pgrp = proc->pgrp;
	new->ctrl_tty = proc->ctrl_tty;

	signal_copy(new->signals, proc->signals);

	new->parent = proc;
	list_push_back(&proc->children, &new->p_child);

	*child = new;

	return 0;

fail:
	process_destroy(new);

	return err;
}

static int lock_threads(struct process* proc, const struct thread* cur)
{
	int err;
	list_node_t* it;

	list_foreach(&proc->threads, it) {
		struct thread* thread = list_entry(it, struct thread, p_thr_list);

		if (cur != thread) {
			err = sched_remove_thread(thread);
			if (err)
				return err;
		}
	}

	return 0;
}

int process_lock(struct process* proc, const struct thread* cur)
{
	int err;

	irq_disable();

	err = lock_threads(proc, cur);
	if (!err)
		proc->state = PROC_LOCKED;

	irq_enable();

	return err;
}

static int unlock_threads(struct process* proc)
{
	int err;
	list_node_t* it;

	list_foreach(&proc->threads, it) {
		struct thread* thread = list_entry(it, struct thread, p_thr_list);

		if (thread->state == THREAD_READY) {
			err = sched_add_thread(thread);
			if (err)
				return err;
		}
	}

	return 0;
}

int process_unlock(struct process* proc)
{
	proc->state = PROC_RUNNABLE;
	return unlock_threads(proc);
}

int process_add_thread(struct process* proc, struct thread* thr)
{
	kassert(thr->type == UTHREAD);

	list_push_back(&proc->threads, &thr->p_thr_list);
	thread_ref(thr);

	thr->name = proc->name;
	thr->process = proc;

	return 0;
}

void process_remove_thread(struct process* proc, struct thread* thr)
{
	list_erase(&thr->p_thr_list);
}

void process_set_vmm(struct process* proc, struct vmm* vmm)
{
	irq_disable();

	proc->vmm = vmm;

	irq_enable();
}

void process_set_name(struct process* proc, const char* name)
{
	strlcpy(proc->name, name, PROCESS_NAME_MAX_LENGTH);
}

void process_set_process_image(struct process* proc,
							   const struct process_image* img)
{
	memcpy(&proc->img, img, sizeof(struct process_image));
}

bool process_signal_pending(const struct process* proc)
{
	return (proc) ? signal_pending(proc->signals) : false;
}

static bool process_has_ready_thread(const struct process* proc)
{
	list_node_t* it;

	list_foreach(&proc->threads, it) {
		struct thread* thr = list_entry(it, struct thread, p_thr_list);
		if (thread_get_state(thr) == THREAD_READY)
			return true;
	}

	return false;
}

static int process_intr_sleeping_thread(const struct process* proc)
{
	list_node_t* it;

	list_foreach(&proc->threads, it) {
		struct thread* thr = list_entry(it, struct thread, p_thr_list);
		if (thread_get_state(thr) == THREAD_SLEEPING)
			return thread_intr_sleep(thr);
	}

	return -ESRCH;
}

int process_kill(pid_t pid, uint32_t sig)
{
	struct process* proc = process_table[pid];
	if (proc) {
		if (list_empty(&proc->threads))
			return -EINVAL;

		irq_disable();
		// try to interrupt a sleeping thread if no other thread
		// can handle the signal
		if (!process_has_ready_thread(proc))
			process_intr_sleeping_thread(proc);
		irq_enable();

		signal_send(proc->signals, sig, NULL, sched_get_current_process());

		return 0;
	}

	return -EINVAL;
}

int process_pgrp_kill(int pgrp, uint32_t sig)
{
	if (pgrp <= 0)
		return -EINVAL;

	for (size_t i = 1; i <= PROCESS_TABLE_SIZE; ++i) {
		if (process_table[i] && process_table[i]->pgrp == pgrp)
			process_kill(i, sig);
	}

	return 0;
}

static inline void threads_foreach(struct process* proc,
								   void (*f)(struct thread*))
{
	list_node_t* it;
	list_node_t* next;

	list_foreach_safe(&proc->threads, it, next)
		f(list_entry(it, struct thread, p_thr_list));
}

static void exit_thread(struct thread* thread)
{
	thread_set_state(thread, THREAD_DEAD);
	sched_remove_thread(thread);
}

static void exit_threads(struct process* proc)
{
	threads_foreach(proc, exit_thread);
}

int process_exit_quiet(struct process* proc)
{
	exit_threads(proc);
	return 0;
}

int process_exit(struct process* proc, int status)
{
	process_exit_quiet(proc);

	proc->state = PROC_ZOMBIE;
	proc->exit_status = status;

	if (proc->parent) {
		process_kill(proc->parent->pid, SIGCHLD);
		wait_wake_all(&proc->parent->wait_wq);
	}

	if (proc->session_leader && proc->ctrl_tty)
		tty_reset_pgrp(proc->ctrl_tty);

	return 0;
}

static void destroy_thread(struct thread* thread)
{
	list_erase(&thread->p_thr_list);
	thread_unref(thread);
}

static inline void destroy_threads(struct process* proc)
{
	threads_foreach(proc, destroy_thread);
}

static inline void remove_from_parent_list(struct process* proc)
{
	if (proc->parent)
		list_erase(&proc->p_child);
}

static void close_fds(struct process* proc)
{
	for (int i = 0; i < PROCESS_MAX_FD; ++i) {
		if (proc->fds[i])
			vfs_close(proc->fds[i]);
	}
}

void process_destroy(struct process* proc)
{
	vmm_unref(proc->vmm);
	signal_destroy(proc->signals);
	wait_reset(&proc->wait_wq);
	destroy_threads(proc);
	unregister_process(proc);
	close_fds(proc);
	if (proc->root)
		vfs_cache_node_unref(proc->root);
	if (proc->cwd)
		vfs_cache_node_unref(proc->cwd);

	remove_from_parent_list(proc);
	kfree(proc);
}

void process_exec(struct process* proc)
{
	wait_reset(&proc->wait_wq);

	exit_threads(proc);
	destroy_threads(proc);
}

int process_add_file(struct process* proc, struct vfs_file* file)
{
	int ret = -1;

	for (int i = 0; i < PROCESS_MAX_FD; ++i) {
		irq_disable();

		if (!proc->fds[i]) {
			proc->fds[i] = file;
			ret = i;
		}

		irq_enable();

		if (ret >= 0)
			return ret;
	}

	return -EMFILE;
}

struct vfs_file* process_remove_file(struct process* proc, int fd)
{
	struct vfs_file* file = NULL;

	if (fd < PROCESS_MAX_FD) {
		irq_disable();

		file = proc->fds[fd];
		proc->fds[fd] = NULL;

		irq_enable();
	}

	return file;
}

struct vfs_file* process_get_file(struct process* proc, int fd)
{
	return (fd < PROCESS_MAX_FD) ? proc->fds[fd] : NULL;
}

struct process* process_get(pid_t pid)
{
	return (pid > 0 && pid <= PROCESS_TABLE_SIZE) ? process_table[pid] : NULL;
}

void process_set_tty_for_pgrp(int pgrp, struct tty* tty)
{
	if (pgrp == 0)
		return;

	for (size_t i = 1; i <= PROCESS_TABLE_SIZE; ++i) {
		if (process_table[i] && process_table[i]->pgrp == pgrp)
			process_table[i]->ctrl_tty = tty;
	}
}
