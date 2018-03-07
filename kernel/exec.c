#include <arch/vm.h>
#include <fs/file.h>
#include <fs/vfs.h>
#include <kernel/elf.h>
#include <kernel/errno.h>
#include <kernel/exec.h>
#include <kernel/memory.h>
#include <kernel/paging.h>
#include <kernel/sched/sched.h>
#include <kernel/usermode.h>
#include <libk/libk.h>

static int unschedule_process_threads(struct process* proc,
									  struct thread* current_thread)
{
	int err;
	list_node_t* it;

	list_foreach(&proc->threads, it) {
		struct thread* thread = list_entry(it, struct thread, p_thr_list);

		if (current_thread != thread) {
			thread->state = THREAD_STOPPED;
			if ((err = sched_remove_thread(thread)) != 0)
				return err;
		}
	}

	return 0;
}

static int prepare_exec(struct process* proc, struct thread* current_thread)
{
	int err;

	err = unschedule_process_threads(proc, current_thread);
	if (err)
		return err;

	process_prepare_exec(proc, current_thread);

	return err;
}

static int create_stack(v_addr_t* stack)
{
	p_addr_t frame;
	int err;

	frame = memory_page_frame_alloc();
	if (!frame)
		return -ENOMEM;

	*stack = USER_VADDR_SPACE_END;
	err = paging_map(frame, *stack - PAGE_SIZE, VM_FLAG_USER | VM_FLAG_WRITE);

	return err;
}

int exec(const char* __kernel path, char* const __kernel argv[],
		 char* const __kernel envp[])
{
	struct thread* thread = sched_get_current_thread();
	struct process* proc = thread->process;
	v_addr_t entry_point, stack;
	int err;

	vfs_path_t exec_path;
	err = vfs_path_init(&exec_path, path, strlen(path));
	if (err)
		return err;

	struct vfs_cache_node* exec;
	err = vfs_lookup(&exec_path, proc->root, proc->cwd, &exec);
	if (err)
		goto fail_lookup;

	struct vfs_file file;
	vfs_file_init(&file, exec, O_RDONLY);
	err = exec->inode->op->open(exec->inode, exec, O_RDONLY, &file);
	if (err)
		goto fail_file;

	// TODO: save usr context
	// clear process
	err = prepare_exec(proc, thread);
	if (err)
		goto fail_load_bin;

	err = elf_load_binary(&file, &entry_point);
	if (err)
		goto fail_load_bin;

	err = create_stack(&stack);
	if (err)
		goto fail_stack;


	// no return
	usermode_entry(entry_point, stack);

fail_stack:

fail_load_bin:
	exec->inode->op->close(exec->inode, &file);
fail_file:
	vfs_cache_node_unref(exec);
fail_lookup:
	vfs_path_reset(&exec_path);

	return err;
}

int sys_execve(const char* __user path, char* const __user argv[],
			   char* const __user envp[])
{
	// copy_from_user args
	return exec(NULL, NULL, NULL);
}
