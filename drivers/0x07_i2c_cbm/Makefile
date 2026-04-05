# ------------------------------------------------------------------------------
# @file    Makefile
# @author  Kevin Thomas
# @brief   Build script for RP2350 bare-metal C I2C driver.
#
#          Compiles, links, and generates UF2 firmware for the RP2350.
# ------------------------------------------------------------------------------

# Toolchain
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# Target
TARGET  = i2c

# Directories
SRC_DIR  = Src
INC_DIR  = Inc
BUILD_DIR = build

# CPU flags
CPU_FLAGS = -mcpu=cortex-m33 -mthumb

# Compiler flags
CFLAGS  = $(CPU_FLAGS) -Og -g3 -Wall -Wextra \
          -ffunction-sections -fdata-sections \
          -I$(INC_DIR)

# Linker flags
LDFLAGS = $(CPU_FLAGS) -T linker.ld -nostdlib -Wl,--gc-sections

# Source files
SRCS = $(SRC_DIR)/vector_table.c \
       $(SRC_DIR)/rp2350_reset_handler.c \
       $(SRC_DIR)/rp2350_stack.c \
       $(SRC_DIR)/rp2350_xosc.c \
       $(SRC_DIR)/rp2350_reset.c \
       $(SRC_DIR)/rp2350_coprocessor.c \
       $(SRC_DIR)/rp2350_uart.c \
       $(SRC_DIR)/rp2350_i2c.c \
       $(SRC_DIR)/rp2350_delay.c \
       $(SRC_DIR)/main.c \
       $(SRC_DIR)/image_def.c

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Rules
.PHONY: all clean flash

all: $(BUILD_DIR)/$(TARGET).uf2
	@echo "==================================="
	@echo "SUCCESS! Created $(TARGET).bin and $(TARGET).uf2"
	@echo "==================================="

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(TARGET).uf2: $(BUILD_DIR)/$(TARGET).bin
	python3 uf2conv.py -b 0x10000000 -f 0xe48bff59 -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

flash: $(BUILD_DIR)/$(TARGET).elf
	openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg \
	  -c "adapter speed 5000" \
	  -c "program $< verify reset exit"
