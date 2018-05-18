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
	for (pid_t pid = 1; pid < PROCESS_TABLE_SIZE; ++pid) {
		if (!process_table[pid])
			return pid;
	}

	return -1;
}

pid_t process_register_pid(struct process* proc, pid_t pid)
{
	if (pid > 0 && pid < PROCESS_TABLE_SIZE && !process_table[pid]) {
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

	strlcpy(proc->name, name, PROCESS_NAME_MAX_LENGTH);
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
		goto fail_thr_clone;
	process_add_thread(new, *child_thread);
	thread_unref(*child_thread);

	// TODO: copy root, cwd, file descriptors, ...

	signal_copy(new->signals, proc->signals);

	new->parent = proc;
	list_push_back(&proc->children, &new->p_child);

	*child = new;

	return 0;

fail_thr_clone:
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

void process_set_vmm(struct process* proc, struct vmm* vmm)
{
	irq_disable();

	proc->vmm = vmm;

	irq_enable();
}

bool process_signal_pending(const struct process* proc)
{
	return (proc) ? signal_pending(proc->signals) : false;
}

int process_kill(pid_t pid, uint32_t sig)
{
	struct process* proc = process_table[pid];
	if (proc) {
		if (list_empty(&proc->threads))
			return -EINVAL;

		signal_send(proc->signals, sig, NULL, sched_get_current_process());
		return 0;
	}

	return -EINVAL;
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

void process_destroy(struct process* proc)
{
	vmm_unref(proc->vmm);
	signal_destroy(proc->signals);
	wait_reset(&proc->wait_wq);
	destroy_threads(proc);
	unregister_process(proc);

	remove_from_parent_list(proc);
	kfree(proc);
}
