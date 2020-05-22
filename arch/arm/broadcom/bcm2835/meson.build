linker_script = files('kernel.ld')

machine_src = files(
  'bcm2835.c',
  'gpio.c',
  'irq.c',
  'log.c',
  'timer.c',
  'uart.c',
  )

subdir('mailbox')

machine_src += machine_mailbox_src
