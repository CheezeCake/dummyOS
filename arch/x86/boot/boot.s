# constants for the multiboot header
.set ALIGN,	1 << 0 # align loaded modules on page boundaries
.set MEMINFO,	1 << 1 # provide memory map
.set VIDEOINFO, 1 << 2 # provide video mode table
#.set MULTIBOOT_FLAGS, ALIGN | MEMINFO | VIDEOINFO # multiboot flags field
.set MULTIBOOT_FLAGS, ALIGN | MEMINFO # multiboot flags field
.set MULTIBOOT_MAGIC, 0x1BADB002 # magic number: mark for bootloader
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

# virtual memory constants
.set KERNEL_VMA, 0xc0000000 # kernel Virtual Memory Address: 3GB
.set KERNEL_PAGE_DIRECTORY_INDEX, (KERNEL_VMA >> 22) # 3GB / 4MB
.set PAGE_DIRECTORY_SIZE, 1024

# page directory physical address
.globl boot_page_directory_phys
.set boot_page_directory_phys, boot_page_directory - KERNEL_VMA
# page table physical address
.globl boot_page_table_phys
.set boot_page_table_phys, boot_page_table - KERNEL_VMA


# Multiboot header. We will force the .multiboot section to come first in
# the kernel in the linker script. It must be in the first 8192 bytes.
.section .multiboot
.align 4 # The multiboot header must be aligned to a 4-byte boundary
.long MULTIBOOT_MAGIC
.long MULTIBOOT_FLAGS
.long MULTIBOOT_CHECKSUM


.globl __stack_top

# allocate the kernel's stack
.section .bss
.align 16
__stack_bottom:
.skip 0x4000 # 16KB
__stack_top:

.globl boot_page_directory
.globl boot_page_table

.data

# Allocate the initial page directory and page table:
# This assumes the kernel is smaller than 3MB.
# The kernel is loaded in physical memory at 1MB (0x100000).
# Fill the page directory and the page table to have the physical
# memory 0 -> 4MB identity mapped, so the CPU can fetch the next
# instruction after enabling paging.
# Also map 0 -> 4MB to the kernel virtual address (0xc0000000)
# The first mapping can be later disabled so we are only left with
# the higher-half mapping.
.align 0x1000
boot_page_table:
.set addr, 0
.rept 1024
.long addr | 3 # set bit 1 (present) and 2 (r/w)
.set addr, addr + 0x1000
.endr

.align 0x1000
boot_page_directory:
.rept 1024
.long (boot_page_table - 0xc0000000) + 1
.endr


.globl _start

.text
_start:
	# enable paging:
	# cr3 = page directory physical address
	# set PG flag in cr0
	# (Intel Architecture Software Developer’s Manual Volume 3, section 2.5)
	movl $boot_page_directory_phys, %eax
	movl %eax, %cr3
	movl %cr0, %eax
	orl $0x80000000, %eax
	movl %eax, %cr0

	# absolute jump into higher-half kernel
	movl $higher_half, %ecx
	jmp *%ecx

higher_half:
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
	addl $KERNEL_VMA, %ebx # convert to virtual
	pushl %ebx

	# call the C main function
	call __kernel_main

	cli # disable interrupts
	hlt
.Lhang:
	jmp .Lhang
