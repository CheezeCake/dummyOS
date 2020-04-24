project('dummyOS', 'c', default_options: ['c_std=c11'])

supported_archs = ['x86']
arch = get_option('arch')

subdir('arch')
subdir('fs')
subdir('kernel')
subdir('libk')

# source files and headers
src = arch_src + fs_src + kernel_src + libk_src
inc = include_directories('include', join_paths('arch', arch, 'include'))

# flags
c_flags = ['-ffreestanding', '-Wall', '-Wpedantic', '-pipe'] # -ggdb
if host_machine.endian() == 'little'
  c_flags += '-DARCH_ENDIAN_LITTLE'
else
  c_flags += '-DARCH_ENDIAN_BIG'
endif
c_flags += '-D__KERNEL__'
ld_flags = [
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
  dependencies: libgcc
  )

kernel_iso = 'dummy_os.iso'
iso = custom_target('iso',
  output: kernel_iso,
  command: [files(join_paths('scripts', 'iso.sh')), kernel_elf, '@OUTPUT@', '/home/manu/dummyOS_user/archive'],
  depends: elf
  )

# run targets
qemu = join_paths('scripts', 'qemu.sh')
run_target('run',
  command: [qemu, arch, '-kernel', kernel_elf, '-initrd', '/home/manu/dummyOS_user/archive'],
  depends: elf
  )
run_target('run-iso',
  command: [qemu, arch, '-cdrom', kernel_iso],
  depends: iso
  )
run_target('debug',
  command: [qemu, arch, '-kernel', kernel_elf, '-initrd', '/home/manu/dummyOS_user/archive', '-s', '-S'],
  depends: elf
  )
