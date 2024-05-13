TARGET := gcd

BUILD_DIRS := ./build
SRC_DIRS := ./src

SRCS := $(shell find $(SRC_DIRS) -name '*.s')

# Prepends BUILD_DIR and appends .o to every src file
# As an example, ./your_dir/hello.cpp turns into ./build/./your_dir/hello.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIRS)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
ASFLAGS := -g -mips1 -O0 -msoft-float
AS := mips-linux-gnu-as

LD := mips-linux-gnu-ld
LDFLAGS := --no-gc-sections --print-output-format --verbose

OBJDUMP := mips-linux-gnu-objdump
DUMPFLAGS := -S 

# The final build step.
$(BUILD_DIRS)/$(TARGET): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LDFLAGS) -Map $@_map > $@_output

# Build step for asm source
$(BUILD_DIRS)/%.s.o: %.s
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -alg=$@_listing $< -o $@

.PHONY: clean

clean:
	rm -r $(BUILD_DIRS)

dump: 
	$(OBJDUMP) $(DUMPFLAGS) $(BUILD_DIRS)/$(TARGET) > $(BUILD_DIRS)/$(TARGET)_dump

