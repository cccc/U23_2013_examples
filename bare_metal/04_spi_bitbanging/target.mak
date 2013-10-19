# Target file name.
TARGET = 04_spi_bitbanging

# List C source files here.
CCSOURCES = main.c \
            shiftbrite.c \
            bitbanging.c \
            lib/CMSIS/DeviceSupport/ST/STM32F4xx/system_stm32f4xx.c \
            lib/CMSIS/DeviceSupport/ST/STM32F4xx/startup/startup_stm32f4xx.c \
            lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
            lib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
            lib/STM32F4xx_StdPeriph_Driver/src/misc.c

# List C++ source files here.
CXXSOURCES =

# List Files to be assembled here
ASOURCES =

# C compiler flags
CFLAGS  = -std=gnu99 -ggdb -O0 -Werror -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -fdata-sections -ffunction-sections

# C++ compiler flags
CXXFLAGS =

# GAS flags
ASFLAGS =

# Linker flags
LDFLAGS  = -mthumb -mcpu=cortex-m4 -mfix-cortex-m3-ldrd -mfloat-abi=hard -mfpu=fpv4-sp-d16 -nostartfiles
LDFLAGS += -Wl,-T,$(SELF_DIR)/$(TARGET)/lib/CMSIS/DeviceSupport/ST/STM32F4xx/startup/stm32f4xx_flash.ld,-Map,$(SELF_DIR)/$(TARGET)/$(TARGET).map -Wl,--gc-sections

# Additional include paths to consider
INCLUDES = $(SELF_DIR)/$(TARGET) \
           $(SELF_DIR)/$(TARGET)/lib/CMSIS/CoreSupport \
           $(SELF_DIR)/$(TARGET)/lib/CMSIS/DeviceSupport/ST/STM32F4xx \
           $(SELF_DIR)/$(TARGET)/lib/STM32F4xx_StdPeriph_Driver/inc

# Additional local static libs to link against
LIBS =
#LIBS = $(BINARY-libstartup) $(BINARY-libstm32f4xx)	#example on how to link against static libs

# Folder for object files
OBJDIR = obj

# Folder for sourcecode
SRCDIR = .

# Additional defines
DEFINES :=

include $(ROOT)/build/targets/executable.mak
