BOOT   := boot.bin
KERNEL := kernel.bin
IMAGE  := disk.bin
GAME   := game.bin

CC      := gcc
LD      := ld
OBJCOPY := objcopy
DD      := dd
QEMU    := qemu-system-i386
GDB     := gdb

CFLAGS := -Wall -Werror -Wfatal-errors #开启所有警告, 视警告为错误, 第一个错误结束编译
CFLAGS += -MD #生成依赖文件
CFLAGS += -std=gnu11
CFLAGS += -m32 -c #编译标准, 目标架构, 只编译
CFLAGS += -I . -I ./inc#头文件搜索目录
CFLAGS += -O0 #不开优化, 方便调试
CFLAGS += -fno-builtin #禁止内置函数
CFLAGS += -ggdb3 #GDB调试信息
CFLAGS += -fno-stack-protector #??
#CFLAGS += -fno-omit-frame-pointer

QEMU_OPTIONS := -serial stdio #以标准输入输为串口(COM1)
#QEMU_OPTIONS += -d int #输出中断信息
QEMU_OPTIONS += -monitor telnet:127.0.0.1:1111,server,nowait #telnet monitor

QEMU_DEBUG_OPTIONS := -S #启动不执行
QEMU_DEBUG_OPTIONS += -s #GDB调试服务器: 127.0.0.1:1234

GDB_OPTIONS := -ex "target remote 127.0.0.1:1234"
GDB_OPTIONS += -ex "symbol kernel.bak"

OBJ_DIR        := obj
LIB_DIR        := lib
BOOT_DIR       := boot
KERNEL_DIR     := kernel
GAME_DIR       := game

OBJ_LIB_DIR    := $(OBJ_DIR)/$(LIB_DIR)
OBJ_BOOT_DIR   := $(OBJ_DIR)/$(BOOT_DIR)
OBJ_KERNEL_DIR := $(OBJ_DIR)/$(KERNEL_DIR)
OBJ_GAME_DIR   := $(OBJ_DIR)/$(GAME_DIR)
 
LD_SCRIPT := $(shell find $(KERNEL_DIR) -name "*.ld")

LIB_C := $(wildcard $(LIB_DIR)/*.c)
LIB_O := $(LIB_C:%.c=$(OBJ_DIR)/%.o)

BOOT_S := $(wildcard $(BOOT_DIR)/*.S)
BOOT_C := $(wildcard $(BOOT_DIR)/*.c)
BOOT_O := $(BOOT_S:%.S=$(OBJ_DIR)/%.o)
BOOT_O += $(BOOT_C:%.c=$(OBJ_DIR)/%.o)

KERNEL_C := $(shell find $(KERNEL_DIR) -name "*.c")
KERNEL_S := $(shell find $(KERNEL_DIR) -name "*.S")
KERNEL_O := $(KERNEL_C:%.c=$(OBJ_DIR)/%.o)
KERNEL_O += $(KERNEL_S:%.S=$(OBJ_DIR)/%.o)

GAME_C	:= $(shell find $(GAME_DIR) -name "*.c")
GAME_O 	:= $(GAME_C:%.c=$(OBJ_DIR)/%.o)

#####################
$(IMAGE): $(BOOT) $(KERNEL) $(GAME)
	@@$(DD) if=/dev/zero of=$(IMAGE) count=10000         > /dev/null # 准备磁盘文件
	@@$(DD) if=$(BOOT) of=$(IMAGE) conv=notrunc          > /dev/null # 填充 boot loader
	@@$(DD) if=$(KERNEL) of=$(IMAGE) seek=1 conv=notrunc > /dev/null # 填充 kernel, 跨过 mbr
	./format disk $(GAME)
#	@@$(DD) if=$(GAME) of=$(IMAGE) seek=201 conv=notrunc > /dev/null # 不指定扇区会默认从头开始，会将boot冲掉
	@@$(DD) if=disk of=$(IMAGE) seek=201 conv=notrunc > /dev/null # 不指定扇区会默认从头开始，会将boot冲掉

#####################
$(BOOT): $(BOOT_O)
	@$(LD) -e start -Ttext=0x7C00 -m elf_i386 -nostdlib -o $@.out $^
	@$(OBJCOPY) --strip-all --only-section=.text --output-target=binary $@.out $@
	@@rm $@.out
	@boot/genboot.pl $@

$(OBJ_BOOT_DIR)/%.o: $(BOOT_DIR)/%.S
	@@mkdir -p $(OBJ_BOOT_DIR)
	@$(CC) $(CFLAGS)  -Os $< -o $@

$(OBJ_BOOT_DIR)/%.o: $(BOOT_DIR)/%.c
	@@mkdir -p $(OBJ_BOOT_DIR)
	@$(CC) $(CFLAGS) -Os $< -o $@

#####################
$(KERNEL): $(LD_SCRIPT)
$(KERNEL): $(KERNEL_O) $(LIB_O)
	$(LD) -m elf_i386 -T $(LD_SCRIPT) -nostdlib -o $@ $^ $(shell $(CC) $(CFLAGS) -I$(KERNEL_DIR) -print-libgcc-file-name)
	@cp $@ kernel.bak
	@./fill_kernel.sh $@

$(OBJ_LIB_DIR)/%.o : $(LIB_DIR)/%.c
	@@mkdir -p $(OBJ_LIB_DIR)
	@$(CC) $(CFLAGS) -I$(LIB_DIR) $< -o $@
#	@$(CC) $(CFLAGS) -I$(LIB_DIR) -I$(KERNEL_DIR) $< -o $@

$(OBJ_KERNEL_DIR)/%.o: $(KERNEL_DIR)/%.[cS]
	@mkdir -p $(OBJ_DIR)/$(dir $<)
#	@$(CC)  $< -o $@ $(CFLAGS) -I$(KERNEL_DIR)
	@$(CC)  $< -o $@ $(CFLAGS) -I$(KERNEL_DIR) -I$(LIB_DIR)

#####################
$(GAME): $(GAME_O) $(LIB_O)
	@$(LD) -m elf_i386 -e file_main -nostdlib -o $@ $^ $(shell $(CC) $(CFLAGS) -I$(GAME_DIR) -print-libgcc-file-name)
#	@$(LD) -m elf_i386 -e pc_problem -nostdlib -o $@ $^ $(shell $(CC) $(CFLAGS) -I$(GAME_DIR) -print-libgcc-file-name)

$(OBJ_GAME_DIR)/%.o: $(GAME_DIR)/%.c
	@mkdir -p $(OBJ_DIR)/$(dir $<)
#	@$(CC) $(CFLAGS) -I$(GAME_DIR) $< -o $@
	@$(CC) $(CFLAGS) -I ./game/inc -I$(GAME_DIR) -I$(LIB_DIR) $< -o $@

#####################
DEPS := $(shell find -name "*.d")
-include $(DEPS)

.PHONY: qemu debug gdb clean

qemu: $(IMAGE)
	@$(QEMU) $(QEMU_OPTIONS) $(IMAGE)

# Faster, but not suitable for debugging
qemu-kvm: $(IMAGE)
	@$(QEMU) $(QEMU_OPTIONS) --enable-kvm $(IMAGE)

debug: $(IMAGE)
	@$(QEMU) $(QEMU_DEBUG_OPTIONS) $(QEMU_OPTIONS) $(IMAGE)

gdb:
	@$(GDB) $(GDB_OPTIONS)

clean:
	@@rm -rf $(OBJ_DIR) 2> /dev/null
	@@rm -rf $(BOOT)    2> /dev/null
	@@rm -rf $(KERNEL)  2> /dev/null
	@@rm -rf $(IMAGE)   2> /dev/null
	@@rm -rf $(GAME)    2> /dev/null
