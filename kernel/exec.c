#include <arch/vm.h>
#include <fs/file.h>
#include <fs/vfs.h>
#include <kernel/elf.h>
#include <kernel/errno.h>
#include <kernel/exec.h>
#include <kernel/sched/sched.h>
#include <kernel/mm/vmm.h>
#include <libk/libk.h>

static int kill_process_threads(struct process* proc)
{
	int err;
	list_node_t* it;

	list_foreach(&proc->threads, it) {
		struct thread* thread = list_entry(it, struct thread, p_thr_list);

		thread_set_state(thread, THREAD_DEAD);
		err = sched_remove_thread(thread);
		if (err)
			return err;
	}

	return 0;
}

static int create_user_stack(v_addr_t* stack_top)
{
	v_addr_t stack_bottom;
	const size_t stack_size = PAGE_SIZE;
	int err;

	*stack_top = USER_SPACE_END;
	stack_bottom = *stack_top - stack_size;

	err = vmm_create_user_mapping(stack_bottom, stack_size,
								  VMM_PROT_USER | VMM_PROT_WRITE,
								  VMM_MAP_GROWSDOW);

	return err;
}

#include <kernel/log.h>
int exec(const char* __kernel path, char* const __kernel argv[],
		 char* const __kernel envp[])
{
	struct thread* thread = sched_get_current_thread();
	struct process* proc = thread->process;
	pid_t proc_pid = proc->pid;
	struct vmm* proc_vmm = proc->vmm;
	v_addr_t entry_point, stack_top;
	int err;

	const char* new_proc_name = (argv && argv[0]) ? argv[0] : path;
	struct process* new_proc;
	err = process_create(new_proc_name, &new_proc);
	if (err)
		return err;
	new_proc->pid = proc->pid;

	vfs_path_t exec_path;
	err = vfs_path_init(&exec_path, path, strlen(path));
	if (err)
		return err;

	log_puts("exec vmm_switch_to new\n");
	process_lock(proc, thread);
	vmm_switch_to(new_proc->vmm);
	proc->vmm = new_proc->vmm;

	struct vfs_cache_node* exec;
	err = vfs_lookup(&exec_path, proc->root, proc->cwd, &exec);
	if (err)
		goto fail_lookup;

	struct vfs_file file;
	vfs_file_init(&file, exec, O_RDONLY);
	err = exec->inode->op->open(exec->inode, exec, O_RDONLY, &file);
	if (err)
		goto fail_file;

	err = elf_load_binary(&file, &entry_point);
	if (err)
		goto fail_load_bin;

	// XXX:
	err = create_user_stack(&stack_top);
	if (err)
		goto fail_stack;

	struct thread* new_proc_main_thr;
	err = thread_create(entry_point, stack_top, &new_proc_main_thr);
	if (err) {
		// XXX:
	}
	err = process_add_thread(new_proc, new_proc_main_thr);
	err = sched_add_thread(new_proc_main_thr);
	thread_unref(new_proc_main_thr);


	log_puts("exec vmm_switch_to old\n");
	proc->vmm = proc_vmm;
	vmm_switch_to(proc->vmm);

	kill_process_threads(proc);
	process_destroy(proc);

	// XXX: ...
	err = process_register_pid(new_proc, proc_pid);
	if (err) {
	}

	sched_exit();

fail_stack:
fail_load_bin:
	exec->inode->op->close(exec->inode, &file);
fail_file:
	vfs_cache_node_unref(exec);
fail_lookup:
	vfs_path_reset(&exec_path);

	proc->vmm = proc_vmm;
	vmm_switch_to(proc_vmm);
	process_unlock(proc);

	return err;
}

int sys_execve(const char* __user path, char* const __user argv[],
			   char* const __user envp[])
{
	// copy_from_user args
	return exec(NULL, NULL, NULL);
}
