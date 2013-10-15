SUBDIRS := 01_leds 02_rawgpio 03_gpioinput 04_gpiointerrupt

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(abspath $(addprefix $(SELF_DIR),$(addsuffix /target.mak,$(SUBDIRS))))
SELF_DIR := $(abspath $(SELF_DIR)/..)/
