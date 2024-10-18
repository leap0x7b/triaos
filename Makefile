MKDOSFS_V := -v
ifneq ($(VERBOSE), 1)
	Q := @
	MKDOSFS_V :=
	DD_STATUS := status=none
endif

SHELL = bash
MKDIR = mkdir -p
MKCWD = $(MKDIR) $(@D)

TARGET ?= i386-pc
ifeq ($(filter $(TARGET),i386-pc m68k-mac),)
$(error Invalid target: $(TARGET))
endif

ARCH = $(word 1,$(subst -, ,$(TARGET)))
MACHINE = $(word 2,$(subst -, ,$(TARGET)))

SRCDIR = $(CURDIR)/src
INCLUDEDIR = $(CURDIR)/include
BUILDDIR = $(CURDIR)/build/$(TARGET)
EXTERNALDIR = $(CURDIR)/external
MOUNTDIR = $(BUILDDIR)/mount

ISO = $(BUILDDIR)/triaos.iso

ifeq ($(ARCH), i386)
CC = clang -target i386-pc-elf
AS = nasm
LD = ld.lld
OBJCOPY = llvm-objcopy

CFLAGS ?= -Os -gdwarf
ASFLAGS ?= -g -Fdwarf
LDFLAGS ?=

CHARDFLAGS := \
	-I$(INCLUDEDIR) \
	-nostdlib -std=c99 -pedantic \
	-ffreestanding -MMD -MP \
	-mno-80387 -mno-mmx -mno-3dnow \
	-mno-sse -mno-sse2
ASHARDFLAGS := -felf32 -MD -MP
LDHARDFLAGS := \
	-nostdlib -static \
	-L$(BUILDDIR) -ltria \
	-zmax-page-size=0x1000 \
	--no-dynamic-linker
else ifeq ($(ARCH), m68k)
CC = m68k-elf-gcc
AS = m68k-elf-gcc
LD = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy

CFLAGS ?= -Os -gdwarf
ASFLAGS ?= -gdwarf
LDFLAGS ?=

CHARDFLAGS := \
	-I$(INCLUDEDIR) \
	-nostdlib -std=c99 \
	-pedantic -ffreestanding \
	-march=68000 -MMD -MP
ASHARDFLAGS := \
	-I$(INCLUDEDIR) \
	-march=68000 -MMD
LDHARDFLAGS := \
	-nostdlib -static \
	-L$(BUILDDIR) -ltria
else
$(error Invalid target: $(TARGET))
endif

ifeq ($(TARGET), i386-pc)
QEMU = qemu-system-i386
QEMUFLAGS ?= -no-reboot -no-shutdown
QEMUMEMSIZE ?= 512M
QEMUHARDFLAGS := -machine q35,accel=kvm:whpx:hvf -debugcon stdio
else ifeq ($(TARGET), m68k-mac)
QEMU = qemu-system-m68k
QEMUFLAGS ?= -no-reboot -no-shutdown
QEMUMEMSIZE ?= 512M
QEMUHARDFLAGS := -machine q800 -bios ../mac/q800.rom -g 640x480x8
else
$(error Invalid target: $(TARGET))
endif

TRIABOOT_SHARED := $(BUILDDIR)/libtriaboot.a
LIB := $(BUILDDIR)/libtria.a
KERNEL := $(BUILDDIR)/triakrnl.trx
TRIAOS_IMG := $(BUILDDIR)/triaos.img
LOOPBACK := $(BUILDDIR)/loopback_dev

ifeq ($(TARGET), i386-pc)
TRIABOOT := $(BUILDDIR)/triaboot.bin
TRIABOOT_STAGE2 := $(BUILDDIR)/triaboot.s2
TRIABOOT_STAGE2_TRX := $(BUILDDIR)/triaboot.s2.trx

TRIABOOT_STAGE2_CFILES := \
	$(shell find $(SRCDIR)/boot -not \( -path $(SRCDIR)/boot/arch -prune \) -name *.c) \
	$(shell find $(SRCDIR)/boot/arch/$(ARCH) -not \( -path $(SRCDIR)/boot/arch/$(ARCH)/machines -prune \) -name *.c) \
	$(shell find $(SRCDIR)/boot/arch/$(ARCH)/machines/$(MACHINE)/stage2 -name *.c)
TRIABOOT_STAGE2_ASMFILES := \
	$(shell find $(SRCDIR)/boot -not \( -path $(SRCDIR)/boot/arch -prune \) -name *.s) \
	$(shell find $(SRCDIR)/boot/arch/$(ARCH) -not \( -path $(SRCDIR)/boot/arch/$(ARCH)/machines -prune \) -name *.s) \
	$(shell find $(SRCDIR)/boot/arch/$(ARCH)/machines/$(MACHINE)/stage2 -name *.s)
TRIABOOT_STAGE2_OBJ := $(patsubst $(SRCDIR)/boot/%, $(BUILDDIR)/boot/%, $(TRIABOOT_STAGE2_CFILES:.c=.c.o))
TRIABOOT_STAGE2_ASMOBJ := $(patsubst $(SRCDIR)/boot/%, $(BUILDDIR)/boot/%, $(TRIABOOT_STAGE2_ASMFILES:.s=.s.o))
TRIABOOT_STAGE2_DEPS := $(patsubst $(SRCDIR)/boot/%, $(BUILDDIR)/boot/%, $(TRIABOOT_STAGE2_CFILES:.c=.c.d))
TRIABOOT_STAGE2_ASMDEPS := $(patsubst $(SRCDIR)/boot/%, $(BUILDDIR)/boot/%, $(TRIABOOT_STAGE2_ASMFILES:.s=.s.d))
else ifeq ($(TARGET), m68k-mac)
TRIABOOT := $(BUILDDIR)/triaboot.bin
TRIABOOT_TRX := $(BUILDDIR)/triaboot.trx

TRIABOOT_CFILES := \
	$(shell find $(SRCDIR)/boot -not \( -path $(SRCDIR)/boot/arch -prune \) -name *.c) \
	$(shell find $(SRCDIR)/boot/arch/$(ARCH) -not \( -path $(SRCDIR)/boot/arch/$(ARCH)/machines -prune \) -name *.c) \
	$(shell find $(SRCDIR)/boot/arch/$(ARCH)/machines/$(MACHINE) -name *.c)
TRIABOOT_ASMFILES := \
	$(shell find $(SRCDIR)/boot -not \( -path $(SRCDIR)/boot/arch -prune \) -name *.s) \
	$(shell find $(SRCDIR)/boot/arch/$(ARCH) -not \( -path $(SRCDIR)/boot/arch/$(ARCH)/machines -prune \) -name *.s) \
	$(shell find $(SRCDIR)/boot/arch/$(ARCH)/machines/$(MACHINE) -name *.s)
TRIABOOT_OBJ := $(patsubst $(SRCDIR)/boot/%, $(BUILDDIR)/boot/%, $(TRIABOOT_CFILES:.c=.c.o))
TRIABOOT_ASMOBJ := $(patsubst $(SRCDIR)/boot/%, $(BUILDDIR)/boot/%, $(TRIABOOT_ASMFILES:.S=.S.o))))
TRIABOOT_DEPS := $(patsubst $(SRCDIR)/boot/%, $(BUILDDIR)/boot/%, $(TRIABOOT_CFILES:.c=.c.d))
TRIABOOT_ASMDEPS := $(patsubst $(SRCDIR)/boot/%, $(BUILDDIR)/boot/%, $(TRIABOOT_ASMFILES:.S=.S.d))))
else
$(error Invalid target: $(TARGET))
endif

KERNEL_CFILES := \
	$(shell find $(SRCDIR)/kernel -not \( -path $(SRCDIR)/kernel/arch -prune \) -name *.c) \
	$(shell find $(SRCDIR)/kernel/arch/$(ARCH) -not \( -path $(SRCDIR)/kernel/arch/$(ARCH)/machines -prune \) -name *.c) \
	$(shell find $(SRCDIR)/kernel/arch/$(ARCH)/machines/$(MACHINE) -name *.c)
KERNEL_ASMFILES := \
	$(shell find $(SRCDIR)/kernel -not \( -path $(SRCDIR)/kernel/arch -prune \) -name *.s) \
	$(shell find $(SRCDIR)/kernel/arch/$(ARCH) -not \( -path $(SRCDIR)/kernel/arch/$(ARCH)/machines -prune \) -name *.s) \
	$(shell find $(SRCDIR)/kernel/arch/$(ARCH)/machines/$(MACHINE) -name *.s)
KERNEL_OBJ := $(patsubst $(SRCDIR)/kernel/%, $(BUILDDIR)/kernel/%, $(KERNEL_CFILES:.c=.c.o))
KERNEL_ASMOBJ := $(patsubst $(SRCDIR)/kernel/%, $(BUILDDIR)/kernel/%, $(KERNEL_ASMFILES:.s=.s.o))
KERNEL_DEPS := $(patsubst $(SRCDIR)/kernel/%, $(BUILDDIR)/kernel/%, $(KERNEL_CFILES:.c=.c.d))
KERNEL_ASMDEPS := $(patsubst $(SRCDIR)/kernel/%, $(BUILDDIR)/kernel/%, $(KERNEL_ASMFILES:.s=.s.d))

LIB_CFILES := \
	$(shell find $(SRCDIR)/lib -not \( -path $(SRCDIR)/lib/arch -prune \) -name *.c) \
	$(shell find $(SRCDIR)/lib/arch/$(ARCH) -not \( -path $(SRCDIR)/lib/arch/$(ARCH)/machines -prune \) -name *.c) \
	$(shell find $(SRCDIR)/lib/arch/$(ARCH)/machines/$(MACHINE) -name *.c)
LIB_ASMFILES := \
	$(shell find $(SRCDIR)/lib -not \( -path $(SRCDIR)/lib/arch -prune \) -name *.s) \
	$(shell find $(SRCDIR)/lib/arch/$(ARCH) -not \( -path $(SRCDIR)/lib/arch/$(ARCH)/machines -prune \) -name *.s) \
	$(shell find $(SRCDIR)/lib/arch/$(ARCH)/machines/$(MACHINE) -name *.s)
LIB_OBJ := $(patsubst $(SRCDIR)/lib/%, $(BUILDDIR)/lib/%, $(LIB_CFILES:.c=.c.o))
LIB_ASMOBJ := $(patsubst $(SRCDIR)/lib/%, $(BUILDDIR)/lib/%, $(LIB_ASMFILES:.s=.s.o))
LIB_DEPS := $(patsubst $(SRCDIR)/lib/%, $(BUILDDIR)/lib/%, $(LIB_CFILES:.c=.c.d))
LIB_ASMDEPS := $(patsubst $(SRCDIR)/lib/%, $(BUILDDIR)/lib/%, $(LIB_ASMFILES:.s=.s.d))

CC_RUNTIME_CFILES := $(shell find $(EXTERNALDIR)/cc-runtime -name *.c)
CC_RUNTIME_OBJ := $(patsubst $(EXTERNALDIR)/%, $(BUILDDIR)/%, $(CC_RUNTIME_CFILES:.c=.c.o))
CC_RUNTIME_DEPS := $(patsubst $(EXTERNALDIR)/%, $(BUILDDIR)/%, $(CC_RUNTIME_CFILES:.c=.c.d))

all: triaboot kernel

triaboot: $(TRIABOOT) $(TRIAOS_IMG)
kernel: $(KERNEL)

ifeq ($(TARGET), i386-pc)
$(TRIAOS_IMG): $(TRIABOOT) $(KERNEL)
	@$(MKCWD)
	@$(MKDIR) -p $(MOUNTDIR)
	@echo -e "[DD]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)dd if=/dev/zero bs=1M count=0 seek=64 of=$(TRIAOS_IMG) $(DD_STATUS)
	@echo -e "[PARTED]\t$(@:$(BUILDDIR)/%=%)"
	$(Q)parted -s $(TRIAOS_IMG) mklabel msdos
	$(Q)parted -s $(TRIAOS_IMG) mkpart primary 2048s 6143s
	$(Q)parted -s $(TRIAOS_IMG) mkpart primary 6144s 131038s
	@echo -e "[LOSETUP]\t$(@:$(BUILDDIR)/%=%)"
	$(Q)sudo losetup -Pf --show $(TRIAOS_IMG) > $(LOOPBACK)
	@echo -e "[MKFS]\t\t`cat $(LOOPBACK)`p2"
	$(Q)sudo mkfs.fat -F 32 $(MKDOSFS_V) `cat $(LOOPBACK)`p2
	@echo -e "[MOUNT]\t\t`cat $(LOOPBACK)`p2"
	$(Q)sudo mount `cat $(LOOPBACK)`p2 $(MOUNTDIR)
	$(Q)sudo mkdir $(MOUNTDIR)/TriaOS
	$(Q)sudo cp $(KERNEL) $(MOUNTDIR)/TriaOS/triakrnl.trx
	@echo -e "[UMOUNT]\t`cat $(LOOPBACK)`p2"
	$(Q)sudo umount $(MOUNTDIR)
	$(Q)sudo losetup -d `cat $(LOOPBACK)`
	$(Q)rm $(LOOPBACK)
	@echo -e "[TRIABOOT]\t$(@:$(BUILDDIR)/%=%)"
	$(Q)tools/boot/$(ARCH)/$(MACHINE)/triaboot-install $(TRIABOOT) $(TRIAOS_IMG) 2048

$(TRIABOOT): $(TRIABOOT_STAGE2) $(SRCDIR)/boot/arch/$(ARCH)/machines/$(MACHINE)/stage1.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(AS) -fbin -MD -MP -I$(BUILDDIR) $(SRCDIR)/boot/arch/$(ARCH)/machines/$(MACHINE)/stage1.s -o $@

$(TRIABOOT_STAGE2): $(TRIABOOT_STAGE2_TRX)
	@$(MKCWD)
	@echo -e "[OBJCOPY]\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(OBJCOPY) -Obinary $< $@

$(TRIABOOT_STAGE2_TRX): $(CC_RUNTIME_OBJ) $(TRIABOOT_STAGE2_OBJ) $(TRIABOOT_STAGE2_ASMOBJ) $(LIB)
	@$(MKCWD)
	@echo -e "[LD]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(LD) $(CC_RUNTIME_OBJ) $(TRIABOOT_STAGE2_OBJ) $(TRIABOOT_STAGE2_ASMOBJ) $(LDFLAGS) $(LDHARDFLAGS) -T$(SRCDIR)/boot/arch/$(ARCH)/machines/$(MACHINE)/stage2/linker.ld -o $@

-include $(TRIABOOT_STAGE2_DEPS) $(TRIABOOT_STAGE2_ASMDEPS)

$(BUILDDIR)/boot/%.c.o: $(SRCDIR)/boot/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/boot/%.s.o: $(SRCDIR)/boot/%.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(AS) $(ASHARDFLAGS) -I$(SRCDIR)/boot $< -o $@
else ifeq ($(TARGET), m68k-mac)
$(TRIAOS_IMG): $(TRIABOOT)
	@$(MKCWD)
	@echo -e "[TRUNCATE]\t$(@:$(BUILDDIR)/%=%)"
	$(Q)cp $< $@
	$(Q)truncate -s 12288 $@

$(TRIABOOT): $(TRIABOOT_TRX)
	@$(MKCWD)
	@echo -e "[OBJCOPY]\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(OBJCOPY) -Obinary $< $@

$(TRIABOOT_TRX): $(CC_RUNTIME_OBJ) $(TRIABOOT_OBJ) $(TRIABOOT_ASMOBJ) $(LIB)
	@$(MKCWD)
	@echo -e "[LD]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(LD) $(CC_RUNTIME_OBJ) $(TRIABOOT_OBJ) $(TRIABOOT_ASMOBJ) $(LDFLAGS) $(LDHARDFLAGS) -T$(SRCDIR)/boot/arch/$(ARCH)/machines/$(MACHINE)/linker.ld -o $@

-include $(TRIABOOT_DEPS) $(TRIABOOT_ASMDEPS)

$(BUILDDIR)/boot/%.c.o: $(SRCDIR)/boot/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/boot/%.S.o: $(SRCDIR)/boot/%.S
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(AS) $(ASHARDFLAGS) -I$(SRCDIR)/boot -c $< -o $@
else
$(error Invalid target: $(TARGET))
endif

$(LIB): $(LIB_OBJ) $(LIB_ASMOBJ)
	@$(MKCWD)
	@echo -e "[AR]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(AR) rcs $@ $(LIB_OBJ) $(LIB_ASMOBJ)

-include $(LIB_DEPS) $(LIB_ASMDEPS)

$(BUILDDIR)/lib/%.c.o: $(SRCDIR)/lib/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/lib/%.s.o: $(SRCDIR)/lib/%.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(AS) $(ASHARDFLAGS) -I$(SRCDIR)/lib $< -o $@

$(KERNEL): $(CC_RUNTIME_OBJ) $(KERNEL_OBJ) $(KERNEL_ASMOBJ) $(LIB)
	@$(MKCWD)
	@echo -e "[LD]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(LD) $(CC_RUNTIME_OBJ) $(KERNEL_OBJ) $(KERNEL_ASMOBJ) $(LDFLAGS) $(LDHARDFLAGS) -T$(SRCDIR)/kernel/arch/$(ARCH)/linker.ld -o $@

-include $(KERNEL_DEPS) $(KERNEL_ASMDEPS)

$(BUILDDIR)/kernel/%.c.o: $(SRCDIR)/kernel/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/kernel/%.s.o: $(SRCDIR)/kernel/%.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(AS) $(ASHARDFLAGS) -I$(SRCDIR)/kernel $< -o $@

-include $(CC_RUNTIME_DEPS)

$(BUILDDIR)/cc-runtime/%.c.o: $(EXTERNALDIR)/cc-runtime/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(EXTERNALDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

run: all
	@echo -e "[QEMU]\t\t$(TRIAOS_IMG:$(BUILDDIR)/%=%)"
	$(Q)$(QEMU) -m $(QEMUMEMSIZE) $(QEMUFLAGS) $(QEMUHARDFLAGS) -hda $(TRIAOS_IMG)

run-multiboot: kernel
	@echo -e "[QEMU]\t\t$(KERNEL:$(BUILDDIR)/%=%)"
	$(Q)$(QEMU) -m $(QEMUMEMSIZE) $(QEMUFLAGS) $(QEMUHARDFLAGS) -kernel $(KERNEL)

clean:
	$(Q)$(RM)r $(BUILDDIR)
