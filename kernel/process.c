#include <kernel/errno.h>
#include <kernel/exec.h>
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


static void destroy_threads_except(struct process* proc,
								   const struct thread* save)
{
	list_node_t* it;
	list_node_t* next;

	list_foreach_safe(&proc->threads, it, next) {
		struct thread* thread = list_entry(it, struct thread, p_thr_list);
		if (thread != save) {
			thread_unref(thread);
			list_erase(&proc->threads, &thread->p_thr_list);
		}
	}

	/* list_clear(&proc->threads); */
}

static void destroy_threads(struct process* proc)
{
	destroy_threads_except(proc, NULL);
}

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

	err = lock_threads(proc, cur);
	if (!err)
		proc->state = PROC_LOCKED;

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

void process_destroy(struct process* proc)
{
	vmm_unref(proc->vmm);
	destroy_threads(proc);
	unregister_process(proc);

	kfree(proc);
}
