project('dummyOS', 'c', default_options: ['c_std=c11'])

# CFLAGS
c_flags = ['-ffreestanding', '-Wall', '-Wpedantic', '-pipe', '-D__KERNEL__']

# config.h
conf_data = configuration_data()

# QEMU options
qemu_machine_options = []

subdir('arch')
subdir('fs')
subdir('kernel')
subdir('libk')

# generate config.h
configure_file(output : 'config.h',
  configuration : conf_data,
  )

# source files and headers
src = arch_src + fs_src + kernel_src + libk_src
inc = include_directories('include', join_paths('arch', arch, 'include'), '.')
obj = []
if arch == 'arm'
  obj += '/home/manu/dummyOS_user/archive.o'
endif

# link
ld_flags = [
  '-n',
  '-T@0@/@1@'.format(meson.current_source_dir(), linker_script[0]),
  '-nostdlib'
  ]
# libgcc
cc = meson.get_compiler('c')
libgcc = cc.find_library('gcc', required: true)

# build targets
kernel_elf = 'dummy_os.elf'
elf = executable(kernel_elf,
  src,
  include_directories: inc,
  implicit_include_directories: false,
  c_args: c_flags,
  link_args: ld_flags,
  link_depends: linker_script,
  dependencies: libgcc,
  objects: obj,
  )

kernel_img = 'kernel.img'
if get_option('machine') == 'rpi2'
  kernel_img = 'kernel7.img'
endif
img = custom_target('kernel.img',
  output: kernel_img,
  command: [find_program('objcopy'), kernel_elf, '-O', 'binary', kernel_img],
  depends: elf
  )

kernel_iso = 'dummy_os.iso'
iso = custom_target('iso',
  output: kernel_iso,
  command: [files(join_paths('scripts', 'iso.sh')), kernel_elf, '@OUTPUT@', '/home/manu/dummyOS_user/archive'],
  depends: elf
  )

# run targets
qemu_script = join_paths('scripts', 'qemu.sh')
qemu = [qemu_script, arch] + qemu_machine_options
run_target('run',
  command: qemu + ['-kernel', kernel_elf, '-initrd', '/home/manu/dummyOS_user/archive'],
  depends: elf
  )
run_target('run-iso',
  command: qemu + ['-cdrom', kernel_iso],
  depends: iso
  )
run_target('debug',
  command: qemu + ['-kernel', kernel_elf, '-initrd', '/home/manu/dummyOS_user/archive', '-s', '-S'],
  depends: elf
  )
