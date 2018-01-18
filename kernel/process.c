#include <kernel/errno.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <kernel/process.h>
#include <libk/libk.h>

/**
 * @brief "Kernel" process
 *
 * The process to which kernel threads are attached
 */
static struct process kprocess;

static pid_t last_pid = 0;


static int create(struct process* proc, const char* name)
{
	int err;

	memset(proc, 0, sizeof(struct process));

	if ((err = vm_context_create(&proc->vm_ctx)) < 0)
		return err;

	proc->pid = ++last_pid;
	strlcpy(proc->name, name, PROCESS_NAME_MAX_LENGTH);

	proc->parent = NULL;
	list_init_null(&proc->children);

	list_init_null(&proc->threads);

	return 0;
}

int process_init(struct process* proc, const char* name)
{
	int err;
	if ((err = create(proc, name)) != 0)
		return err;

	start_func_t start = NULL;
	void* start_args = NULL;

	struct thread* initial_thread;
	err = thread_uthread_create(name, 2048, start, start_args, &initial_thread);
	if (err != 0) {
		process_destroy(proc);
		return err;
	}

	err = process_add_thread(proc, initial_thread);
	if (err != 0)
		process_destroy(proc);

	thread_unref(initial_thread);

	return err;
}

struct process* process_create(const char* name)
{
	struct process* proc = kmalloc(sizeof(struct process));
	if (!proc)
		return NULL;

	if (process_init(proc, name) < 0) {
		kfree(proc);
		return NULL;
	}

	return proc;
}

int process_kprocess_init(void)
{
	return create(&kprocess, "kthreadd");
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

int process_add_thread(struct process* proc, struct thread* thread)
{
	return add_thread(proc, thread, UTHREAD);
}

int process_add_kthread(struct thread* kthread)
{
	return add_thread(&kprocess, kthread, KTHREAD);
}

static void destroy_threads(struct process* proc)
{
	struct list_node* it = list_begin(&proc->threads);

	while (it) {
		struct list_node* tmp = it;
		it = list_it_next(it);

		struct thread* thread = list_entry(tmp, struct thread, p_thr_list);
		thread_unref(thread);
		kfree(thread);
	}

	list_clear(&proc->threads);
}

void process_destroy(struct process* proc)
{
	vm_context_destroy(&proc->vm_ctx);
	destroy_threads(proc);
}
