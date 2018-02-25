#include <kernel/errno.h>
#include <kernel/exec.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <kernel/locking/spinlock.h>
#include <kernel/process.h>
#include <libk/libk.h>

#define INVALID_PID	0
#define PID_MIN		PROCESS_INIT_PID

/**
 * @brief list of existing processes
 * This list is sorted by pid.
 */
static LIST_DEFINE(process_list);

/**
 * @brief process_list spinlock
 */
static spinlock_t process_list_lock = SPINLOCK_NULL;

/**
 * @brief "Kernel" process
 *
 * The process to which kernel threads are attached
 */
static struct process kprocess;

/**
 * @brief Finds a free pid for a new process
 *
 * @return the process after which the new process with pid
 * return_value->pid + 1 should be inserted.
 *
 * @note the process_list should be locked before calling ths function.
 */
static struct process* find_next_free_pid(void)
{
	list_node_t* it;
	struct process* previous = NULL;
	struct process *current;

	if (list_empty(&process_list))
		return NULL;

	// last process
	current = list_entry(list_back(&process_list), struct process, p_list);
	if (current->pid + 1 != INVALID_PID)
		return current;

	// first process
	it = list_front(&process_list);
	current = list_entry(it, struct process, p_list);
	it = list_it_next(it);
	while (it != list_end(&process_list)) {
		previous = current;
		current = list_entry(it, struct process, p_list);

		if (current->pid - previous->pid > 1)
			return previous;

		it = list_it_next(it);
	}

	return NULL;
}

/**
 * @brief Assigns a pid to the process and inserts it in the process_list
 *
 * @return 0 on sucess
 * -EAGAIN if no free pid was found
 */
static int register_process(struct process* proc)
{
	struct process* previous;
	int err = 0;

	spinlock_lock(&process_list_lock);

	if (list_empty(&process_list)) {
		proc->pid = PID_MIN;
		list_push_back(&process_list, &proc->p_list);
	}
	else {
		previous = find_next_free_pid();

		// no pid available
		if (!previous) {
			err = -EAGAIN;
		}
		else {
			proc->pid = previous->pid + 1;
			// insert after previous pid
			list_insert_after(&previous->p_list, &proc->p_list);
		}
	}

	spinlock_unlock(&process_list_lock);

	return err;

}

static void destroy_threads_except(struct process* proc,
								   const struct thread* save)
{
	list_node_t* it;
	list_node_t* next;

	list_foreach_safe(&proc->threads, it, next) {
		struct thread* thread = list_entry(it, struct thread, p_thr_list);
		if (thread != save)
			thread_unref(thread);
	}

	list_clear(&proc->threads);
}

static void destroy_threads(struct process* proc)
{
	destroy_threads_except(proc, NULL);
}

static int create(struct process* proc, const char* name)
{
	int err;

	memset(proc, 0, sizeof(struct process));

	err = vm_context_create(&proc->vm_ctx);
	if (err)
		return err;

	strlcpy(proc->name, name, PROCESS_NAME_MAX_LENGTH);

	proc->root = vfs_cache_node_get_root();

	proc->parent = NULL;
	list_init(&proc->children);

	list_init(&proc->threads);

	err = register_process(proc);
	if (err)
		process_destroy(proc);

	return err;
}

int process_init(struct process* proc, const char* name, start_func_t start,
				 void* start_args)
{
	int err;

	err = create(proc, name);
	if (!err) {
		err = process_create_uthread(proc, name, start, start_args);
		if (err)
			process_destroy(proc);
	}

	return err;
}

int process_create(const char* name, start_func_t start, void* start_args,
				   struct process** result)
{
	struct process* proc;
	int err;

	proc = kmalloc(sizeof(struct process));
	if (!proc)
		return -ENOMEM;

	err = process_init(proc, name, start, start_args);
	if (err) {
		kfree(proc);
		proc = NULL;
	}

	*result = proc;

	return err;
}

void process_prepare_exec(struct process* proc, const struct thread* save)
{
	/* vfs_cache_node_drop_ref(proc->exec); */
	proc->exec = NULL;

	vm_context_clear_userspace(&proc->vm_ctx);

	destroy_threads_except(proc, save);
}

int process_kprocess_init(void)
{
	int err = create(&kprocess, "kthreadd");
	if (!err)
		kassert(kprocess.pid == PROCESS_KTHREADD_PID);

	return err;
}

struct process* process_get_kprocess(void)
{
	return &kprocess;
}

static int add_thread(struct process* proc, struct thread* thread,
					  enum thread_type expected)
{
	if (thread->type != expected)
		return -EINVAL;

	if (!list_empty(&proc->threads)) {
		const struct thread* head =
			list_entry(list_front(&proc->threads), struct thread, p_thr_list);
		kassert(head->type == thread->type);
	}

	thread->process = proc;

	list_push_back(&proc->threads, &thread->p_thr_list);
	thread_ref(thread);

	return 0;
}

static int
process_create_thread(struct process* proc, enum thread_type type,
					  int (*__thread_create)(const char*, start_func_t, void*,
											 struct thread**),
					  const char* name, start_func_t start, void* start_args,
					  struct thread** result)
{
	struct thread* thread;
	int err;

	err = __thread_create(name, start, start_args, &thread);
	if (!err) {
		err = add_thread(proc, thread, type);
		thread_unref(thread);
	}

	if (result)
		*result = thread;

	return err;
}

int process_create_uthread(struct process* proc, const char* name,
						  start_func_t __user start, void* __user start_args)
{
	return process_create_thread(proc, UTHREAD, thread_uthread_create, name,
								 start, start_args, NULL);
}

int process_create_kthread(const char* name, start_func_t start,
						   void* start_args, struct thread** result)
{
	return process_create_thread(&kprocess, KTHREAD, thread_kthread_create,
								 name, start, start_args, result);
}

struct thread* process_get_initial_thread(const struct process* proc)
{
	return list_entry(list_front(&proc->threads), struct thread, p_thr_list);
}

void process_destroy(struct process* proc)
{
	vm_context_destroy(&proc->vm_ctx);
	destroy_threads(proc);
	list_erase(&process_list, &proc->p_list);
}
