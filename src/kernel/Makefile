HOST?=x86_64-elf
HOSTARCH=x86_64

CC=$(HOST)-gcc

SYSROOT=../sysroot

CFLAGS?=-g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR:=$(SYSROOT)
PREFIX:=/usr
EXEC_PREFIX:=$(PREFIX)
INCLUDEDIR:=$(PREFIX)/include
LIBDIR:=$(EXEC_PREFIX)/lib

CC:=$(CC) --sysroot=$(SYSROOT) -isystem=$(INCLUDEDIR)

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -Wpedantic
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel -Iinclude -I/usr/include/efi -I/usr/include/efi/x86_64
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk -lgcc

ARCHDIR=arch/$(HOSTARCH)

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS=\
$(KERNEL_ARCH_OBJS) \
kernel/kernel.o \
kernel/heap.o \
kernel/dev/ramdisk.o \
kernel/ds/tree.o \
kernel/ds/list.o \
kernel/fs/vfs.o \
kernel/fs/tmpfs.o \
kernel/fs/tarfs.o \
kernel/fs/devfs.o \
kernel/dev/serial.o \
kernel/tasking.o \
kernel/spinlock.o \
kernel/log/log.o \
kernel/dev/fbvideo.o

#OBJS=\
$(ARCHDIR)/crti.o \
$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/crtn.o \

OBJS=\
$(KERNEL_OBJS) \

#LINK_LIST=\
$(LDFLAGS) \
$(ARCHDIR)/crti.o \
$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(LIBS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/crtn.o \

LINK_LIST=\
$(KERNEL_OBJS) \
$(LIBS) \

.PHONY: all clean install-headers

all: kernel.elf

kernel.elf: $(OBJS) $(ARCHDIR)/linker.ld
	@echo $(OBJS)
	@$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) -z max-page-size=0x1000 $(LINK_LIST)

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

%.o: %.c Makefile
	@$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

%.o: %.S Makefile
	@$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

install-headers:
	@mkdir -p $(DESTDIR)$(INCLUDEDIR)
	@cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.
	@mkdir -p $(DESTDIR)$(INCLUDEDIR)/arch
	@cp -R --preserve=timestamps $(ARCHDIR)/include/. $(DESTDIR)$(INCLUDEDIR)/.

clean:
	@rm -f kernel.elf
	@rm -f $(OBJS) *.o */*.o */*/*.o
	@rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

-include $(OBJS:.o=.d)