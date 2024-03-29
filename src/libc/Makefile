HOST?=x86_64-elf
HOSTARCH=x86_64

CC=$(HOST)-gcc
AR=$(HOST)-ar

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
CPPFLAGS:=$(CPPFLAGS) -D__is_libc -Iinclude
LIBK_CFLAGS:=$(CFLAGS)
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__is_libk
 
ARCHDIR=arch/$(HOSTARCH)
 
include $(ARCHDIR)/make.config
 
CFLAGS:=$(CFLAGS) $(ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(ARCH_CPPFLAGS)
LIBK_CFLAGS:=$(LIBK_CFLAGS) $(KERNEL_ARCH_CFLAGS)
LIBK_CPPFLAGS:=$(LIBK_CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
 
FREEOBJS=\
$(ARCH_FREEOBJS) \
ctype/isdigit.o \
ctype/isspace.o \
stdio/_vsnprintf.o \
stdio/printf.o \
stdio/snprintf.o \
stdio/putchar.o \
stdio/puts.o \
stdlib/abort.o \
stdlib/atoi.o \
stdlib/malloc.o \
string/memcmp.o \
string/memmove.o \
string/memset.o \
string/strlen.o \
string/strcmp.o \
string/strdup.o \
string/strcpy.o \
string/strncat.o \
 
HOSTEDOBJS=\
$(ARCH_HOSTEDOBJS) \
 
OBJS=\
$(FREEOBJS) \
$(HOSTEDOBJS) \
 
LIBK_OBJS=$(FREEOBJS:.o=.libk.o)
 
#BINARIES=libc.a libk.a # Not ready for libc yet.
BINARIES=libk.a
 
.PHONY: all clean install install-headers install-libs
.SUFFIXES: .o .libk.o .c .S
 
all: install-libs
 
libc.a: $(OBJS)
	@$(AR) rcs $@ $(OBJS)
 
libk.a: $(LIBK_OBJS)
	@$(AR) rcs $@ $(LIBK_OBJS)
 
.c.o:
	@$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)
 
.S.o:
	@$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)
 
.c.libk.o:
	@$(CC) -MD -c $< -o $@ -std=gnu11 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)
 
.S.libk.o:
	@$(CC) -MD -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)
 
clean:
	@rm -f $(BINARIES) *.a
	@rm -f $(OBJS) $(LIBK_OBJS) *.o */*.o */*/*.o
	@rm -f $(OBJS:.o=.d) $(LIBK_OBJS:.o=.d) *.d */*.d */*/*.d
 
install: install-headers install-libs
 
install-headers:
	@mkdir -p $(DESTDIR)$(INCLUDEDIR)
	@cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.
 
install-libs: $(BINARIES)
	@mkdir -p $(DESTDIR)$(LIBDIR)
	@cp $(BINARIES) $(DESTDIR)$(LIBDIR)
 
-include $(OBJS:.o=.d)
-include $(LIBK_OBJS:.o=.d)