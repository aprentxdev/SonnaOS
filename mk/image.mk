INITRD_DIR = $(BUILD_DIR)/INITRD_DIR
INITRD     = $(BUILD_DIR)/initrd.cpio

initrd: spleen userspace
	mkdir -p $(INITRD_DIR)/bin $(INITRD_DIR)/fonts $(INITRD_DIR)/assets
	cp $(SPLEEN_DIR)/spleen-12x24.psfu $(INITRD_DIR)/fonts/
	cp logo.raw $(INITRD_DIR)/assets/logo.raw
	$(foreach prog,$(USER_PROGRAMS), \
		cp $(BUILD_DIR)/$(prog).elf $(INITRD_DIR)/bin/$(prog).elf;)
	cd $(INITRD_DIR) && find . | cpio -o --format=newc > ../../$(INITRD)

esp: initrd limine spleen $(TARGET) userspace limine.conf
	mkdir -p $(ESP_DIR)/EFI/BOOT $(ESP_DIR)/boot/limine
	cp $(LIMINE_DIR)/BOOTX64.EFI  $(ESP_DIR)/EFI/BOOT/BOOTX64.EFI
	cp $(TARGET)                  $(ESP_DIR)/boot/estella.elf
	cp limine.conf                $(ESP_DIR)/boot/limine/limine.conf
	cp $(INITRD)                  $(ESP_DIR)/boot/initrd.cpio

$(DISK_IMG): esp
	rm -f $@
	dd if=/dev/zero of=$@ bs=1M count=$(IMG_SIZE_MB)
	mkfs.fat -F 32 $@
	mcopy -s -i $@ $(ESP_DIR)/* ::/

img: $(DISK_IMG)

.PHONY: initrd esp img