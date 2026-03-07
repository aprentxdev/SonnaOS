CC  = clang
LD  = ld.lld
QEMU = qemu-system-x86_64

BUILD_DIR  = build
SRC_DIR    = kernel
ESP_DIR    = $(BUILD_DIR)/ESP
DISK_IMG   = $(BUILD_DIR)/SonnaOS-estella.img
IMG_SIZE_MB ?= 64

KERNEL_CFLAGS = \
    -target x86_64-unknown-elf \
    -ffreestanding -fno-pic -fno-pie -mno-red-zone \
    -fno-stack-protector -fshort-wchar -Wall -O2 \
    -I kernel -I kernel/include \
    -MMD -MP -mcmodel=kernel \
    -fno-omit-frame-pointer -mno-sse

KERNEL_LDFLAGS = -T x86-64.lds -nostdlib

USER_CFLAGS = \
    -target x86_64-unknown-elf \
    -ffreestanding -fno-pic -fno-pie -mno-red-zone \
    -fno-stack-protector -fshort-wchar -Wall -O2 \
    -I userspace/include -nostdlib -static \
    -fno-omit-frame-pointer -mno-sse \
    -fno-asynchronous-unwind-tables

USER_LDFLAGS = -static -no-pie -nostdlib -T userspace/user.ld

QEMU_FLAGS ?= \
    -enable-kvm -cpu host,+invtsc \
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