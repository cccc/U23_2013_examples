SUBDIRS :=  01_leds \
			02_rawgpio \
			03_gpioinput \
			04_gpiointerrupt \
			05_ledpwm \
			06_servo \
			07_accelerometer \
			08_synth \
			09_i2c

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(abspath $(addprefix $(SELF_DIR),$(addsuffix /target.mak,$(SUBDIRS))))
SELF_DIR := $(abspath $(SELF_DIR)/..)/
