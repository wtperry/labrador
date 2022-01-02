CFLAGS?=-g
LDFLAGS?=

CC=x86_64-w64-mingw32-gcc

SYSROOT=../sysroot

DESTDIR:=$(SYSROOT)
PREFIX:=/usr
EXEC_PREFIX:=$(PREFIX)
INCLUDEDIR:=$(PREFIX)/include
LIBDIR:=$(EXEC_PREFIX)/lib

CC:=$(CC) --sysroot=$(SYSROOT) -isystem=$(INCLUDEDIR)

CFLAGS:=$(CFLAGS) -Wall -Wextra -Wpedantic -ffreestanding -I/usr/include/efi -I/usr/include/efi/x86_64 -Iinclude
LDFLAGS:=$(LDFLAGS) -nostdlib -Wl,-dll -shared -Wl,--subsystem,10

OBJS=\
efi_helper.o \
psf1_helper.o \
main.o

.PHONY: all clean mkiso run debug install-headers

all: BOOTX64.EFI

BOOTX64.EFI: $(OBJS)
	@echo $(OBJS)
	@$(CC) $(LDFLAGS) -e efi_main -o $@ $(OBJS)

%.o: %.c Makefile
	@$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

clean:
	@rm -f BOOTX64.EFI
	@rm -f $(OBJS) *.o */*.o */*/*.o
	@rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install-headers:
	@mkdir -p $(DESTDIR)$(INCLUDEDIR)
	@cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

-include $(OBJS:.o=.d)