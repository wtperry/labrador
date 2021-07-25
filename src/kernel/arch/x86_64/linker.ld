/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(_start)

KERNEL_VMA = 0xFFFFFFFF80000000;

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
	. = KERNEL_VMA;

	_kernel_start = .;

	/* Next we'll put the .text section. */
	.text BLOCK(4K) : AT(ADDR(.text) - KERNEL_VMA)
	{
		*(.text)
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : AT(ADDR(.rodata) - KERNEL_VMA)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : AT(ADDR(.data) - KERNEL_VMA)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : AT(ADDR(.bss) - KERNEL_VMA)
	{
		*(.bss)
		*(COMMON)
	}

	/* The compiler may produce other sections, put them in the proper place in
	   in this file, if you'd like to include them in the final kernel. */

	_kernel_end = .;

	/DISCARD/ :
	{
		*(.comment)
	}
}