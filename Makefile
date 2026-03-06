CC = clang
LD = ld.lld

KERNEL_CFLAGS = -target x86_64-unknown-elf \
         -ffreestanding -fno-pic -fno-pie -mno-red-zone \
         -fno-stack-protector -fshort-wchar -Wall -O2 \
		 -I kernel -I kernel/include \
		 -MMD -MP -mcmodel=kernel -fno-omit-frame-pointer \
		 -mno-sse
KERNEL_LDFLAGS = -T x86-64.lds -nostdlib

USER_CFLAGS = -target x86_64-unknown-elf \
    -ffreestanding -fno-pic -fno-pie -mno-red-zone \
    -fno-stack-protector -fshort-wchar -Wall -O2 \
    -I userspace/include \
    -nostdlib -static -fno-omit-frame-pointer \
    -mno-sse -fno-asynchronous-unwind-tables

USER_LDFLAGS = -static -no-pie -nostdlib -T userspace/user.ld

LIMINE_DIR ?= ./limine
QEMU ?= qemu-system-x86_64
QEMU_FLAGS ?= -enable-kvm -cpu host,+invtsc  \
              -M q35 -m 2G -serial stdio -display gtk \
              -device VGA,xres=1920,yres=1080 \
              -no-reboot -no-shutdown

OVMF_CODE ?= $(firstword \
    $(wildcard /usr/share/OVMF/OVMF_CODE_4M.fd) \
    $(wildcard /usr/share/OVMF/OVMF_CODE.fd) \
	$(wildcard /usr/share/OVMF/x64/OVMF_CODE.4m.fd) \
    $(wildcard /usr/share/edk2/ovmf/OVMF_CODE.fd) \
    $(wildcard /usr/share/edk2-ovmf/x64/OVMF_CODE.fd) \
    $(wildcard /usr/share/qemu/ovmf-x86_64-code.fd) \
    $(wildcard /opt/homebrew/share/qemu/edk2-x86_64-code.fd) \
    $(wildcard /usr/local/share/qemu/OVMF_CODE.fd) \
)

ifeq ($(OVMF_CODE),)
$(error OVMF not found.)
endif

BUILD_DIR = build
SRC_DIR = kernel
ESP_DIR = $(BUILD_DIR)/ESP

DISK_IMG = $(BUILD_DIR)/SonnaOS-estella.img
IMG_SIZE_MB ?= 64

SPLEEN_URL ?= https://github.com/fcambus/spleen/releases/download/2.2.0/spleen-2.2.0.tar.gz
SPLEEN_TAR ?= spleen-2.2.0.tar.gz
SPLEEN_DIR ?= spleen

TARGET = $(BUILD_DIR)/estella.elf

C_SOURCES := $(shell find $(SRC_DIR) -type f -name '*.c')
ASM_SOURCES := $(shell find $(SRC_DIR) -type f -name '*.S')
KERNEL_OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SOURCES)) \
                  $(patsubst $(SRC_DIR)/%.S, $(BUILD_DIR)/%.o, $(ASM_SOURCES))
KERNEL_DEPENDS := $(KERNEL_OBJECTS:.o=.d)

USER_PROGRAMS := task_a task_b task_c
USER_LIB_SRC := $(shell find userspace/lib -name '*.c')
USER_LIB_OBJ := $(patsubst userspace/lib/%.c,$(BUILD_DIR)/userspace/lib/%.o,$(USER_LIB_SRC))
USER_CRT0_OBJ := $(BUILD_DIR)/userspace/crt0.o

USER_PROG_OBJS := $(addprefix $(BUILD_DIR)/userspace/programs/,$(addsuffix .o,$(USER_PROGRAMS)))
USER_ELFS := $(addprefix $(BUILD_DIR)/,$(addsuffix .elf,$(USER_PROGRAMS)))

all: limine spleen $(TARGET) userspace

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(KERNEL_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	mkdir -p $(@D)
	$(CC) $(KERNEL_CFLAGS) -c $< -o $@

$(TARGET): $(KERNEL_OBJECTS) x86-64.lds
	$(LD) $(KERNEL_LDFLAGS) -o $@ $(KERNEL_OBJECTS)

$(BUILD_DIR)/userspace/crt0.o: userspace/crt0.S
	mkdir -p $(@D)
	$(CC) $(USER_CFLAGS) -c $< -o $@

$(BUILD_DIR)/userspace/lib/%.o: userspace/lib/%.c
	mkdir -p $(@D)
	$(CC) $(USER_CFLAGS) -c $< -o $@

$(BUILD_DIR)/userspace/programs/%.o: userspace/programs/%.c
	mkdir -p $(@D)
	$(CC) $(USER_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.elf: $(USER_CRT0_OBJ) $(BUILD_DIR)/userspace/programs/%.o $(USER_LIB_OBJ)
	$(LD) $(USER_LDFLAGS) -o $@ $^

userspace: $(USER_ELFS)

limine:
	if [ ! -d "$(LIMINE_DIR)" ]; then \
		git clone --branch=v10.6.6-binary --depth=1 https://github.com/limine-bootloader/limine.git $(LIMINE_DIR); \
	fi

spleen:
	mkdir -p $(SPLEEN_DIR)
	if command -v curl >/dev/null 2>&1; then \
		curl -L -o $(SPLEEN_TAR) "$(SPLEEN_URL)"; \
	elif command -v wget >/dev/null 2>&1; then \
		wget -O $(SPLEEN_TAR) "$(SPLEEN_URL)"; \
	else \
		echo "||| failed to get spleen fonts" \
		exit 1; \
	fi 
	tar -xzf $(SPLEEN_TAR) \
		--strip-components=1 \
		-C $(SPLEEN_DIR) \
		--wildcards '*.psfu' || true
	rm -f $(SPLEEN_TAR)

esp: limine spleen $(TARGET) userspace limine.conf
	mkdir -p $(ESP_DIR)/EFI/BOOT $(ESP_DIR)/boot/limine $(ESP_DIR)/boot/spleen
	cp $(LIMINE_DIR)/BOOTX64.EFI $(ESP_DIR)/EFI/BOOT/BOOTX64.EFI
	cp $(TARGET) $(ESP_DIR)/boot/estella.elf
	cp limine.conf $(ESP_DIR)/boot/limine/limine.conf
	cp $(SPLEEN_DIR)/spleen-12x24.psfu $(ESP_DIR)/boot/spleen/
	cp logo.raw $(ESP_DIR)/boot/logo.raw
	$(foreach prog,$(USER_PROGRAMS),cp $(BUILD_DIR)/$(prog).elf $(ESP_DIR)/boot/$(prog).elf;)

$(DISK_IMG): esp
	rm -f $@
	dd if=/dev/zero of=$@ bs=1M count=$(IMG_SIZE_MB)
	mkfs.fat -F 32 $@
	mcopy -s -i $@ $(ESP_DIR)/* ::/

img: $(DISK_IMG)

run: img esp
	$(QEMU) $(QEMU_FLAGS) \
		-drive if=pflash,format=raw,unit=0,file=$(OVMF_CODE),readonly=on \
		-drive format=raw,file=$(DISK_IMG)

clean:
	rm -rf $(BUILD_DIR) 

distclean: clean 
	rm -rf $(LIMINE_DIR) $(SPLEEN_DIR)

compdb:
	bear -- make clean all

.PHONY: all userspace esp img run clean distclean compdb

-include $(KERNEL_DEPENDS)