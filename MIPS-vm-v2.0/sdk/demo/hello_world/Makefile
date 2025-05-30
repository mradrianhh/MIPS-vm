TARGET := helloworld

BUILD_DIRS := ./build
SRC_DIRS := ./src

SRCS := $(shell find $(SRC_DIRS) -name '*.c' -or -name '*.s' -or -name '*.S')

# Prepends BUILD_DIR and appends .o to every src file
# As an example, ./your_dir/hello.cpp turns into ./build/./your_dir/hello.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIRS)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Use 'mipsel' for little endian or 'mips' for big endian.
CFLAGS := $(INC_FLAGS) -MMD -MP -ffreestanding -v 
CC = mips-none-elf-gcc

LDFLAGS := -T ../../R3000.lds -Map $(BUILD_DIRS)/mapfile.txt --oformat=binary
LD = mips-none-elf-ld

# The final build step.
$(BUILD_DIRS)/$(TARGET): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LDFLAGS)

# Build step for asm source
$(BUILD_DIRS)/%.s.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Build step for C source
$(BUILD_DIRS)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIRS)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -r $(BUILD_DIRS)

