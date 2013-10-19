# Target file name.
TARGET = libCMSIS

# List C source files here.
CCSOURCES = DeviceSupport/ST/STM32F4xx/system_stm32f4xx.c \
            DeviceSupport/ST/STM32F4xx/startup/startup_stm32f4xx.c

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

# Additional include paths to consider
INCLUDES = $(SELF_DIR)/$(TARGET)/CoreSupport \
           $(SELF_DIR)/$(TARGET)/DeviceSupport/ST/STM32F4xx \
           $(SELF_DIR)/libstm32f4xx/inc
           
# Folder for object files
OBJDIR = obj

# Folder for sourcecode
SRCDIR = .

# Additional defines
DEFINES :=

include $(ROOT)/build/targets/lib.mak
