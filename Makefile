include mk/toolchain.mk
include mk/deps.mk
include mk/kernel.mk
include mk/userspace.mk
include mk/image.mk

all: limine spleen $(TARGET) userspace

run: img
	$(QEMU) $(QEMU_FLAGS) \
		-drive if=pflash,format=raw,unit=0,file=$(OVMF_CODE),readonly=on \
		-drive format=raw,file=$(DISK_IMG)

clean:
	rm -rf $(BUILD_DIR)

distclean: clean
	rm -rf $(LIMINE_DIR) $(SPLEEN_DIR)

compdb:
	bear -- make clean all

.PHONY: all run clean distclean compdb