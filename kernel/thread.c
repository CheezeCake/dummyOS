#include <dummyos/errno.h>
#include <kernel/cpu_context.h>
#include <kernel/interrupt.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <kernel/sched/sched.h>
#include <kernel/thread.h>
#include <libk/libk.h>

#include <kernel/log.h>

static int create_kstack(struct stack* kstack, size_t size)
{
	void* sp = kmalloc(size);
	if (!sp)
		return -ENOMEM;

	kstack->sp = (v_addr_t)sp;
	kstack->size = size;

	return 0;
}

static inline void free_kstack(struct stack* kstack)
{
	kfree((void*)kstack->sp);
}

static void thread_reset(struct thread* thread)
{
	if (thread->type == KTHREAD)
		kfree(thread->name);
	free_kstack(&thread->kstack);

	/* memset(thread, 0, sizeof(struct thread)); */
}

static void thread_destroy(struct thread* thread)
{
	log_printf("#### %s(): %s (%p) state=%d\n", __func__, thread->name,
			   (void*)thread, thread->state);
	thread_reset(thread);
	kfree(thread);
}

static int init(struct thread* thread, char* name, size_t kstack_size,
				enum thread_type type, thread_priority_t priority)
{
	int err;

	memset(thread, 0, sizeof(struct thread));

	thread->name = name;

	err = create_kstack(&thread->kstack, kstack_size);
	if (!err) {
		const v_addr_t top = stack_get_top(&thread->kstack);
		thread->cpu_context = cpu_context_get_next_user((struct cpu_context*)top);

		thread->syscall_ctx = NULL;
		thread->state = THREAD_READY;
		thread->type = type;
		thread->priority = priority;

		refcount_init(&thread->refcnt);
	}

	return err;
}

static int __thread_create(char* name, size_t kstack_size,
						   enum thread_type type, struct thread** result)
{
	int err;

	struct thread* thread = kmalloc(sizeof(struct thread));
	if (!thread)
		return -ENOMEM;

	err = init(thread, name, kstack_size, type, SCHED_PRIORITY_LEVEL_DEFAULT);
	if (err) {
		kfree(thread);
		thread = NULL;
	}

	*result = thread;

	return err;
}

int __kthread_create(char* name, v_addr_t start, struct thread** result)
{
	int err;

	err = __thread_create(name, DEFAULT_KSTACK_SIZE, KTHREAD, result);
	if (!err)
		cpu_context_kernel_init((*result)->cpu_context, start);

	return err;
}

int thread_create(v_addr_t start, v_addr_t user_stack, struct thread** result)
{
	int err;

	err = __thread_create(NULL, DEFAULT_KSTACK_SIZE, UTHREAD, result);
	if (!err)
		cpu_context_user_init((*result)->cpu_context, start, user_stack);

	return err;
}

static void clone_kstack(const struct thread* thread, struct thread* new)
{
	kassert((v_addr_t)thread->cpu_context >= thread->kstack.sp);

	ptrdiff_t diff = (v_addr_t)thread->cpu_context - thread->kstack.sp;
	new->cpu_context = (struct cpu_context*)(new->kstack.sp + diff);

	// copies cpu_context
	memcpy((void*)new->kstack.sp, (void*)thread->kstack.sp,
		   thread->kstack.size);
}

static int clone(const struct thread* thread, char* name, struct thread* new)
{
	int err;

	err = init(new, name, thread->kstack.size, thread->type, thread->priority);
	if (!err)
		clone_kstack(thread, new);

	return err;
}

int thread_clone(const struct thread* thread, char* name,
				 struct thread** result)
{
	int err;

	struct thread* new = kmalloc(sizeof(struct thread));
	if (!new)
		return -ENOMEM;

	err = clone(thread, name, new);
	if (err) {
		kfree(new);
		new = NULL;
	}

	*result = new;

	return err;
}

void thread_ref(struct thread* thread)
{
	refcount_inc(&thread->refcnt);
}

void thread_unref(struct thread* thread)
{
	if (refcount_dec(&thread->refcnt) == 0)
		thread_destroy(thread);
}

int thread_get_ref(const struct thread* thread)
{
	return refcount_get(&thread->refcnt);
}

void thread_set_state(struct thread* thread, enum thread_state state)
{
	irq_disable();

	thread->state = state;

	irq_enable();
}

enum thread_state thread_get_state(const struct thread* thread)
{
	return thread->state;
}

v_addr_t thread_get_kstack_top(const struct thread* thread)
{
	return thread->kstack.sp + thread->kstack.size;
}

void thread_switch_setup(struct cpu_context* cpu_ctx)
{
	if (cpu_context_is_usermode(cpu_ctx))
		vmm_switch_to(sched_get_current_thread()->process->vmm);
}

int thread_intr_sleep(struct thread* thr)
{
	if (thr->type == KTHREAD ||
		thread_get_state(thr) != THREAD_SLEEPING)
		return -EINVAL;

	kassert(!cpu_context_is_usermode(thr->cpu_context));
	kassert((v_addr_t)thr->syscall_ctx > thr->kstack.sp &&
			(v_addr_t)thr->syscall_ctx <= thread_get_kstack_top(thr));
	kassert(cpu_context_is_usermode(thr->syscall_ctx));
	kassert(list_node_chained(&thr->wqe));

	list_erase(&thr->wqe);
	sched_add_thread(thr);

	thr->cpu_context = thr->syscall_ctx;

	return 0;
}

bool thread_sleep_was_intr(const struct thread* thr)
{
	return (thr->cpu_context == thr->syscall_ctx);
}

void thread_set_cpu_context(struct thread* thr, struct cpu_context* ctx)
{
	thr->cpu_context = ctx;
}

void thread_set_syscall_context(struct thread* thr, struct cpu_context* ctx)
{
	thr->syscall_ctx = ctx;
}
