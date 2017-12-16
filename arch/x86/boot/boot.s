# constants for the multiboot header
.set ALIGN,	1 << 0 # align loaded modules on page boundaries
.set MEMINFO,	1 << 1 # provide memory map
.set VIDEOINFO, 1 << 2 # provide video mode table
#.set MULTIBOOT_FLAGS, ALIGN | MEMINFO | VIDEOINFO # multiboot flags field
.set MULTIBOOT_FLAGS, ALIGN | MEMINFO # multiboot flags field
.set MULTIBOOT_MAGIC, 0x1BADB002 # magic number: mark for bootloader
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

.set KERNEL_VMA, 0xc0000000 # 3GB
.set KERNEL_PAGE_DIRECTORY_INDEX, (KERNEL_VMA >> 22) # 3GB / 4MB
.set PAGE_DIRECTORY_SIZE, 1024

# page directory physical address
.globl __boot_page_directory_phys
.set __boot_page_directory_phys, boot_page_directory - KERNEL_VMA
# page directory virtual address
.globl __boot_page_directory_virt
.set __boot_page_directory_virt, boot_page_directory

.globl __boot_page_table_phys
.set __boot_page_table_phys, boot_page_table - KERNEL_VMA


# Multiboot header. We will force the .multiboot section to come first in
# the kernel in the linker script. It must be in the first 8192 bytes.
.section .multiboot
.align 4 # The multiboot header must be aligned on a 4 byte boundary
.long MULTIBOOT_MAGIC
.long MULTIBOOT_FLAGS
.long MULTIBOOT_CHECKSUM


# allocate the kernel's stack
.section .bss
.align 16
__stack_bottom:
.skip 0x4000 # 16KB
__stack_top:
.globl __stack_top



.section .data
.align 0x1000
boot_page_directory:
.fill 1024, 4, 0

.align 0x1000
boot_page_table:
.fill 1024, 4, 0


.text
# the entry point in the linker script is _start
# setup _start to where start is loaded in physical memory
.globl _start
.set _start, start - 0xc0000000
start:
	# page directory
	movl $(boot_page_directory - KERNEL_VMA), %edi
	xorl %ecx, %ecx

	# 0x1003: address=0x4000, r/w, present
	movl $(boot_page_table - KERNEL_VMA), %eax
	orl $3, %eax
	movl %eax, (%edi)
	movl $KERNEL_PAGE_DIRECTORY_INDEX, %ecx
	movl %eax, (%edi, %ecx, 4)

	# page table
	movl $(boot_page_table - KERNEL_VMA), %edi
	xorl %ecx, %ecx
	xorl %eax, %eax
repeat:
	movl %eax, %edx
	orl $3, %edx
	movl %edx, (%edi, %ecx, 4)
	addl $0x1000, %eax
	incl %ecx
	cmpl $1024, %ecx
	jne repeat

	movl $(boot_page_directory - KERNEL_VMA), %eax
	movl %eax, %cr3


	# enable paging
	movl %cr0, %eax
	orl $0x80000000, %eax
	movl %eax, %cr0
in_higher_half:
	# setup stack
	movl $__stack_top, %esp

	# stacktrace sentinel
	pushl $0 # ret eip
	pushl $0 # old ebp
	movl %esp, %ebp

	# set EFLAGS register to 0
	pushl $0
	popf

	# GRUB multiboot_info_t struct
	pushl %ebx

	# call the C main function
	call kernel_main

	cli # disable interrupts
	hlt
.Lhang:
	jmp .Lhang
