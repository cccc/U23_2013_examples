# Target file name.
TARGET = libsystem

# List C source files here.
CCSOURCES = NewlibSyscalls.c \
			SysTick.c \
			Debug.c \
			USART.c \
			Random.c \
			Audio.c \
			Synth.c \
			Spi.c \
			LED.c \
			Accelerometer.c \
			Init.c

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
INCLUDES = $(SELF_DIR)/$(TARGET)/inc \
           $(SELF_DIR)/libCMSIS/CoreSupport \
           $(SELF_DIR)/libCMSIS/DeviceSupport/ST/STM32F4xx \
           $(SELF_DIR)/libstm32f4xx/inc
           
# Folder for object files
OBJDIR = obj

# Folder for sourcecode
SRCDIR = src

# Additional defines
DEFINES :=

include $(ROOT)/build/targets/lib.mak
