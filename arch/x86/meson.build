arch_src = files(
  'arch.c',
  'cpu.c',
  'cpu_context.S',
  'cpu_context.c',
  'debug.c',
  'exception.S',
  'exception.c',
  'gdt.c',
  'i8254.c',
  'i8259.c',
  'idt.c',
  'irq.S',
  'irq.c',
  'syscall.S',
  'tss.c',
  )

subdir('boot')
arch_src += arch_boot_src

subdir('drivers')
arch_src += arch_drivers_src

subdir('mm')
arch_src += arch_mm_src

linker_script = files('kernel.ld')
