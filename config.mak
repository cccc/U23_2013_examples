#Define used JTAG adapter here, included files for different adapters are located in /build/jtag_adapters
JTAG_ADAPTER := stlink_v2
#JTAG_ADAPTER := jlink

#Main firmware to be flashed when doing 'make upload'
FIRMWARE ?= 01_blink

#enter global defines here
GLOBAL_DEFINES := -DDEBUG

#TODO: same for cflags and ldflags?
