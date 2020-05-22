# source files
arch_src = files(
  'arch.c',
  'atags.c',
  'cpu.c',
  'cpu_context.S',
  'cpu_context.c',
  'exception.S',
  'exception.c',
  'machine.c',
  'syscall.S',
  )

if machine.startswith('rpi')
  subdir('broadcom/bcm2835')
endif
if machine == 'rpi2'
  subdir('broadcom/bcm2836')
endif

arch_src += machine_src

# machine specific CFLAGS
# QEMU options
# armv value
# config
if machine == 'rpi1'
  c_flags += '-mcpu=arm1176jzf-s'
  conf_data.set('RPI_MODEL', 1)
  armv=6
elif machine == 'rpi2'
  c_flags += '-mcpu=cortex-a7'
  qemu_machine_options = ['-M', 'raspi2', '-serial', 'stdio', '-serial', 'tcp:localhost:8888']
  conf_data.set('RPI_MODEL', 2)
  armv=7
endif

subdir('v' + armv.to_string())

arch_src += armv_src
