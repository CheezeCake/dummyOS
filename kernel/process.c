#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <kernel/process.h>
#include <libk/libk.h>

struct
{
	LIST_CREATE(struct process);
} process_list;

pid_t last_pid = 0;

void process_init(void)
{
	list_init_null(&process_list);
}

static int process_create(struct process* proc, const char* name)
{
	memset(proc, 0, sizeof(struct process));

	if (vm_context_create(&proc->vm_ctx) != 0)
		return -1;

	proc->pid = ++last_pid;
	strlcpy(proc->name, name, strlen(name));
	proc->parent = NULL;

	list_init_null(&proc->threads);

	return 0;
}

static inline int process_create_initial_thread(struct process* proc, struct thread* thread)
{
	if (!thread || process_add_thread(proc, thread) != 0) {
		if (thread)
			thread_destroy(thread);
		process_destroy(proc);
		return -1;
	}

	thread_unref(thread);

	list_push_back(&process_list, proc);

	return 0;
}

int process_uprocess_create(struct process* proc, const char* name)
{
	if (process_create(proc, name) != 0)
		return -1;

	start_func_t start = NULL;
	void* start_args = NULL;
	exit_func_t exit = NULL;

	return process_create_initial_thread(proc,
			thread_uthread_create(name, proc, 2048, 2048, start, start_args, exit));
}

int process_kprocess_create(struct process* proc, const char* name,
		start_func_t start)
{
	if (process_create(proc, name) != 0)
		return -1;

	void* start_args = NULL;
	exit_func_t exit = NULL;

	return process_create_initial_thread(proc,
			thread_kthread_create(name, proc, 2048, start, start_args, exit));
}

int process_add_thread(struct process* proc, struct thread* thread)
{
	if (!list_empty(&proc->threads))
			kassert(list_front(&proc->threads)->thread->type == thread->type);

	thread->process = proc;
	struct thread_list_node* node = thread_list_node_create(thread);
	if (!node)
		return -1;

	list_push_back(&proc->threads, node);
	thread_ref(thread);

	return 0;
}

void process_destroy(struct process* proc)
{
	struct thread_list_node* it = list_begin(&proc->threads);

	while (it) {
		struct thread_list_node* tmp = it;
		it = list_it_next(it);

		thread_destroy(tmp->thread);
		kfree(tmp->thread);
		kfree(tmp);
	}
	list_clear(&proc->threads);

	list_erase(&process_list, proc);
}
