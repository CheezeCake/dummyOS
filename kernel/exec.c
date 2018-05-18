#include <dummyos/const.h>
#include <arch/vm.h>
#include <fs/file.h>
#include <fs/vfs.h>
#include <kernel/elf.h>
#include <kernel/errno.h>
#include <kernel/exec.h>
#include <kernel/kmalloc.h>
#include <kernel/sched/sched.h>
#include <kernel/mm/uaccess.h>
#include <kernel/mm/vmm.h>
#include <libk/libk.h>
#include <libk/utils.h>

#include <kernel/log.h>

static void free_arg_str_array(struct arg_str* array, size_t size)
{
	for (size_t i = 0; i < size; ++i)
		kfree(array[i].str);

	kfree(array);
}

static inline void free_user_args(struct user_args* uargs)
{
	free_arg_str_array(uargs->array, uargs->size);
}

/**
 * Returns the argument array size (including the last, "NULL" element)
 */
static ssize_t arg_array_size(char* const __user array[])
{
	char* str = NULL;
	ssize_t size = 0;

	do {
		if (copy_from_user(&str, &array[size], sizeof(char*)) != 0)
			return -EFAULT;

		++size;
	} while (str);

	return size;
}

static ssize_t copy_from_user_args(struct user_args* result,
								   char* const __user array[])
{
	struct arg_str* arg_str_array = NULL;
	ssize_t size = 0;

	if (array) {
		size = arg_array_size(array);
		if (size < 0)
			return -EFAULT;

		arg_str_array = kcalloc(size, sizeof(struct arg_str));
		if (!arg_str_array)
			return -ENOMEM;

		for (size_t i = 0; i < size - 1; ++i) {
			char* str_copy = strndup_from_user(array[i], 1024);
			if (!str_copy) {
				free_arg_str_array(arg_str_array, i);
				return -EFAULT;
			}

			arg_str_array[i].str = str_copy;
			arg_str_array[i].len = strlen(str_copy);;
		}

		memset(&arg_str_array[size - 1], 0, sizeof(struct arg_str));
	}

	result->array = arg_str_array;
	result->size = size;

	return size;
}

static inline size_t user_args_data_size(const struct user_args* args)
{
	size_t s = 0;
	for (size_t i = 0; i < args->size; ++i)
		s += args->array[i].len;

	return s;
}

static inline size_t user_args_array_size(const struct user_args* args)
{
	return (args->size * sizeof(char**));
}

static int copy_to_user_args(const struct user_args* args,
							 char* __user stack_top, char* __user stack_bottom,
							 char** args_base)
{
	char* __user str = stack_top;
	char* __user array = stack_top - user_args_data_size(args) -
		user_args_array_size(args);
	int err;

	array = (char*)align_down((v_addr_t)array, sizeof(char*));
	if (array < stack_bottom)
		return -ENOMEM;

	*args_base = array;

	for (size_t i = 0; i < args->size; ++i) {
		str -= args->array[i].len;
		if (str < stack_bottom)
			return -ENOMEM;

		err = copy_to_user(str, args->array[i].str, args->array[i].len);
		if (err)
			return err;

		const char* str_kptr = str;
		copy_to_user(&array[i], &str_kptr, sizeof(char*));
	}

	return 0;
}

/**
 * Copies the environment variables array (envp), the arguments array (argv) 
 * and the argument count (argc) to the user stack
 */
static int setup_user_args(char* __user stack_top, size_t stack_size,
						   const struct user_args* argv,
						   const struct user_args* envp)
{
	int argc = argv->size;
	char* __user top = stack_top;
	char* __user stack_bottom = stack_top - stack_size;
	int err;

	err = copy_to_user_args(envp, top, stack_bottom, &top);
	if (err)
		return err;

	err = copy_to_user_args(argv, top, stack_bottom, &top);
	if (err)
		return err;

	top = (char*)align_down((v_addr_t)top, sizeof(int));
	err = copy_to_user(top - sizeof(int), &argc, sizeof(int));

	return err;
}

static int create_user_stack(v_addr_t* stack_bottom, size_t stack_size)
{
	v_addr_t stack_top;
	int err;

	stack_top = USER_SPACE_END;
	*stack_bottom = stack_top - stack_size;

	err = vmm_create_user_mapping(*stack_bottom, stack_size,
								  VMM_PROT_USER | VMM_PROT_WRITE,
								  VMM_MAP_GROWSDOW);

	return err;
}

int exec(const char* path, const struct user_args* argv,
		 const struct user_args* envp)
{
	struct thread* thread = sched_get_current_thread();
	struct process* proc = thread->process;
	pid_t proc_pid = proc->pid;
	struct vmm* proc_vmm = proc->vmm;
	v_addr_t entry_point, stack_bottom;
	const size_t stack_size = PAGE_SIZE;
	int err;

	const char* new_proc_name =
		(argv && argv->size > 0) ? argv->array[0].str : path;
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
	proc->vmm = new_proc->vmm;
	vmm_switch_to(new_proc->vmm);

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
	err = create_user_stack(&stack_bottom, stack_size);
	if (err)
		goto fail_stack;

	struct thread* new_proc_main_thr;
	err = thread_create(entry_point, stack_bottom + stack_size - 1,
						&new_proc_main_thr);
	if (err) {
		// XXX:
	}
	err = process_add_thread(new_proc, new_proc_main_thr);
	err = sched_add_thread(new_proc_main_thr);
	thread_unref(new_proc_main_thr);


	proc->vmm = proc_vmm;

	process_exit_quiet(proc);
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

// TODO: add sys_execve to the syscall table
int sys_execve(const char* __user path, char* const __user argv[],
			   char* const __user envp[])
{
	char* kpath;
	struct user_args kargv;
	struct user_args kenvp;
	ssize_t err;

	kpath = strndup_from_user(path, VFS_PATH_MAX_LEN);
	if (!kpath)
		return -EFAULT;

	err = copy_from_user_args(&kargv, argv);
	if (err)
		goto fail_argv;

	err = copy_from_user_args(&kenvp, envp);
	if (err)
		goto fail_envp;

	return exec(kpath, &kargv, &kenvp);

fail_envp:
	free_user_args(&kenvp);
fail_argv:
	free_user_args(&kargv);

	kfree(kpath);

	return err;
}
