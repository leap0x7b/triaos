.SUFFIXES:
.DELETE_ON_ERROR:
.DEFAULT_GOAL := all

ifneq ($(VERBOSE), 1)
	Q := @
	MKDOSFS_V := -v
	DD_STATUS := status=none
endif

SHELL = bash
MKDIR = mkdir -p
MKCWD = $(MKDIR) $(@D)

TARGET ?= i386-pc
ifeq (,$(filter $(TARGET),i386-pc m68k-mac))
$(error Invalid target: $(TARGET))
endif

SRCDIR = $(CURDIR)/src
INCLUDEDIR = $(CURDIR)/include
BUILDDIR = $(CURDIR)/build/$(TARGET)
EXTERNALDIR = $(CURDIR)/external

ISO = $(BUILDDIR)/triaos.iso

ifeq ($(TARGET), i386-pc)
CC = clang -target i386-pc-elf
AS = nasm
LD = ld.lld
OBJCOPY = llvm-objcopy
QEMU = qemu-system-i386

CFLAGS ?= -Og -gdwarf
ASFLAGS ?= -g -Fdwarf
LDFLAGS ?=
QEMUFLAGS ?= -no-reboot -no-shutdown
QEMUMEMSIZE ?= 512M

CHARDFLAGS := \
	-I$(INCLUDEDIR) \
	-nostdlib -std=c99 \
	-pedantic -ffreestanding \
	-mcmodel=kernel -MMD -MP \
	-mno-80387 -mno-mmx -mno-3dnow \
	-mno-sse -mno-sse2 -msoft-float
ASHARDFLAGS := -felf32 -MD -MP
LDHARDFLAGS := \
	-nostdlib -static \
	-L$(BUILDDIR) -ltria \
	-L$(SRCDIR) -lcompiler_rt \
	-zmax-page-size=0x1000 \
	--no-dynamic-linker
QEMUHARDFLAGS := -machine q35,accel=kvm:whpx:hvf -debugcon stdio
else ifeq ($(TARGET), m68k-mac)
CC = m68k-none-elf-gcc
AS = m68k-none-elf-as
LD = m68k-none-elf-ld
OBJCOPY = m68k-none-elf-objcopy
QEMU = qemu-system-m68k

CFLAGS ?= -O2 -gdwarf
ASFLAGS ?= -Fdwarf
LDFLAGS ?=
QEMUFLAGS ?= -no-reboot -no-shutdown
QEMUMEMSIZE ?= 512M

CHARDFLAGS := \
	-I$(INCLUDEDIR) \
	-nostdlib -std=c99 \
	-pedantic -ffreestanding \
	-march=68000 -MMD -MP
ASHARDFLAGS := $(CFLAGS)
LDHARDFLAGS := \
	-nostdlib -static \
	-L$(BUILDDIR) -ltria
QEMUHARDFLAGS := -machine q800 -bios ../mac/Quadra800.ROM
else
$(error Invalid target: $(TARGET))
endif

TRIABOOT_IMG := $(BUILDDIR)/triaboot.img
TRIABOOT_STAGE1 := $(BUILDDIR)/triaboot.s1
TRIABOOT_STAGE2 := $(BUILDDIR)/triaboot.s2
TRIABOOT_STAGE2_TRX := $(BUILDDIR)/triaboot.s2.trx
TRIABOOT_STAGE3 := $(BUILDDIR)/triaboot.s3
TRIABOOT_STAGE3_TRX := $(BUILDDIR)/triaboot.s3.trx
LIB := $(BUILDDIR)/libtria.a
KERNEL := $(BUILDDIR)/triakrnl.trx

ifeq ($(TARGET), i386-pc)
TRIABOOT_STAGE2_CFILES := $(shell find $(SRCDIR)/boot/$(TARGET)/stage2 -name *.c)
TRIABOOT_STAGE2_ASMFILES := $(shell find $(SRCDIR)/boot/$(TARGET)/stage2 -name *.s)
TRIABOOT_STAGE2_OBJ := $(patsubst $(SRCDIR)/boot/$(TARGET)/%, $(BUILDDIR)/boot/%, $(TRIABOOT_STAGE2_CFILES:.c=.c.o))
TRIABOOT_STAGE2_ASMOBJ := $(patsubst $(SRCDIR)/boot/$(TARGET)/%, $(BUILDDIR)/boot/%, $(TRIABOOT_STAGE2_ASMFILES:.s=.s.o))
TRIABOOT_STAGE2_DEPS := $(patsubst $(SRCDIR)/boot/$(TARGET)/%, $(BUILDDIR)/boot/%, $(TRIABOOT_STAGE2_CFILES:.c=.c.d))
TRIABOOT_STAGE2_ASMDEPS := $(patsubst $(SRCDIR)/boot/$(TARGET)/%, $(BUILDDIR)/boot/%, $(TRIABOOT_STAGE2_ASMFILES:.s=.s.d))

TRIABOOT_STAGE3_CFILES := $(shell find $(SRCDIR)/boot/$(TARGET)/stage3 -name *.c)
TRIABOOT_STAGE3_ASMFILES := $(shell find $(SRCDIR)/boot/$(TARGET)/stage3 -name *.s)
TRIABOOT_STAGE3_OBJ := $(patsubst $(SRCDIR)/boot/$(TARGET)/%, $(BUILDDIR)/boot/%, $(TRIABOOT_STAGE3_CFILES:.c=.c.o))
TRIABOOT_STAGE3_ASMOBJ := $(patsubst $(SRCDIR)/boot/$(TARGET)/%, $(BUILDDIR)/boot/%, $(TRIABOOT_STAGE3_ASMFILES:.s=.s.o))
TRIABOOT_STAGE3_DEPS := $(patsubst $(SRCDIR)/boot/$(TARGET)/%, $(BUILDDIR)/boot/%, $(TRIABOOT_STAGE3_CFILES:.c=.c.d))
TRIABOOT_STAGE3_ASMDEPS := $(patsubst $(SRCDIR)/boot/$(TARGET)/%, $(BUILDDIR)/boot/%, $(TRIABOOT_STAGE3_ASMFILES:.s=.s.d))
else ifeq ($(TARGET), m68k-mac)
TRIABOOT_CFILES := $(shell find $(SRCDIR)/boot/$(TARGET) -name *.c)
TRIABOOT_ASMFILES := $(shell find $(SRCDIR)/boot/$(TARGET) -name *.S)
TRIABOOT_OBJ := $(patsubst $(SRCDIR)/boot/$(TARGET)/%, $(BUILDDIR)/boot/%, $(TRIABOOT_CFILES:.c=.c.o))
TRIABOOT_ASMOBJ := $(patsubst $(SRCDIR)/boot/$(TARGET)/%, $(BUILDDIR)/boot/%, $(TRIABOOT_ASMFILES:.S=.S.o))
TRIABOOT_DEPS := $(patsubst $(SRCDIR)/boot/$(TARGET)/%, $(BUILDDIR)/boot/%, $(TRIABOOT_CFILES:.c=.c.d))
TRIABOOT_ASMDEPS := $(patsubst $(SRCDIR)/boot/$(TARGET)/%, $(BUILDDIR)/boot/%, $(TRIABOOT_ASMFILES:.S=.S.d))
else
$(error Invalid target: $(TARGET))
endif

KERNEL_CFILES := $(shell find $(SRCDIR)/kernel -name *.c)
KERNEL_ASMFILES := $(shell find $(SRCDIR)/kernel -name *.s)
KERNEL_OBJ := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(KERNEL_CFILES:.c=.c.o))
KERNEL_ASMOBJ := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(KERNEL_ASMFILES:.s=.s.o))
KERNEL_DEPS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(KERNEL_CFILES:.c=.c.d))
KERNEL_ASMDEPS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(KERNEL_ASMFILES:.s=.s.d))

LIB_CFILES := $(shell find $(SRCDIR)/lib -not \( -path $(SRCDIR)/lib/machines -prune \) -name *.c) $(shell find $(SRCDIR)/lib/machines/$(TARGET) -name *.c)
LIB_ASMFILES := $(shell find $(SRCDIR)/lib -not \( -path $(SRCDIR)/lib/machines -prune \) -name *.s) $(shell find $(SRCDIR)/lib/machines/$(TARGET) -name *.s)
LIB_OBJ := $(patsubst $(BUILDDIR)/lib/machines/$(TARGET)/%, $(BUILDDIR)/lib/%, $(patsubst $(SRCDIR)/lib/%, $(BUILDDIR)/lib/%, $(LIB_CFILES:.c=.c.o)))
LIB_ASMOBJ := $(patsubst $(BUILDDIR)/lib/machines/$(TARGET)/%, $(BUILDDIR)/lib/%, $(patsubst $(SRCDIR)/lib/%, $(BUILDDIR)/lib/%, $(LIB_ASMFILES:.s=.s.o)))
LIB_DEPS := $(patsubst $(BUILDDIR)/lib/machines/$(TARGET)/%, $(BUILDDIR)/lib/%, $(patsubst $(SRCDIR)/lib/%, $(BUILDDIR)/lib/%, $(LIB_CFILES:.c=.c.d)))
LIB_ASMDEPS := $(patsubst $(BUILDDIR)/lib/machines/$(TARGET)/%, $(BUILDDIR)/lib/%, $(patsubst $(SRCDIR)/lib/%, $(BUILDDIR)/lib/%, $(LIB_ASMFILES:.s=.s.d)))

all: triaboot kernel

triaboot: $(TRIABOOT) $(TRIABOOT_IMG)
kernel: $(KERNEL)

ifeq ($(TARGET), i386-pc)
$(TRIABOOT_IMG): $(TRIABOOT_STAGE1) $(TRIABOOT_STAGE2) $(TRIABOOT_STAGE3) $(KERNEL)
	@$(MKCWD)
	@echo -e "[DD]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)dd if=/dev/zero of=$(TRIABOOT_IMG) bs=512 count=8192 $(DD_STATUS)
	$(Q)dd if=$(TRIABOOT_STAGE1) of=$(TRIABOOT_IMG) bs=512 count=1 seek=0 conv=notrunc $(DD_STATUS)
	$(Q)dd if=$(TRIABOOT_STAGE2) of=$(TRIABOOT_IMG) bs=512 count=16 seek=8 conv=notrunc $(DD_STATUS)
	@echo -e "[MTOOLS]\t$(@:$(BUILDDIR)/%=%)"
	$(Q)mformat -i $(TRIABOOT_IMG) -c 1 -k -R 48 ::
	$(Q)mcopy -i $(TRIABOOT_IMG) $(TRIABOOT_STAGE3) ::/triaboot.s3
	$(Q)mattrib -i $(TRIABOOT_IMG) +r +s +h ::/triaboot.s3
	$(Q)mmd -i $(TRIABOOT_IMG) ::/TriaOS
	$(Q)mcopy -i $(TRIABOOT_IMG) $(KERNEL) ::/TriaOS

$(TRIABOOT_STAGE1): $(SRCDIR)/boot/$(TARGET)/stage1.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(AS) -fbin -MD -MP -I$(SRCDIR)/boot $< -o $@

$(TRIABOOT_STAGE2): $(TRIABOOT_STAGE2_TRX)
	@$(MKCWD)
	@echo -e "[OBJCOPY]\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(OBJCOPY) -Obinary $< $@

$(TRIABOOT_STAGE2_TRX): $(TRIABOOT_STAGE2_OBJ) $(TRIABOOT_STAGE2_ASMOBJ) $(LIB)
	@$(MKCWD)
	@echo -e "[LD]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(LD) $(TRIABOOT_STAGE2_OBJ) $(TRIABOOT_STAGE2_ASMOBJ) $(LDFLAGS) $(LDHARDFLAGS) -T$(SRCDIR)/boot/$(TARGET)/stage2/linker.ld -o $@

-include $(TRIABOOT_STAGE2_DEPS) $(TRIABOOT_STAGE2_ASMDEPS)

$(BUILDDIR)/boot/stage2/%.c.o: $(SRCDIR)/boot/$(TARGET)/stage2/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/boot/stage2/%.s.o: $(SRCDIR)/boot/$(TARGET)/stage2/%.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(AS) $(ASHARDFLAGS) -I$(SRCDIR)/boot $< -o $@

$(TRIABOOT_STAGE3): $(TRIABOOT_STAGE3_TRX)
	@$(MKCWD)
	@echo -e "[OBJCOPY]\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(OBJCOPY) -Obinary $< $@

$(TRIABOOT_STAGE3_TRX): $(TRIABOOT_STAGE3_OBJ) $(TRIABOOT_STAGE3_ASMOBJ) $(LIB)
	@$(MKCWD)
	@echo -e "[LD]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(LD) $(TRIABOOT_STAGE3_OBJ) $(TRIABOOT_STAGE3_ASMOBJ) $(LDFLAGS) $(LDHARDFLAGS) -T$(SRCDIR)/boot/$(TARGET)/stage3/linker.ld -o $@

-include $(TRIABOOT_STAGE3_DEPS) $(TRIABOOT_STAGE3_ASMDEPS)

$(BUILDDIR)/boot/stage3/%.c.o: $(SRCDIR)/boot/$(TARGET)/stage3/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/boot/stage3/%.s.o: $(SRCDIR)/boot/$(TARGET)/stage3/%.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(AS) $(ASHARDFLAGS) -I$(SRCDIR)/boot $< -o $@
else ifeq ($(TARGET), m68k-mac)
$(TRIABOOT_IMG): $(TRIABOOT)
	@$(MKCWD)
	@echo -e "[TRUNCATE]\t$(@:$(BUILDDIR)/%=%)"
	$(Q)cp $< $@
	$(Q)truncate -s 1440K $@

$(TRIABOOT): $(TRIABOOT_TRX)
	@$(MKCWD)
	@echo -e "[OBJCOPY]\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(OBJCOPY) -Obinary $< $@

$(TRIABOOT_TRX): $(TRIABOOT_OBJ) $(TRIABOOT_ASMOBJ) $(LIB)
	@$(MKCWD)
	@echo -e "[LD]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(LD) $(TRIABOOT_OBJ) $(TRIABOOT_ASMOBJ) $(LDFLAGS) $(LDHARDFLAGS) -T$(SRCDIR)/boot/$(TARGET)/linker.ld -o $@

-include $(TRIABOOT_DEPS) $(TRIABOOT_ASMDEPS)

$(BUILDDIR)/boot/%.c.o: $(SRCDIR)/boot/$(TARGET)/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/boot/%.S.o: $(SRCDIR)/boot/$(TARGET)/%.S
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

$(BUILDDIR)/lib/%.c.o: $(SRCDIR)/lib/machines/$(TARGET)/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/lib/%.s.o: $(SRCDIR)/lib/machines/$(TARGET)/%.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(AS) $(ASHARDFLAGS) -I$(SRCDIR)/lib $< -o $@

$(KERNEL): $(KERNEL_OBJ) $(KERNEL_ASMOBJ) $(LIB)
	@$(MKCWD)
	@echo -e "[LD]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(LD) $(KERNEL_OBJ) $(KERNEL_ASMOBJ) $(LDFLAGS) $(LDHARDFLAGS) -T$(SRCDIR)/kernel/linker.ld -o $@

-include $(KERNEL_DEPS) $(KERNEL_ASMDEPS)

$(BUILDDIR)/kernel/%.c.o: $(SRCDIR)/kernel/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/kernel/%.s.o: $(SRCDIR)/kernel/%.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(AS) $(ASHARDFLAGS) -I$(SRCDIR)/kernel $< -o $@

run: all
	@echo -e "[QEMU]\t\t$(TRIABOOT_IMG:$(BUILDDIR)/%=%)"
	$(Q)$(QEMU) -m $(QEMUMEMSIZE) $(QEMUFLAGS) $(QEMUHARDFLAGS) -hda $(TRIABOOT_IMG)

clean:
	$(Q)$(RM)r $(BUILDDIR)
