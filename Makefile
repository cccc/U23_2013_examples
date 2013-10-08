ROOT := $(PWD)
include $(ROOT)/build/base.mak
include $(ROOT)/config.mak

STARTTIME := $(shell date +%s)
# Main targets
all:
	$(call cmd_msg,NOTICE,Build completed in $$(($$(date +%s)-$(STARTTIME))) seconds)

upload: upload-$(FIRMWARE)
upload-fast: upload-fast-$(FIRMWARE)
upload-gdb: upload-gdb-$(FIRMWARE)
debug-gdb: debug-gdb-$(FIRMWARE) 

SUBDIRS = bare_metal
SELF_DIR = $(ROOT)/
include $(abspath $(addprefix $(SELF_DIR),$(addsuffix /target.mak,$(SUBDIRS))))

clean: cleaninstall

.PHONY: all clean upload upload-gdb debug-gdb install cleaninstall
