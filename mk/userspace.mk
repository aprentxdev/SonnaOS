USER_PROGRAMS  := task_a task_b task_c

USER_LIB_SRC  := $(shell find userspace/lib -name '*.c')
USER_LIB_OBJ  := $(patsubst userspace/lib/%.c, \
                    $(BUILD_DIR)/userspace/lib/%.o, $(USER_LIB_SRC))

USER_CRT0_OBJ  := $(BUILD_DIR)/userspace/crt0.o
USER_PROG_OBJS := $(addprefix $(BUILD_DIR)/userspace/programs/, \
                    $(addsuffix .o, $(USER_PROGRAMS)))
USER_ELFS      := $(addprefix $(BUILD_DIR)/, \
                    $(addsuffix .elf, $(USER_PROGRAMS)))

$(BUILD_DIR)/userspace/crt0.o: userspace/crt0.S
	mkdir -p $(@D)
	$(CC) $(USER_CFLAGS) -c $< -o $@

$(BUILD_DIR)/userspace/lib/%.o: userspace/lib/%.c
	mkdir -p $(@D)
	$(CC) $(USER_CFLAGS) -c $< -o $@

$(BUILD_DIR)/userspace/programs/%.o: userspace/programs/%.c
	mkdir -p $(@D)
	$(CC) $(USER_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.elf: $(USER_CRT0_OBJ) \
                    $(BUILD_DIR)/userspace/programs/%.o \
                    $(USER_LIB_OBJ)
	$(LD) $(USER_LDFLAGS) -o $@ $^

userspace: $(USER_ELFS)

.PHONY: userspace