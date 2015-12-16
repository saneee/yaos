# Default make target
.PHONY: all
all: yaos.img 

X64 = 1

BITS = 64
XFLAGS = -DDEBUG -std=gnu11 -m64 -DX64 -mcmodel=kernel -mtls-direct-seg-refs -mno-red-zone
LDFLAGS = -m elf_x86_64 -nodefaultlibs  

FSGSBASE=$(shell cat /proc/cpuinfo|grep fsgsbase)
ifneq ($(strip $(FSGSBASE)),)
   XFLAGS+= -D__FSGSBASE__
endif


OUT = out

HOST_CC ?= gcc

# specify OPT to enable optimizations. improves performance, but may make
# debugging more difficult
OPT ?= -O2


ifneq ("$(MEMFS)","")
# build filesystem image in to kernel and use memory-ide-device
# instead of mounting the filesystem on ide1
OBJS := $(filter-out ide.o,$(OBJS)) memide.o
FSIMAGE := fs.img
endif
ARCHOBJ_DIR =.archobj
DOBJ_DIR =.dobj
KOBJ_DIR = .kobj
OBJS := $(addprefix $(KOBJ_DIR)/,$(OBJS))
AOBJS :=  entry64.o pm64.o main.o uart.o   vgaoutput.o  cpu.o \
vectors.o trapasm64.o multiboot.o mmu.o pgtable.o phymem.o apic.o acpi.o \
trap.o ioapic.o time.o pci.o irq.o lib/memset_64.o lib/memmove_64.o hpet.o\
lib/memcpy_64.o alternative.o lib/copy_page_64.o lib/clear_page_64.o \
lib/iomap_copy_64.o 
KOBJS :=  yaos.o printk.o  kheap.o vm.o  module.o yaoscall.o main.o \
smp.o yaos_page.o sched.o kthread.o timer.o debug.o tasklet.o worker.o dummy.o
DOBJS := 

ifneq ($(MAKECMDGOALS),clean)
include $(shell test -d $(ARCHOBJ_DIR) && find $(ARCHOBJ_DIR) -name '*.d')
include $(shell test -d $(DOBJ_DIR) && find $(DOBJ_DIR) -name '*.d')
include $(shell test -d $(KOBJ_DIR) && find $(KOBJ_DIR) -name '*.d')

endif

MODULEC_SOURCES = $(shell find module -name "*.c")
MODULEC_OBJECTS = $(patsubst %.c, %.o, $(MODULEC_SOURCES))

LIBS_SOURCES = $(shell find libs -name "*.c")
LIBS_OBJECTS = $(patsubst %.c, %.o, $(LIBS_SOURCES))


OBJS := $(addprefix $(ARCHOBJ_DIR)/,$(AOBJS)) \
          $(addprefix $(KOBJ_DIR)/,$(KOBJS)) $(addprefix $(DOBJ_DIR)/,$(DOBJS)) 
OBJS += $(MODULEC_OBJECTS) $(LIBS_OBJECTS)
# Cross-compiling (e.g., on Mac OS X)
CROSS_COMPILE ?=

# If the makefile can't find QEMU, specify its path here
QEMU ?= qemu-system-x86_64

CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)gas
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

# cc-option
# Usage: OP_CFLAGS+=$(call cc-option, -falign-functions=0, -malign-functions=0)
cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
	> /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -Wall -Werror
CFLAGS += -g -Wall -MD -D__KERNEL__ -fno-omit-frame-pointer
CFLAGS += -ffreestanding -fno-common -nostdlib -I arch/x86_64/include -Iinclude -I bsd/sys -gdwarf-2 $(XFLAGS) $(OPT)
CFLAGS += $(call cc-option, -fno-stack-protector, "")
CFLAGS += $(call cc-option, -fno-stack-protector-all, "")
ASFLAGS = -gdwarf-2 -Wa,-divide -D__ASSEMBLY__ -Iinclude -I arch/x86_64/include $(XFLAGS)

MODULEC_FLAGS=$(CFLAGS) -D_MODULE
acpi-defines = -DACPI_MACHINE_WIDTH=64 -DACPI_USE_LOCAL_CACHE

acpi-source := $(shell find extern/x64/acpica/components -type f -name '*.c')
acpi = $(patsubst %.c, %.o, $(acpi-source))

ACPIOBJ=$(acpi)
$(acpi:%=$(out)/%): CFLAGS += -fno-strict-aliasing -Wno-strict-aliasing -iextern/x64/acpica/include/

out/acpi.o:$(acpi)
        

yaos.img: $(OUT)/bootblock $(OUT)/kernel.elf fs.img
	dd if=/dev/zero of=yaos.img count=10000
	dd if=$(OUT)/bootblock of=yaos.img conv=notrunc
	dd if=$(OUT)/kernel.elf of=yaos.img seek=1 conv=notrunc

yaosmemfs.img: $(OUT)/bootblock $(OUT)/kernelmemfs.elf
	dd if=/dev/zero of=yaosmemfs.img count=10000
	dd if=$(OUT)/bootblock of=yaosmemfs.img conv=notrunc
	dd if=$(OUT)/kernelmemfs.elf of=yaosmemfs.img seek=1 conv=notrunc

# kernel object files
$(KOBJ_DIR)/%.o: kernel/%.c
	@mkdir -p $(KOBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KOBJ_DIR)/%.o: kernel/%.S
	@mkdir -p $(KOBJ_DIR)
	$(CC) $(ASFLAGS) -c -o $@ $<

$(ARCHOBJ_DIR)/%.o: arch/x86_64/%.c
	 @mkdir -p $(ARCHOBJ_DIR)
	 $(CC) $(CFLAGS) -c -o $@ $<

$(ARCHOBJ_DIR)/%.o: arch/x86_64/%.S
	@mkdir -p $(ARCHOBJ_DIR)
	@mkdir -p $(ARCHOBJ_DIR)/lib
	 $(CC) $(ASFLAGS) -c -o $@ $<

$(DOBJ_DIR)/%.o: drivers/%.c
	@mkdir -p $(DOBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(DOBJ_DIR)/%.o: drivers/%.S
	@mkdir -p $(DOBJ_DIR)
	$(CC) $(ASFLAGS) -c -o $@ $<

.c.o:
	$(CC) $(MODULEC_FLAGS) -c $< -o $@



UOBJ_DIR = .uobj
# userspace object files

# FIXME: -O1 and -O2 result in larger user programs, which can not fit mkfs
CFLAGS_user = $(filter-out -O1 -O2,$(CFLAGS)) -Os

$(UOBJ_DIR)/%.o: user/%.c
	@mkdir -p $(UOBJ_DIR)
	$(CC) $(CFLAGS_user) -c -o $@ $<

$(UOBJ_DIR)/%.o: ulib/%.c
	@mkdir -p $(UOBJ_DIR)
	$(CC) $(CFLAGS_user) -c -o $@ $<

$(UOBJ_DIR)/%.o: ulib/%.S
	@mkdir -p $(UOBJ_DIR)
	$(CC) $(ASFLAGS) -c -o $@ $<

# bootblock is optimized for space
$(OUT)/bootblock: bootloader/bootasm.S bootloader/bootmain.c
	@mkdir -p $(OUT)
	$(CC) -fno-builtin -fno-pic -m32 -nostdinc -Iinclude -Os -o $(OUT)/bootmain.o -c bootloader/bootmain.c
	$(CC) -fno-builtin -fno-pic -m32 -nostdinc -Iarch/x86_64/include -Iinclude -o $(OUT)/bootasm.o -c bootloader/bootasm.S
	$(LD) -m elf_i386 -nodefaultlibs --omagic -e start -Ttext 0x7C00 \
                -o $(OUT)/bootblock.o $(OUT)/bootasm.o $(OUT)/bootmain.o
	$(OBJDUMP) -S $(OUT)/bootblock.o > $(OUT)/bootblock.asm
	$(OBJCOPY) -S -O binary -j .text $(OUT)/bootblock.o $(OUT)/bootblock
	tools/sign.pl $(OUT)/bootblock

$(OUT)/entryother: arch/x86_64/entryother.S
	@mkdir -p $(OUT)
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I. -o $(OUT)/entryother.o -c arch/x86_64/entryother.S
	$(LD) $(LDFLAGS) --omagic -e start -Ttext 0x7000 -o $(OUT)/bootblockother.o $(OUT)/entryother.o
	$(OBJCOPY) -S -O binary -j .text $(OUT)/bootblockother.o $(OUT)/entryother
	$(OBJDUMP) -S $(OUT)/bootblockother.o > $(OUT)/entryother.asm

INITCODESRC = arch/x86_64/initcode64.S
$(OUT)/initcode: $(INITCODESRC)
	@mkdir -p $(OUT)
	$(CC) $(CFLAGS) -nostdinc -I. -o $(OUT)/initcode.o -c $(INITCODESRC)
	$(LD) $(LDFLAGS) --omagic -e start -Ttext 0 -o $(OUT)/initcode.out out/initcode.o
	$(OBJCOPY) -S -O binary out/initcode.out $(OUT)/initcode
	$(OBJDUMP) -S $(OUT)/initcode.o > $(OUT)/initcode.asm

ENTRYCODE = $(ARCHOBJ_DIR)/entry64.o
LINKSCRIPT = arch/x86_64/kernel64.ld
LIBS=
$(OUT)/kernel.elf: $(OBJS)  $(OUT)/entryother $(OUT)/initcode $(LINKSCRIPT) $(FSIMAGE)
	$(LD) $(LDFLAGS) -T $(LINKSCRIPT) -o $(OUT)/kernel.elf \
		$(OBJS) $(LIBS)\
		-b binary $(OUT)/initcode $(OUT)/entryother $(FSIMAGE)
	$(OBJDUMP) -S $(OUT)/kernel.elf > $(OUT)/kernel.asm
	$(OBJDUMP) -t $(OUT)/kernel.elf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(OUT)/kernel.sym

MKVECTORS = tools/vectors64.pl
kernel/vectors.S: $(MKVECTORS)
	perl $(MKVECTORS) > kernel/vectors.S

ULIB := \
	ulib.o \
	usys.o \
	printf.o \
	umalloc.o

ULIB := $(addprefix $(UOBJ_DIR)/,$(ULIB))

FS_DIR = .fs

LDFLAGS_user = $(LDFLAGS)

# use simple contiguous section layout and do not use dynamic linking
LDFLAGS_user += --omagic # same as "-N"

# where program execution should begin
LDFLAGS_user += --entry=main

# location in memory where the program will be loaded
LDFLAGS_user += --section-start=.text=0x0 # same of "-Ttext="

$(FS_DIR)/%: $(UOBJ_DIR)/%.o $(ULIB)
	@mkdir -p $(FS_DIR) $(OUT)
	$(LD) $(LDFLAGS_user) -o $@ $^
	$(OBJDUMP) -S $@ > $(OUT)/$*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(OUT)/$*.sym

$(FS_DIR)/forktest: $(UOBJ_DIR)/forktest.o $(ULIB)
	@mkdir -p $(FS_DIR)
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD) $(LDFLAGS_user) -o $(FS_DIR)/forktest \
		$(UOBJ_DIR)/forktest.o \
		$(UOBJ_DIR)/ulib.o \
		$(UOBJ_DIR)/usys.o
	$(OBJDUMP) -S $(FS_DIR)/forktest > $(OUT)/forktest.asm

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: $(UOBJ_DIR)/%.o

UPROGS := \
	cat \
	chmod \
	echo \
	forktest \
	grep \
	init \
	kill \
	ln \
	ls \
	mkdir \
	rm \
	sh \
	stressfs \
	usertests \
	wc \
	zombie

UPROGS := $(addprefix $(FS_DIR)/,$(UPROGS))

$(FS_DIR)/README: README
	@mkdir -p $(FS_DIR)
	cp -f README $(FS_DIR)/README


#-include */*.d

clean: 
	rm -rf $(OUT) $(FS_DIR) $(UOBJ_DIR) $(KOBJ_DIR) $(ARCHOBJ_DIR) $(DOBJ_DIR)
	rm -f kernel/vectors.S yaos.img yaosmemfs.img  .gdbinit
	rm -rf $(MODULEC_OBJECTS)
	rm -rf $(LIBS_OBJECTS)
# run in emulators

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

# number of CPUs to emulate in QEMU
ifndef CPUS
CPUS := $(shell grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)
endif
QEMUOPTS =-enable-kvm -cpu host,+x2apic -kernel out/kernel.elf -smp $(CPUS) -m 512 $(QEMUEXTRA)

qemu: yaos.img
	@echo Ctrl+a h for help
	$(QEMU) -serial mon:stdio  -nographic -netdev type=tap,script=qemu-ifup.sh,id=net0 -device virtio-net-pci,netdev=net0 $(QEMUOPTS)

qemu-memfs: yaosmemfs.img
	@echo Ctrl+a h for help
	$(QEMU) yaosmemfs.img -smp $(CPUS)

qemu-nox:  yaos.img
	@echo Ctrl+a h for help
	$(QEMU) -nographic $(QEMUOPTS)
.gdbinit: tools/gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

.gdbinit64: tools/gdbinit64.tmpl out/kernel.asm out/bootblock.asm
	cat tools/gdbinit64.tmpl | sed "s/localhost:1234/localhost:$(GDBPORT)/" | sed "s/TO32/$(shell grep 'call\s*\*0x1c' out/bootblock.asm  | cut -f 1 -d: |sed -e 's/^\s*/\*0x/')/" | sed "s/TO64/$(shell grep 'ljmp' -A 1 out/kernel.asm  | grep 'ffffffff80' | cut -f 1 -d: | sed "s/ffffffff80/\*0x/")/" > $@

.gdbinit64-2: tools/gdbinit64-2.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu-gdb:  yaos.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	@echo Ctrl+a h for help
	$(QEMU) -serial mon:stdio $(QEMUOPTS) -S $(QEMUGDB)

qemu-nox-gdb:  yaos.img .gdbinit .gdbinit64 .gdbinit64-2
	@echo "*** Now run 'gdb'." 1>&2
	@echo Ctrl+a h for help
	$(QEMU) -nographic $(QEMUOPTS) -S $(QEMUGDB)

.DEFAULT:
	@echo "No rule to make target $@"
