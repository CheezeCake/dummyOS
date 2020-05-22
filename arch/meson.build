arm_machines = ['rpi1', 'rpi2']
machine = get_option('machine')

if machine == 'pc'
  arch = 'x86'
elif arm_machines.contains(machine)
  arch = 'arm'
else
  error('Unsupported machine: ' + machine)
endif

# source files
subdir(arch)

# config.h
if host_machine.endian() == 'little'
  conf_data.set('ARCH_ENDIAN_LITTLE', true)
else
  conf_data.set('ARCH_ENDIAN_BIG', true)
endif
