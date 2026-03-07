TARGET = $(BUILD_DIR)/estella.elf

C_SOURCES   := $(shell find $(SRC_DIR) -type f -name '*.c')
ASM_SOURCES := $(shell find $(SRC_DIR) -type f -name '*.S')

KERNEL_OBJECTS := \
    $(patsubst $(SRC_DIR)/%.c,  $(BUILD_DIR)/%.o, $(C_SOURCES)) \
    $(patsubst $(SRC_DIR)/%.S,  $(BUILD_DIR)/%.o, $(ASM_SOURCES))

KERNEL_DEPENDS := $(KERNEL_OBJECTS:.o=.d)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(KERNEL_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	mkdir -p $(@D)
	$(CC) $(KERNEL_CFLAGS) -c $< -o $@

$(TARGET): $(KERNEL_OBJECTS) x86-64.lds
	$(LD) $(KERNEL_LDFLAGS) -o $@ $(KERNEL_OBJECTS)

-include $(KERNEL_DEPENDS)