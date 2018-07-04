#include <arch/vm.h>
#include <dummyos/const.h>
#include <dummyos/fcntl.h>
#include <fs/file.h>
#include <fs/vfs.h>
#include <kernel/elf.h>
#include <kernel/errno.h>
#include <kernel/exec.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <kernel/mm/uaccess.h>
#include <kernel/mm/vmm.h>
#include <kernel/sched/sched.h>
#include <libk/libk.h>
#include <libk/utils.h>

#include <kernel/log.h>

#define USER_STACK_TOP	USER_SPACE_END;
#define USER_STACK_SIZE PAGE_SIZE

int user_args_init(user_args_t* user_args, size_t size)
{
	user_arg_t* args = NULL;

	if (size > 0) {
		args = kcalloc(size, sizeof(user_arg_t));
		if (!args)
			return -ENOMEM;
	}

	user_args->args = args;
	user_args->size = size;

	return 0;
}

void user_args_reset(user_args_t* user_args)
{
	if (user_args->args)
		kfree(user_args->args);

	memset(user_args, 0, sizeof(user_args_t));
}

int user_args_set_arg(user_args_t* user_args, size_t i, char* str)
{
	if (i >= user_args->size)
		return -EINVAL;

	user_args->args[i].str = str;
	user_args->args[i].len = strlen(str);

	return 0;
}

static inline const char* user_args_get_arg(const user_args_t* user_args,
											size_t i)
{
	return (i < user_args->size) ? user_args->args[i].str : NULL;
}

static ssize_t user_args_size_from_user(char* const __user args[])
{
	ssize_t size = 0;
	char* arg;
	bool done = false;
	int err;

	if (args) {
		do {
			err = copy_from_user(&arg, &args[size], sizeof(char*));
			if (!err && arg)
				++size;
			else
				done = true;
		} while (!done);
	}

	return size;
}

static size_t user_args_data_size(const user_args_t* user_args)
{
	size_t size = 0;

	for (size_t i = 0; i < user_args->size; ++i)
		size += user_args->args[i].len + 1;

	return size;
}

static inline size_t user_args_array_size(const user_args_t* user_args)
{
	return ((user_args->size + 1) * sizeof(char*));
}

static int user_args_copy_from_user(char* const __user args[],
									user_args_t* user_args)
{
	ssize_t size = 0;
	char* str_copy;
	int err;

	if (args) {
		size = user_args_size_from_user(args);
		if (size < 0)
			return -EFAULT;
	}

	err = user_args_init(user_args, size);
	if (err)
		return err;

	for (size_t i = 0; i < size; ++i) {
		err = strndup_from_user(args[i], 1024, &str_copy);
		if (err) {
			user_args_reset(user_args);
			return err;
		}

		user_args_set_arg(user_args, i, str_copy);
	}

	return 0;
}

static int user_args_copy_to_user(const user_args_t* user_args,
								  v_addr_t __user stack_top, size_t stack_size,
								  v_addr_t* args)
{
	v_addr_t data = stack_top - user_args_data_size(user_args);
	v_addr_t array = data - user_args_array_size(user_args);
	int err;

	array = align_down(array, _Alignof(char**));

	if (array + stack_size < stack_top)
		return -ENOMEM;

	for (size_t i = 0; i < user_args->size; ++i) {
		size_t len = user_args->args[i].len + 1; // strlen + '\0'

		err = copy_to_user((void*)data, user_args->args[i].str, len);
		if (err)
			return err;
		err = copy_to_user(&((char**)array)[i], &data, sizeof(char*));
		if (err)
			return err;

		data += len;
	}

	// last element (NULL)
	err = copy_to_user(&((char**)array)[user_args->size], &(char*){NULL},
					   sizeof(char*));

	*args = array;

	return 0;
}

static int main_args_copy_to_user(v_addr_t argc, v_addr_t __user argv,
								  v_addr_t __user envp, v_addr_t stack_top,
								  size_t stack_size, v_addr_t* main_args)
{
	v_addr_t* __user args = (v_addr_t*)argv - 3;
	int err;

	// copy bellow argv
	if ((v_addr_t)args + stack_size < stack_top)
		return -ENOMEM;

	err = copy_to_user(&args[2], &envp, sizeof(v_addr_t));
	if (err)
		return err;

	err = copy_to_user(&args[1], &argv, sizeof(v_addr_t));
	if (err)
		return err;

	err = copy_to_user(&args[0], &argc, sizeof(v_addr_t));
	if (!err)
		*main_args = (v_addr_t)args;

	return err;
}

static int setup_user_args(v_addr_t stack_bottom, size_t stack_size,
						   const user_args_t* argv, const user_args_t* envp,
						   v_addr_t* args)
{
	const v_addr_t stack_top = stack_bottom + stack_size - 1;
	v_addr_t argc_val = argv->size;
	v_addr_t argv_val;
	v_addr_t envp_val;
	int err;

	err = user_args_copy_to_user(envp, stack_top, stack_size, &envp_val);
	if (err)
		return err;

	err = user_args_copy_to_user(argv, envp_val, stack_size, &argv_val);
	if (err)
		return err;

	err = main_args_copy_to_user(argc_val, argv_val, envp_val, stack_top,
								 stack_size, args);
	if (err)
		return err;

	return 0;
}

static int create_user_stack(v_addr_t* stack_bottom, size_t stack_size)
{
	v_addr_t stack_top;
	int err;

	stack_top = USER_STACK_TOP;
	*stack_bottom = stack_top - stack_size;

	err = vmm_create_user_mapping(*stack_bottom, stack_size,
								  VMM_PROT_USER | VMM_PROT_WRITE,
								  VMM_MAP_GROWSDOW);

	return err;
}

static int load_binary(const char* path, struct process_image* img)
{
	vfs_path_t exec_path;
	struct vfs_file file;
	int err;

	err = vfs_path_init(&exec_path, path, strlen(path));
	if (err)
		return err;

	err = vfs_open(&exec_path, O_RDONLY, &file);
	if (err)
		goto fail_open;

	err = elf_load_binary(&file, img);

	vfs_close(&file);
	vfs_file_reset(&file);
fail_open:
	vfs_path_reset(&exec_path);

	return err;
}

static inline const char* get_new_process_name(const char* path,
											   const user_args_t* argv)
{
	return (argv && argv->size > 0) ? user_args_get_arg(argv, 0) : path;
}

int exec(const char* path, const user_args_t* argv, const user_args_t* envp)
{
	struct thread* thread = sched_get_current_thread();
	struct process* proc = thread->process;
	struct vmm* proc_vmm = proc->vmm;

	struct thread* new_thread = NULL;
	struct vmm* new_vmm = NULL;
	struct process_image new_img;
	v_addr_t stack_bottom, stack_top;
	int err;

	err = vmm_create(&new_vmm);
	if (err)
		return err;

	process_lock(proc, thread);
	process_set_vmm(proc, new_vmm);
	vmm_switch_to(new_vmm);

	err = load_binary(path, &new_img);
	if (err)
		goto fail;

	err = create_user_stack(&stack_bottom, USER_STACK_SIZE);
	if (err)
		goto fail;

	err = setup_user_args(stack_bottom, USER_STACK_SIZE, argv, envp, &stack_top);
	if (err)
		goto fail;

	err = thread_create(new_img.entry_point, stack_top, &new_thread);
	if (err)
		goto fail;

	process_set_name(proc, get_new_process_name(path, argv));
	process_set_process_image(proc, &new_img);

	vmm_unref(proc_vmm);
	process_exec(proc);
	process_unlock(proc);

	kassert(process_add_thread(proc, new_thread) == 0);
	kassert(sched_add_thread(new_thread) == 0);
	thread_unref(new_thread);

	sched_exit();

fail:
	vmm_switch_to(proc_vmm);
	process_set_vmm(proc, proc_vmm);
	vmm_unref(new_vmm);
	process_unlock(proc);

	return err;
}

int sys_execve(const char* __user path, char* const __user argv[],
			   char* const __user envp[])
{
	char* kpath;
	user_args_t kargv;
	user_args_t kenvp;
	int err;

	err = strndup_from_user(path, VFS_PATH_MAX_LEN, &kpath);
	if (err)
		return err;

	err = user_args_copy_from_user(argv, &kargv);
	if (err)
		goto fail_argv;

	err = user_args_copy_from_user(envp, &kenvp);
	if (err)
		goto fail_envp;

	err = exec(kpath, &kargv, &kenvp);

fail_envp:
	user_args_reset(&kenvp);
fail_argv:
	user_args_reset(&kargv);

	kfree(kpath);

	return err;
}
