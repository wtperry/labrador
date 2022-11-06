#include <bootloader/boot_spec.h>

#include <kernel/pmm.h>
#include <kernel/tty.h>
#include <kernel/vmm.h>
#include <kernel/fs/vfs.h>
#include <kernel/dev/ramdisk.h>
#include <kernel/log.h>
#include <kernel/dev/serial.h>

#include <kernel/arch/apic.h>
#include <kernel/arch/cpu.h>
#include <kernel/arch/pci.h>
#include <kernel/arch/smp.h>
#include <kernel/arch/time.h>
#include <kernel/arch/userspace.h>

#include <libk/stdio.h>

boot_info *info;

extern void generic_early(void);
extern void kernel_main(void);

void arch_main(boot_info *binfo) {
	log_init();
	init_serial();
	log_add_output(LOG_DEBUG, write_serial);
	log_printf(LOG_INFO, "Kernel log initialized!");
	log_printf(LOG_FATAL, "This is a fatal error");
	log_printf(LOG_ERROR, "This is an error");
	log_printf(LOG_WARNING, "This is a warning");
	log_printf(LOG_INFO, "This is an info message");
	log_printf(LOG_DEBUG, "This is a debug message");
	

    info = binfo;
    info = vmm_preinit(info);
	terminal_initialize(info->fb_ptr, info->fb_width, info->fb_height, info->fb_scanline, info->font);
	pmm_init(&info->mmap, info->num_mmap_entries);
	vmm_init(info);

    set_gs_base((uint64_t)&processor_local_data[0]);

	syscall_init();

    apic_init();
    
    enable_interrupts();
    
    clock_init();

    generic_early();

	pci_enumerate(info->rsdp);

	while(1) {
		asm("hlt");
	}

    apic_timer_init(1000);

    ramdisk_init();
	create_ramdisk_from_address(info->initrd, info->initrd_size, RAMDISK_READ_ONLY, "ram0");

	vfs_mkdir("/boot");
	if (vfs_mount("tar", "/dev/ram0", "/boot")) {
		printf("Initrd Mount Failed!\n");
		while (1) {
			asm("hlt");
		}
	}

    kernel_main();
}