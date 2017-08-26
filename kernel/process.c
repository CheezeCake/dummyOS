#include <kernel/process.h>
#include <kernel/kmalloc.h>
#include <kernel/libk.h>
#include <kernel/kassert.h>

struct
{
	LIST_CREATE(struct process);
} process_list;

pid_t last_pid = 0;

void process_init(void)
{
	list_init_null(&process_list);
}

static int process_create_generic(struct process* proc, const char* name)
{
	memset(proc, 0, sizeof(struct process));

	if (vm_context_create(&proc->vm_ctx) != 0)
		return -1;

	proc->pid = ++last_pid;
	strlcpy(proc->name, name, strlen(name));
	proc->parent = NULL;

	list_init_null(&proc->threads);

	struct thread* initial_thread = NULL;
	if (!(initial_thread = kmalloc(sizeof(struct thread)))) {
		process_destroy(proc);
		return -1;
	}

	struct thread_list_node* node = thread_list_node_create(initial_thread);
	if (!node) {
		process_destroy(proc);
		kfree(initial_thread);
		return -1;
	}

	list_push_back(&proc->threads, node);

	return 0;
}

int process_create(struct process* proc, const char* name)
{
	if (process_create_generic(proc, name) != 0)
		return -1;

	struct thread* thread = list_front(&proc->threads)->thread;
	start_func_t start = NULL;
	void* start_args = NULL;
	exit_func_t exit = NULL;

	if (thread_uthread_create(thread, name, proc, 2048, 2048, start,
				start_args, exit) != 0) {
		process_destroy(proc);
		return -1;
	}

	list_push_back(&process_list, proc);

	return 0;
}

int process_kprocess_create(struct process* proc, const char* name,
		start_func_t start)
{
	if (process_create_generic(proc, name) != 0)
		return -1;

	struct thread* thread = list_front(&proc->threads)->thread;
	void* start_args = NULL;
	exit_func_t exit = NULL;

	if (thread_kthread_create(thread, name, proc, 2048, start,
				start_args, exit) != 0) {
		process_destroy(proc);
		return -1;
	}

	list_push_back(&process_list, proc);

	return 0;
}

int process_add_thread(struct process* proc, struct thread* thread)
{
	kassert(list_front(&proc->threads)->thread->type == thread->type);

	thread->process = proc;
	struct thread_list_node* node = thread_list_node_create(thread);
	if (!node)
		return -1;

	list_push_back(&proc->threads, node);

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
