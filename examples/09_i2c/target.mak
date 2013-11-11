# Target file name.
TARGET = 09_i2c

# List C source files here.
CCSOURCES = main.c

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
LDFLAGS += -Wl,-T,$(ROOT)/libs/libCMSIS/DeviceSupport/ST/STM32F4xx/startup/stm32f4xx_flash.ld,-Map,$(SELF_DIR)/$(TARGET)/$(TARGET).map -Wl,--gc-sections

# Additional include paths to consider
INCLUDES = $(ROOT)/libs/libsystem/inc \
           $(ROOT)/libs/libstm32f4xx/inc \
           $(ROOT)/libs/libCMSIS/CoreSupport \
           $(ROOT)/libs/libCMSIS/DeviceSupport/ST/STM32F4xx

# Additional local static libs to link against
LIBS = $(BINARY-libCMSIS) $(BINARY-libstm32f4xx) $(BINARY-libsystem)

# Folder for object files
OBJDIR = obj

# Folder for sourcecode
SRCDIR = src

# Additional defines
DEFINES := 

include $(ROOT)/build/targets/executable.mak
