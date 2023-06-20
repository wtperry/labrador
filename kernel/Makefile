CC=x86_64-elf-gcc
KERNEL:=kernel.elf

CFLAGS?=-g

BUILD_DIR:=../build/kernel

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -Wpedantic -mno-red-zone -mcmodel=kernel -mno-mmx -mno-sse -mno-sse2
CPPFLAGS:=-MMD -MP
LDFLAGS:=-nostdlib -z max-page-size=0x1000

INCLUDE:=../limine ../include

CPPFLAGS:=$(CPPFLAGS) $(INCLUDE:%=-I%)

CFILES:=$(shell find -L . -type f -name '*.c')
ASMFILES:=$(shell find -L . -type f -name '*.S')

OBJ:=$(CFILES:%=$(BUILD_DIR)/%.o) $(ASMFILES:%=$(BUILD_DIR)/%.o)
DEPS:=$(CFILES:%=$(BUILD_DIR)/%.d) $(ASMFILES:%=$(BUILD_DIR)/%.d)

.PHONY: all

all: $(BUILD_DIR)/$(KERNEL)

$(BUILD_DIR)/$(KERNEL): $(OBJ) linker.ld
	@$(CC) -T linker.ld -o $@ $(CFLAGS) $(LDFLAGS) $(OBJ)

$(BUILD_DIR)/%.c.o: %.c Makefile
	@mkdir -p $(dir $@)
	@$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

$(BUILD_DIR)/%.S.o: %.S Makefile
	@mkdir -p $(dir $@)
	@$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(DEST_DIR)/$(KERNEL)

-include $(DEPS)