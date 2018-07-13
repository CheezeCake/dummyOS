#include <dummyos/errno.h>
#include <kernel/exec.h>
#include <kernel/kassert.h>
#include <kernel/panic.h>
#include <kernel/process.h>
#include <kernel/sched/sched.h>

#define INIT_PID 1

static const char* __init_path;

static int init_exec(void)
{
	const char* init_path = __init_path;
	const user_args_t argv = USER_ARGS_EMPTY;
	const user_args_t envp = USER_ARGS_EMPTY;
	int err;

	if (init_path) {
		err = exec(init_path, &argv, &envp);

		log_e_printf("INIT: %s (error %d)\n", init_path, err);
		PANIC("Failed to execute init command!");

		return err;
	}

	// fallback
	exec("/sbin/init", &argv, &envp);
	exec("/etc/init", &argv, &envp);
	exec("/bin/init", &argv, &envp);
	exec("/bin/sh", &argv, &envp);

	PANIC("Failed to find a working init!");

	return -ENOENT;
}

int init_process_init(const char* init_path)
{
	int err;
	struct process* init;
	struct thread* thr;

	__init_path = init_path;

	err = process_create("init", &init);
	if (err)
		return err;
	process_register_pid(init, INIT_PID);
	kassert(init->pid == INIT_PID);

	init->root = init->cwd = vfs_cache_node_get_root();

	// create user thread
	err = thread_create((v_addr_t)init_exec, (v_addr_t)NULL, &thr);
	// but assign kernel cpu context to execute init_exec()
	cpu_context_kernel_init(thr->cpu_context, (v_addr_t)init_exec);
	if (!err) {
		err = process_add_thread(init, thr);
		if (!err)
			err = sched_add_thread(thr);
	}
	thread_unref(thr);

	return err;
}
