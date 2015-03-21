# constants for the multiboot header
.set ALIGN,	1 << 0 # align loaded modules on page boundaries
.set MEMINFO,	1 << 1 # provide memory map
.set VIDEOINFO, 1 << 2 # provide video mode table
#.set MULTIBOOT_FLAGS, ALIGN | MEMINFO | VIDEOINFO # multiboot flags field
.set MULTIBOOT_FLAGS, ALIGN | MEMINFO # multiboot flags field
.set MULTIBOOT_MAGIC, 0x1BADB002 # magic number: mark for bootloader
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)


# Multiboot header. We will force the .multiboot section to come first in
# the kernel in the linker script. It must be in the first 8192 bytes.
.section .multiboot
.align 4 # The multiboot header must be aligned on a 4 byte boundary
.long MULTIBOOT_MAGIC
.long MULTIBOOT_FLAGS
.long MULTIBOOT_CHECKSUM


# allocate our own temporary stack
.set STACK_SIZE, 0x4000 # 16KB
.comm stack, STACK_SIZE


.text
.globl _start
_start:
	movl $(stack + STACK_SIZE), %ebp
	movl %ebp, %esp

	# set EFLAGS register to 0
	pushl $0
	popf

	# call the C main function
	call kernel_main

	cli # disable interrupts
	hlt
.Lhang:
	jmp .Lhang
