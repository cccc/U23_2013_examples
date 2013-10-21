# Target file name.
TARGET = stlink-trace

# List C source files here.
CCSOURCES = stlink-trace.c

# List C++ source files here.
CXXSOURCES =

# List Files to be assembled here
ASOURCES =

# C compiler flags
CFLAGS  = -std=gnu99 `pkg-config --cflags libusb-1.0`

# C++ compiler flags
CXXFLAGS =

# GAS flags
ASFLAGS =

# Linker flags
LDFLAGS = `pkg-config --libs libusb-1.0`

# Additional include paths to consider
INCLUDES = $(ROOT)/tools/stlink-trace

# Folder for object files
OBJDIR = obj

# Folder for sourcecode
SRCDIR = .

# Additional defines
DEFINES :=

include $(ROOT)/build/targets/host-executable.mak
