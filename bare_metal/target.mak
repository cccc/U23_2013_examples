SUBDIRS := 01_blink 02_systick 03_pwm 04_spi_bitbanging 05_spi_interrupt 06_spi_dma

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(abspath $(addprefix $(SELF_DIR),$(addsuffix /target.mak,$(SUBDIRS))))
SELF_DIR := $(abspath $(SELF_DIR)/..)/
