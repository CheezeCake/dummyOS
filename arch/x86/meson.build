arch_src = files(
  'arch.c',
  'context_switch.c',
  'cpu.c',
  'cpu_context.c',
  'cpu_context_switch.s',
  'debug.c',
  'exception.c',
  'gdt.c',
  'i8254.c',
  'i8259.c',
  'idt.c',
  'interrupt.S',
  'irq.c',
  'paging.c',
  'syscall.S',
  'tss.c',
  'usermode.c',
  'vm_context.c'
  )

subdir('boot')
subdir('drivers')

arch_src += arch_boot_src
arch_src += arch_drivers_src

linker_script = files('kernel.ld')
