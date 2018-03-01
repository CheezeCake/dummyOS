project('dummyOS', 'c', default_options: ['c_std=c11'])

supported_archs = ['x86']
arch = get_option('arch')

subdir('arch')
subdir('fs')
subdir('kernel')
subdir('libk')

src = arch_src + fs_src + kernel_src + libk_src
inc = include_directories('include', join_paths('arch', arch, 'include'))

c_flags = ['-ffreestanding', '-Wall', '-pipe'] # -ggdb
ld_flags = [
  '-T@0@/@1@'.format(meson.current_source_dir(), linker_script[0]),
  '-nostdlib'
  ]

cc = meson.get_compiler('c')
libgcc = cc.find_library('gcc', required: true)

kernel_bin = 'dummy_os.bin'
bin = executable(kernel_bin,
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
  command: [files('scripts/iso.sh'), kernel_bin, '@OUTPUT@'],
  depends: bin
  )

qemu = 'scripts/qemu.sh'
run_target('run', command: [qemu, 'i386', kernel_bin], depends: bin)
run_target('run-iso', command: [qemu, 'i386', kernel_iso], depends: iso)
run_target('debug', command: [qemu, 'i386', kernel_bin, '-s', '-S'], depends: bin)
