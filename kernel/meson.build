kernel_src = files(
  'exec.c',
  'exit.c',
  'fork.c',
  'init.c',
  'kernel.c',
  'kernel_image.c',
  'kheap.c',
  'kmalloc.c',
  'process.c',
  'sys.c',
  'syscall.c',
  'thread.c',
  )

subdir('locking')
kernel_src += kernel_locking_src

subdir('mm')
kernel_src += kernel_mm_src

subdir('sched')
kernel_src += kernel_sched_src

subdir('time')
kernel_src += kernel_time_src
