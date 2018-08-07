#include <dummyos/errno.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <kernel/signal.h>
#include <kernel/types.h>
#include <kernel/mm/uaccess.h>
#include <kernel/sched/sched.h>
#include <libk/libk.h>

static void queued_siginfo_destroy(queued_siginfo_t* qsinfo);

int sys_kill(pid_t pid, int sig)
{
	return process_kill(pid, sig);
}

static inline bool sig_is_valid(int sig)
{
	return (sig > 0 && sig <= SIGNAL_MAX);
}

static inline bool sig_is_settable(int sig)
{
	return (sig_is_valid(sig) && sig != SIGKILL && sig != SIGSTOP);
}

int sigaction(int sig, const struct sigaction* restrict __kernel act,
			  struct sigaction* restrict __kernel oact)
{
	struct process* current_proc = sched_get_current_process();
	struct signal_manager* sigm = current_proc->signals;

	if (oact)
		*oact = sigm->actions[sig - 1];

	sigm->actions[sig - 1] = *act;

	return 0;
}

int sys_sigaction(int sig, const struct sigaction* restrict __user act,
				  struct sigaction* restrict __user oact)
{
	struct sigaction new;
	struct sigaction old;
	int err;

	if (!sig_is_settable(sig))
		return -EINVAL;

	err = copy_from_user(&new, act, sizeof(struct sigaction));
	if (err)
		return -EFAULT;

	err = sigaction(sig, &new, (oact) ? &old : NULL);
	if (!err && oact)
		err = copy_to_user(oact, &old, sizeof(struct sigaction));

	return err;
}

sighandler_t sys_signal(int sig, sighandler_t handler)
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

	cpu_ctx = cpu_context_get_previous(cpu_ctx);
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

static void siginfo_init(siginfo_t* sinfo, int sig,
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

static void queued_siginfo_init(queued_siginfo_t* qsinfo, int sig,
								void* addr, const struct process* sender)
{
	siginfo_init(&qsinfo->sinfo, sig, addr, sender);
	qsinfo->saved_mask = 0;
	list_node_init(&qsinfo->s_queue);
}

static int queued_siginfo_create(int sig, void* addr,
								 const struct process* sender,
								 queued_siginfo_t** result)
{
	queued_siginfo_t* qsinfo = kmalloc(sizeof(queued_siginfo_t));
	if (!qsinfo)
		return -ENOMEM;

	queued_siginfo_init(qsinfo, sig, addr, sender);

	*result = qsinfo;

	return 0;
}

static void queued_siginfo_reset(queued_siginfo_t* qsinfo)
{
	memset(qsinfo, 0, sizeof(queued_siginfo_t));
}

static void queued_siginfo_destroy(queued_siginfo_t* qsinfo)
{
	queued_siginfo_reset(qsinfo);
	kfree(qsinfo);
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

void signal_reset(struct signal_manager* sigm)
{
	list_node_t* it;
	list_node_t* next;

	list_foreach_safe(&sigm->sig_queue, it, next) {
		queued_siginfo_t* qsinfo = list_entry(it, queued_siginfo_t, s_queue);
		kfree(qsinfo);
	}
	list_foreach_safe(&sigm->handled_stack, it, next) {
		queued_siginfo_t* qsinfo = list_entry(it, queued_siginfo_t, s_queue);
		kfree(qsinfo);
	}

	memset(sigm, 0, sizeof(struct signal_manager));
}

void signal_destroy(struct signal_manager* sigm)
{
	signal_reset(sigm);
	kfree(sigm);
}

void signal_reset_dispositions(struct signal_manager* sigm)
{
	sigset_t mask = sigm->mask;

	signal_init(sigm);
	sigm->mask = mask;
}

bool signal_pending(const struct signal_manager* sigm)
{
	return !list_empty(&sigm->sig_queue);
}

static inline bool signal_is(int sig, const struct signal_manager* sigm,
							 sighandler_t hdlr)
{
	const struct sigaction* act = &sigm->actions[sig - 1];
	return sigaction_is(act, hdlr);
}

bool signal_is_dlf(int sig, const struct signal_manager* sigm)
{
	return signal_is(sig, sigm, SIG_DFL);
}

bool signal_is_ign(int sig, const struct signal_manager* sigm)
{
	return signal_is(sig, sigm, SIG_IGN);
}

int signal_send(struct signal_manager* sigm, int sig, void* addr,
				const struct process* sender)
{
	queued_siginfo_t* qsinfo;
	int err;

	err = queued_siginfo_create(sig, addr, sender, &qsinfo);
	if (!err)
		list_push_back(&sigm->sig_queue, &qsinfo->s_queue);

	return err;
}

static void signal_reset_handler(struct signal_manager* sigm, int sig)
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

static int copy_to_user_stack(struct cpu_context* cpu_ctx, const void* data,
							   size_t size, size_t alignment)
{
	v_addr_t usr_stack = cpu_context_get_user_sp(cpu_ctx);
	int err;

	usr_stack = align_down(usr_stack - size, alignment);

	err = copy_to_user((void*)usr_stack, data, size);
	if (!err)
		cpu_context_set_user_sp(cpu_ctx, usr_stack);

	return err;
}

static v_addr_t setup_signal_trampoline(struct cpu_context* cpu_ctx)
{
	extern const uint8_t __sig_tramp_start;
	extern const uint8_t __sig_tramp_end;

	const size_t sig_tramp_size = &__sig_tramp_end - &__sig_tramp_start;
	int err;

	err = copy_to_user_stack(cpu_ctx, &__sig_tramp_start, sig_tramp_size,
							 sizeof(void*));

	return (err) ? 0 : cpu_context_get_user_sp(cpu_ctx);
}

static int handle_dfl(int sig, const struct thread* thr)
{
	if (sig == SIGCHLD) {
		// SIGCHLD default action is to ignore the signal
		return 0;
	}
	// TODO: stop/cont

	process_exit(thr->process, -1);
	return -EAGAIN;
}

int handle(siginfo_t* sinfo, struct thread* thr)
{
	int sig = sinfo->si_signo;
	struct signal_manager* sigm = thr->process->signals;
	const struct sigaction* act = &sigm->actions[sig - 1];
	struct cpu_context* sh_ctx = cpu_context_get_next_user(thr->cpu_context);
	sigset_t saved_mask = sigm->mask;
	int err;

	if (signal_is_ign(sig, sigm))
		return 0;
	if (signal_is_dlf(sig, sigm))
		return handle_dfl(sig, thr);

	if (!(act->sa_flags & SA_NODEFER))
		sigm->mask |= 1 << (sig - 1);
	sigm->mask |= act->sa_mask;

	if (thread_sleep_was_intr(thr)) {
		if (act->sa_flags & SA_RESTART) {
			struct cpu_context* sc_restart =
				cpu_context_get_next_kernel(thr->cpu_context);
			sh_ctx = cpu_context_get_next_user(sc_restart);

			extern v_addr_t __syscall_restart;
			cpu_context_kernel_init(sc_restart, (v_addr_t)&__syscall_restart);
			cpu_context_copy_syscall_regs(sc_restart, thr->cpu_context);
		}
		else {
			cpu_context_set_syscall_return_value(thr->cpu_context, -EINTR);
		}
	}

	v_addr_t usr_sp = cpu_context_get_user_sp(thr->cpu_context);
	if (act->sa_flags & SA_ONSTACK)
		usr_sp = (v_addr_t)sigm->altstack.ss_sp;

	cpu_context_user_init(sh_ctx, (v_addr_t)NULL, usr_sp);

	v_addr_t sig_tramp = setup_signal_trampoline(sh_ctx);
	if (!sig_tramp)
		goto fail;

	v_addr_t handler;
	v_addr_t handler_args[3] = { sig, 0, 0 };
	size_t handler_args_n = 1;

	if (act->sa_flags & SA_SIGINFO) {
		err = copy_to_user_stack(sh_ctx, sinfo, sizeof(siginfo_t),
								 _Alignof(siginfo_t));
		if (err)
			goto fail;
		handler_args[1] = cpu_context_get_user_sp(sh_ctx);
		handler_args_n = 3;

		handler = (v_addr_t)act->sa_sigaction;
	}
	else {
		handler = (v_addr_t)act->sa_handler;
	}

	err = cpu_context_setup_signal_handler(sh_ctx, handler, sig_tramp,
										   handler_args, handler_args_n);
	if (err)
		goto fail;

	thr->cpu_context = sh_ctx;

	if (act->sa_flags & SA_RESETHAND)
		signal_reset_handler(sigm, sig);

	queued_siginfo_t* qsinfo = siginfo_get_queued_siginfo(sinfo);
	qsinfo->saved_mask = saved_mask;
	list_push_back(&sigm->handled_stack, &qsinfo->s_queue);

	return 0;

fail:
	signal_send(sigm, SIGKILL, NULL, 0);
	return -EFAULT;
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
