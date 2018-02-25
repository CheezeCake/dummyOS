#include <kernel/errno.h>
#include <kernel/exec.h>
#include <kernel/kassert.h>
#include <kernel/panic.h>
#include <kernel/process.h>
#include <kernel/sched/sched.h>

static struct process init;

static int init_exec(void* init)
{
	const char* init_path = init;
	int err;

	if (init_path) {
		err = exec(init_path, NULL, NULL);

		log_e_printf("INIT: %s (error %d)\n", init_path, err);
		PANIC("Failed to execute init command!");

		return err;
	}

	// fallback
	exec("/sbin/init", NULL, NULL);
	exec("/etc/init", NULL, NULL);
	exec("/bin/init", NULL, NULL);
	exec("/bin/sh", NULL, NULL);

	PANIC("Failed to find a working init!");

	return -ENOENT;
}

int init_process_init(char* init_path)
{
	int err;

	err = process_init(&init, "init", init_exec, init_path);
	if (!err) {
		kassert(init.pid == PROCESS_INIT_PID);
		err = sched_add_thread(process_get_initial_thread(&init));
	}

	return err;
}
