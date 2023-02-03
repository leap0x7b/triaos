.SUFFIXES:
.DELETE_ON_ERROR:
.DEFAULT_GOAL := all

SHELL = bash
MKDIR = mkdir -p
MKCWD = $(MKDIR) $(@D)

SRCDIR = $(CURDIR)/src
INCLUDEDIR = $(CURDIR)/include
BUILDDIR = $(CURDIR)/build
EXTERNALDIR = $(CURDIR)/external

ISO = $(BUILDDIR)/triaos.iso

CC = clang -target i686-unknown-elf
AS = nasm
LD = ld.lld
OBJCOPY = llvm-objcopy
QEMU = qemu-system-i386

CFLAGS ?= -Og -gdwarf -pipe
ASFLAGS ?=
LDFLAGS ?=
QEMUFLAGS ?=
QEMUMEMSIZE ?= 512M

CHARDFLAGS := \
	-I$(INCLUDEDIR) \
	-Wno-sequence-point \
	-nostdlib -std=gnu11 \
	-ffreestanding -fno-pic \
	-fno-stack-protector \
	-mcmodel=kernel -MMD -MP \
	-mno-80387 -mno-mmx -mno-3dnow \
	-mno-sse -mno-sse2 -msoft-float
ASHARDFLAGS := -felf32 -g -Fdwarf -MD -MP
LDHARDFLAGS := \
	-nostdlib -static \
	-L$(BUILDDIR) -ltria \
	-zmax-page-size=0x1000 \
	--no-dynamic-linker

TRIABOOT := $(BUILDDIR)/triaboot.bin
TRIABOOT_STAGE1 := $(BUILDDIR)/boot/stage1.bin
TRIABOOT_STAGE2 := $(BUILDDIR)/boot/stage2.bin
TRIABOOT_STAGE2_TRX := $(BUILDDIR)/boot/stage2.trx
LIB := $(BUILDDIR)/libtria.a
KERNEL := $(BUILDDIR)/kernel.trx

TRIABOOT_CFILES := $(shell find $(SRCDIR)/boot/stage2 -name *.c)
TRIABOOT_ASMFILES := $(shell find $(SRCDIR)/boot/stage2 -name *.s)
TRIABOOT_OBJ := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(TRIABOOT_CFILES:.c=.o))
TRIABOOT_ASMOBJ := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(TRIABOOT_ASMFILES:.s=.s.o))
TRIABOOT_DEPS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(TRIABOOT_CFILES:.c=.d))
TRIABOOT_ASMDEPS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(TRIABOOT_ASMFILES:.s=.s.d))

KERNEL_CFILES := $(shell find $(SRCDIR)/kernel -name *.c)
KERNEL_ASMFILES := $(shell find $(SRCDIR)/kernel -name *.s)
KERNEL_OBJ := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(KERNEL_CFILES:.c=.o))
KERNEL_ASMOBJ := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(KERNEL_ASMFILES:.s=.s.o))
KERNEL_DEPS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(KERNEL_CFILES:.c=.d))
KERNEL_ASMDEPS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(KERNEL_ASMFILES:.s=.s.d))

LIB_CFILES := $(shell find $(SRCDIR)/lib -name *.c)
LIB_ASMFILES := $(shell find $(SRCDIR)/lib -name *.s)
LIB_OBJ := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(LIB_CFILES:.c=.o))
LIB_ASMOBJ := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(LIB_ASMFILES:.s=.s.o))
LIB_DEPS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(LIB_CFILES:.c=.d))
LIB_ASMDEPS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(LIB_ASMFILES:.s=.s.d))

all: triaboot kernel

triaboot: $(TRIABOOT)
kernel: $(KERNEL)

$(TRIABOOT): $(TRIABOOT_STAGE1) $(TRIABOOT_STAGE2)
	@$(MKCWD)
	@echo -e "[CAT]\t\t$(TRIABOOT_STAGE1:$(BUILDDIR)/%=%) $(TRIABOOT_STAGE2:$(BUILDDIR)/%=%) > $(@:$(BUILDDIR)/%=%)"
	@cat $(TRIABOOT_STAGE1) $(TRIABOOT_STAGE2) > $@

$(TRIABOOT_STAGE1): $(SRCDIR)/boot/stage1/main.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	@$(AS) -fbin -MD -MP $< -o $@

$(TRIABOOT_STAGE2): $(TRIABOOT_STAGE2_TRX)
	@$(MKCWD)
	@echo -e "[OBJCOPY]\t$(@:$(BUILDDIR)/%=%)"
	@$(OBJCOPY) -Obinary $< $@

$(TRIABOOT_STAGE2_TRX): $(TRIABOOT_OBJ) $(TRIABOOT_ASMOBJ) $(LIB)
	@$(MKCWD)
	@echo -e "[LD]\t\t$(@:$(BUILDDIR)/%=%)"
	@$(LD) $(TRIABOOT_OBJ) $(TRIABOOT_ASMOBJ) $(LDFLAGS) $(LDHARDFLAGS) -T$(SRCDIR)/boot/stage2/linker.ld -o $@

-include $(TRIABOOT_DEPS) $(TRIABOOT_ASMDEPS)

$(BUILDDIR)/boot/stage2/%.o: $(SRCDIR)/boot/stage2/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	@$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/boot/stage2/%.s.o: $(SRCDIR)/boot/stage2/%.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	@$(AS) $(ASHARDFLAGS) -I$(SRCDIR)/boot/stage2 $< -o $@

$(LIB): $(LIB_OBJ) $(LIB_ASMOBJ)
	@$(MKCWD)
	@echo -e "[AR]\t\t$(@:$(BUILDDIR)/%=%)"
	@$(AR) rcs $@ $(LIB_OBJ) $(LIB_ASMOBJ)

-include $(LIB_DEPS) $(LIB_ASMDEPS)

$(BUILDDIR)/lib/%.o: $(SRCDIR)/lib/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	@$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/lib/%.s.o: $(SRCDIR)/lib/%.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	@$(AS) $(ASHARDFLAGS) -I$(SRCDIR)/lib $< -o $@

$(KERNEL): $(KERNEL_OBJ) $(KERNEL_ASMOBJ) $(LIB)
	@$(MKCWD)
	@echo -e "[LD]\t\t$(@:$(BUILDDIR)/%=%)"
	@$(LD) $(KERNEL_OBJ) $(KERNEL_ASMOBJ) $(LDFLAGS) $(LDHARDFLAGS) -T$(SRCDIR)/kernel/linker.ld -o $@

-include $(KERNEL_DEPS) $(KERNEL_ASMDEPS)

$(BUILDDIR)/kernel/%.o: $(SRCDIR)/kernel/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	@$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/kernel/%.s.o: $(SRCDIR)/kernel/%.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	@$(AS) $(ASHARDFLAGS) -I$(SRCDIR)/kernel $< -o $@

run: all
	@echo -e "[QEMU]\t\t$(TRIABOOT:$(BUILDDIR)/%=%)"
	@$(QEMU) -m $(QEMUMEMSIZE) -no-reboot -no-shutdown $(QEMUFLAGS) -hda $(TRIABOOT) -debugcon stdio

clean:
	@$(RM)r $(BUILDDIR)
