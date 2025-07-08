######################################
# target
######################################
TARGET = firmware


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
platform/K1921VK035/Source/system_K1921VK035.c \
src/main.c \
# printf
C_SOURCES 	+= app/retarget/retarget.c
# uCMD
C_SOURCES 	+= $(wildcard app/u_read_line/*.c)
C_SOURCES 	+= $(wildcard app/mem/*.c)

# ASM sources
ASM_SOURCES =  \
platform/K1921VK035/Source/startup_K1921VK035.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
# FPU = -mfpu=fpv4-sp-d16
FPU = -mfpu=fpv4-sp-d16 -u _printf_float

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DK1946VK035 \
-DCKO_OSE \
-DSYSCLK_OSE \
-DOSECLK_VAL="8000000" \
-DBAREMETAL

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-Iplatform/K1921VK035/Include \
-Iplatform/CMSIS/Core/Include \

C_INCLUDES += -Iapp/retarget/
C_INCLUDES += -Iapp/u_read_line
C_INCLUDES += -Iapp/mem


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# Этот флаг GCC позволяет смотреть .i и .s файлы. 
CFLAGS += -save-temps=obj 

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = ./platform/ld/K1921VK035.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)


#######################################
# OpenOCD
#######################################
# TODO: Сделать платформонезависимо.

# for windows
# OPENOCD_PATH = C:/work_old1/niiet_riscv_sdk/xpack-openocd-k1921vk-0.12.0-k1921vk-win32-x64/bin
# OPENOCD = $(OPENOCD_PATH)/openocd.exe

# for linux
# OPENOCD = /home/mkaa/vg015/openocd-k1921vk/src/openocd
OPENOCD = openocd

OCD_F = -f ./dbg/cmsis-dap.cfg -f ./dbg/k1921vk035.cfg
OCD_PRE = -c init -c "reset halt" -c "flash probe 0"
OCD_POST = -c "reset run" -c "exit"

program:
	$(OPENOCD) $(OCD_F) $(OCD_PRE) -c "program build/$(TARGET).hex verify" $(OCD_POST)

erase:
	$(OPENOCD) $(OCD_F) $(OCD_PRE) -c "erase" $(OCD_POST)

unbrick:
	echo "Connect SERVEN to VCC, reset controller and press Enter"
	read
	$(OPENOCD) $(OCD_F) -c 'init' -c 'reset halt' -c 'mww 0x40040014 0x00000001' -c 'mdw 0x40040014' -c 'reset run' -c 'exit'

# *** EOF ***
