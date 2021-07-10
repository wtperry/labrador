/* Virtual address base */
.set VIRTUAL_BASE, 0xC0000000

/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

/* 
Declare a multiboot header that marks the program as a kernel. These are magic
values that are documented in the multiboot standard. The bootloader will
search for this signature in the first 8 KiB of the kernel file, aligned at a
32-bit boundary. The signature is in its own section so the header can be
forced to be within the first 8 KiB of the kernel file.
*/
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*
The multiboot standard does not define the value of the stack pointer register
(esp) and it is up to the kernel to provide a stack. This allocates room for a
small stack by creating a symbol at the bottom of it, then allocating 16384
bytes for it, and finally creating a symbol at the top. The stack grows
downwards on x86. The stack is in its own section so it can be marked nobits,
which means the kernel file is smaller because it does not contain an
uninitialized stack. The stack on x86 must be 16-byte aligned according to the
System V ABI standard and de-facto extensions. The compiler will assume the
stack is properly aligned and failure to align the stack will result in
undefined behavior.
*/
.section .bss
.align 16
stack_bottom:
.skip 4096 # 4 KiB
stack_top:

/*
Allocate a page table and single page directory. Additional may be required
if kernel is greater than 3 MB
*/
.section .page_data
.align 4096
page_directory:
.skip 4096
boot_page_table1:
.skip 4096

/*
The linker script specifies _start as the entry point to the kernel and the
bootloader will jump to this position once the kernel has been loaded. It
doesn't make sense to return from this function as the bootloader is gone.
*/
.section .text
.global _start
.type _start, @function
_start:
	# store physical address of paging table
	movl $(boot_page_table1 - VIRTUAL_BASE), %edi
	# start at 0, map first 4 MB as identity map
	movl $0, %esi
	# map 1024 pages
	movl $1024, %ecx

1:
	# only map until the end of the kernel
	# cmpl $(_kernel_end - VIRTUAL_BASE), %esi
	# jge 2f
	# map physical address as present and writable
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

	# size of page is 4096 bytes
	addl $4096, %esi
	# size of entries in boot_page_table1 is 4 bytes
	addl $4, %edi
	# will go until ecx = 0 (1024 times)
	dec %ecx
	loop 1b

2:
	# map the page table to both virtual addresses 0x00000000 and 0xC0000000
	movl $(boot_page_table1 - VIRTUAL_BASE + 0x003), page_directory - VIRTUAL_BASE + 0
	movl $(boot_page_table1 - VIRTUAL_BASE + 0x003), page_directory - VIRTUAL_BASE + 768 * 4

	# set cr3 to the address of the page directory
	movl $(page_directory - VIRTUAL_BASE), %ecx
	movl %ecx, %cr3

	# enable paging and write-protect bit
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	# jump to higher half with an absolute jump
	lea higher_half, %ecx
	jmp *%ecx

higher_half:
	# paging is enabled, unmap identity mapping
	# movl $0, page_directory + 0
	# invlpg 0

	/*
	To set up a stack, we set the esp register to point to the top of our
	stack (as it grows downwards on x86 systems). This is necessarily done
	in assembly as languages such as C cannot function without a stack.
	*/
	mov $stack_top, %esp
	xor %ebp, %ebp  # set ebp to NULL

	/*
	Push the multiboot info onto the stack to allow the kernel main function
	to access the data
	*/
	# add $VIRTUAL_BASE, %ebx
	push %eax
	push %ebx

	/*
	This is a good place to initialize crucial processor state before the
	high-level kernel is entered. It's best to minimize the early
	environment where crucial features are offline. Note that the
	processor is not fully initialized yet: Features such as floating
	point instructions and instruction set extensions are not initialized
	yet. The GDT should be loaded here. Paging should be enabled here.
	C++ features such as global constructors and exceptions will require
	runtime support to work as well.
	*/

	# Call the global constructors
	call _init

	/*
	Enter the high-level kernel. The ABI requires the stack is 16-byte
	aligned at the time of the call instruction (which afterwards pushes
	the return pointer of size 4 bytes). The stack was originally 16-byte
	aligned above and we've since pushed a multiple of 16 bytes to the
	stack since (pushed 0 bytes so far) and the alignment is thus
	preserved and the call is well defined.
	*/
	call kernel_main

	/*
	If the system has nothing more to do, put the computer into an
	infinite loop.
	*/

	cli
1:	hlt
	jmp 1b

.global load_gdt
load_gdt:
	mov 4(%esp), %eax
	lgdt (%eax)

	mov $0x10, %eax
	mov %eax, %ds
	mov %eax, %es
	mov %eax, %fs
	mov %eax, %gs
	mov %eax, %ss
	jmp $0x8, $.long_jump
.long_jump:
	ret

.global load_idt
load_idt:
	mov 4(%esp), %eax
	lidt (%eax)
	ret

.global gen_interrupt
gen_interrupt:
	mov 1(%esp), %al
	mov %al, .genint+1
.genint:
	int $0x00
	ret