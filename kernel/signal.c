#include <kernel/errno.h>
#include <kernel/interrupt.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <kernel/signal.h>
#include <kernel/types.h>
#include <kernel/sched/sched.h>
#include <libk/libk.h>

static void queued_siginfo_destroy(queued_siginfo_t* qsinfo);

int sys_kill(pid_t pid, uint32_t sig)
{
	return process_kill(pid, sig);
}

static inline bool sig_is_valid(uint32_t sig)
{
	return (sig > 0 && sig <= SIGNAL_MAX);
}

static inline bool sig_is_settable(uint32_t sig)
{
	return (sig_is_valid(sig) && sig != SIGKILL && sig != SIGSTOP);
}

int sigaction(uint32_t sig, const struct sigaction* restrict __kernel act,
			  struct sigaction* restrict __kernel oact)
{
	struct process* current_proc = sched_get_current_process();
	struct signal_manager* sigm = current_proc->signals;

	if (oact)
		*oact = sigm->actions[sig - 1];

	sigm->actions[sig - 1] = *act;

	return 0;
}

int sys_sigaction(uint32_t sig, const struct sigaction* restrict __user act,
				  struct sigaction* restrict __user oact)
{
	if (!sig_is_settable(sig))
		return -EINVAL;

	// TODO: copy_from_user
	return sigaction(sig, act, oact);
}

sighandler_t sys_signal(uint32_t sig, sighandler_t handler)
{
	const struct sigaction act = {
		.sa_handler = handler,
		.sa_mask = 1 << (sig - 1),
		.sa_flags = 0,
	};

	log_e_printf("SIGNAL: pid=%d\n", sched_get_current_process()->pid);

	if (!sig_is_settable(sig))
		return SIG_ERR;

	return (sigaction(sig, &act, NULL) == 0) ? 0 : SIG_ERR;
}

void sys_sigreturn(void)
{
	struct thread* current_thread = sched_get_current_thread();
	struct cpu_context* cpu_ctx = current_thread->cpu_context;
	struct signal_manager* sigm = current_thread->process->signals;

	cpu_ctx = (struct cpu_context*)((int8_t*)cpu_ctx + cpu_context_sizeof());
	if ((v_addr_t)cpu_ctx > thread_get_kstack_top(current_thread)) {
		log_e_print("sigreturn while not returning from signal\n");
		return;
	}
	if (list_empty(&sigm->handled_stack)) {
		log_e_print("sigreturn with handled_stack empty\n");
		return;
	}

	queued_siginfo_t* qsinfo = list_entry(list_back(&sigm->handled_stack),
										  queued_siginfo_t, s_queue);
	list_pop_back(&sigm->handled_stack);
	sigm->mask = qsinfo->saved_mask;
	queued_siginfo_destroy(qsinfo);

	cpu_context_switch(cpu_ctx);
}

static inline bool sigaction_is(const struct sigaction* act, sighandler_t hdlr)
{
	return (act->sa_flags & SA_SIGINFO)
		? (act->sa_sigaction == (sigrestore_t)hdlr)
		: (act->sa_handler == hdlr);
}

static void siginfo_init(siginfo_t* sinfo, uint32_t sig,
						 void* addr, const struct process* sender)
{
	memset(sinfo, 0, sizeof(siginfo_t));

	sinfo->si_signo = sig;
	sinfo->si_code = sig;

	if (sender) {
		sinfo->si_pid = sender->pid;
		/* sinfo->si_uid = sender->uid; */
	}
}

static void queued_siginfo_init(queued_siginfo_t* qsinfo, uint32_t sig,
								void* addr, const struct process* sender)
{
	siginfo_init(&qsinfo->sinfo, sig, addr, sender);
	qsinfo->saved_mask = 0;
	list_node_init(&qsinfo->s_queue);
}

int signal_init(struct signal_manager* sigm)
{
	memset(sigm, 0, sizeof(struct signal_manager));
	list_init(&sigm->sig_queue);
	list_init(&sigm->handled_stack);

	return 0;
}

int signal_create(struct signal_manager** result)
{
	struct signal_manager* sigm;
	int err;

	sigm = kmalloc(sizeof(struct signal_manager));
	if (!sigm)
		return -ENOMEM;

	err = signal_init(sigm);
	if (err) {
		kfree(sigm);
		sigm = NULL;
	}

	*result = sigm;

	return err;
}

void signal_copy(struct signal_manager* dst, const struct signal_manager* src)
{
	memcpy(dst, src, sizeof(struct signal_manager));
	list_init(&dst->sig_queue);
	list_init(&dst->handled_stack);
}

void signal_destroy(struct signal_manager* sigm)
{
	kfree(sigm);
}

bool signal_pending(const struct signal_manager* sigm)
{
	return !list_empty(&sigm->sig_queue);
}

int signal_send(struct signal_manager* sigm, uint32_t sig, void* addr,
				const struct process* sender)
{
	queued_siginfo_t* qsinfo = kmalloc(sizeof(queued_siginfo_t));
	if (!qsinfo)
		return -ENOMEM;

	queued_siginfo_init(qsinfo, sig, addr, sender);
	list_push_back(&sigm->sig_queue, &qsinfo->s_queue);

	return 0;
}

static void signal_reset_handler(struct signal_manager* sigm, uint32_t sig)
{
	struct sigaction* act = &sigm->actions[sig - 1];

	memset(act, 0, sizeof(struct sigaction));
	act->sa_handler = SIG_DFL;
}

siginfo_t* signal_pop(struct signal_manager* sigm)
{
	if (!signal_pending(sigm))
		return NULL;

	queued_siginfo_t* ret = list_entry(list_front(&sigm->sig_queue),
									   queued_siginfo_t,
									   s_queue);
	list_pop_front(&sigm->sig_queue);

	return &ret->sinfo;
}

static inline queued_siginfo_t* siginfo_get_queued_siginfo(siginfo_t* sinfo)
{
	return container_of(sinfo, queued_siginfo_t, sinfo);
}

static void queued_siginfo_destroy(queued_siginfo_t* qsinfo)
{
	kfree(qsinfo);
}

extern const uint8_t __sig_tramp_start;
extern const uint8_t __sig_tramp_end;

static void copy_to_user_stack(struct cpu_context* cpu_ctx, const void* data,
							   size_t size, size_t alignment)
{
	v_addr_t usr_stack = cpu_context_get_user_sp(cpu_ctx);
	usr_stack -= size;
	usr_stack = align_down(usr_stack, alignment);

	// TODO: uacces copy_to_user
	memcpy((void*)usr_stack, data, size);

	cpu_context_set_user_sp(cpu_ctx, usr_stack);
}

static v_addr_t setup_signal_trampoline(struct cpu_context* cpu_ctx)
{
	const size_t sig_tramp_size = &__sig_tramp_end - &__sig_tramp_start;
	copy_to_user_stack(cpu_ctx, &__sig_tramp_start, sig_tramp_size,
					   sizeof(void*));

	return cpu_context_get_user_sp(cpu_ctx);
}

static inline bool signal_is(uint32_t sig, const struct signal_manager* sigm,
							 sighandler_t hdlr)
{
	const struct sigaction* act = &sigm->actions[sig - 1];
	return (sig_is_settable(sig) && sigaction_is(act, hdlr));
}

static inline bool signal_is_dlf(uint32_t sig, const struct signal_manager* sigm)
{
	return signal_is(sig, sigm, SIG_DFL);
}

static inline bool signal_is_ign(uint32_t sig, const struct signal_manager* sigm)
{
	return signal_is(sig, sigm, SIG_IGN);
}

static int handle_dfl(const struct thread* thr)
{
	process_exit(thr->process, -1);
	return -EAGAIN;
}

int handle(siginfo_t* sinfo, struct thread* thr)
{
	uint32_t sig = sinfo->si_signo;
	struct signal_manager* sigm = thr->process->signals;
	const struct sigaction* act = &sigm->actions[sig - 1];
	struct cpu_context* sh_ctx =
		(struct cpu_context*)((int8_t*)thr->cpu_context - cpu_context_sizeof());
	sigset_t saved_mask = sigm->mask;

	if (signal_is_ign(sig, sigm))
		return 0;
	if (signal_is_dlf(sig, sigm))
		return handle_dfl(thr);

	__irq_disable();

	kassert(cpu_context_is_usermode(thr->cpu_context));
	// TODO: remove
	vmm_switch_to(thr->process->vmm);

	memcpy(sh_ctx, thr->cpu_context, cpu_context_sizeof());

	if (!(act->sa_flags & SA_NODEFER))
		sigm->mask |= 1 << (sig - 1);
	sigm->mask |= act->sa_mask;

	if (act->sa_flags & SA_ONSTACK)
		cpu_context_set_user_sp(sh_ctx, (v_addr_t)sigm->altstack.ss_sp);

	if (act->sa_flags & SA_RESETHAND)
		signal_reset_handler(sigm, sig);

	v_addr_t sig_tramp = setup_signal_trampoline(sh_ctx);

	v_addr_t handler;
	v_addr_t handler_args[3] = { sig, 0, 0 };
	size_t handler_args_n = 1;

	if (act->sa_flags & SA_SIGINFO) {
		copy_to_user_stack(sh_ctx, sinfo, sizeof(siginfo_t),
						   _Alignof(siginfo_t));
		handler_args[1] = cpu_context_get_user_sp(sh_ctx);
		handler_args_n = 3;

		handler = (v_addr_t)act->sa_sigaction;
	}
	else {
		handler = (v_addr_t)act->sa_handler;
	}

	cpu_context_setup_signal_handler(sh_ctx, handler, sig_tramp, handler_args,
									 handler_args_n);
	thr->cpu_context = sh_ctx;

	queued_siginfo_t* qsinfo = siginfo_get_queued_siginfo(sinfo);
	qsinfo->saved_mask = saved_mask;
	list_push_back(&sigm->handled_stack, &qsinfo->s_queue);

	return 0;
}

int signal_handle(struct thread* thr)
{
	if (thr->type == KTHREAD)
		return -EINVAL;

	log_e_printf("%s: pid=%d\n", __func__, thr->process->pid);
	siginfo_t* sinfo = signal_pop(thr->process->signals);
	if (!sinfo)
		return -EINVAL;

	int err = handle(sinfo, thr);

	return err;
}
