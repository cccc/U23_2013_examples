#current directory
#FIXME: this looks strange somehow...
CURDIR := $(SELF_DIR)$(TARGET)

# Variable mangling
OBJDIR := $(addprefix $(CURDIR)/,$(OBJDIR))
SRCDIR := $(addprefix $(CURDIR)/,$(SRCDIR))

# C compiler flags
CFLAGS := $(CFLAGS)
CFLAGS += $(addprefix -I,$(INCLUDES))
CFLAGS += $(GLOBAL_DEFINES) $(DEFINES)

# C++ compiler flags
CXXFLAGS := $(CXXFLAGS)
CXXFLAGS += $(addprefix -I,$(INCLUDES))
CXXFLAGS += $(GLOBAL_DEFINES) $(DEFINES)

# LD flags
LDFLAGS := $(LDFLAGS)

# Determinte objects to be created
OBJECTS := $(ASOURCES:%.S=%.o)
OBJECTS += $(CCSOURCES:%.c=%.o)
OBJECTS += $(CXXSOURCES:%.cpp=%.o)

# Build lib search directories
LIBS := $(LIBS)

# Build dependency list of libraries
LIBDEPS := $(LIBS)

# A name to reference tis target
BINARY := $(OBJDIR)/$(TARGET).bin

# Main targets
all: $(BINARY)
$(TARGET): $(BINARY)

$(OBJDIR)/$(TARGET).bin: $(OBJDIR)/$(TARGET).elf
	$(call cmd_msg,OBJCOPY,$^ -> $(@))
	$(Q)$(OBJCOPY) -O binary $< $@
	
$(OBJDIR)/$(TARGET).elf: LDFLAGS := $(LDFLAGS)
$(OBJDIR)/$(TARGET).elf: $(addprefix $(OBJDIR)/,$(OBJECTS)) $(LIBDEPS)
	$(call cmd_msg,LINK,$(@))
	$(Q)$(CC) $(LDFLAGS) -o $@ -Wl,--whole-archive $^ -Wl,--no-whole-archive -lm

# Cleaning
clean: clean-$(TARGET)
clean-$(TARGET): CURDIR:=$(CURDIR)
clean-$(TARGET): OBJDIR:=$(OBJDIR)
clean-$(TARGET):
	$(Q)$(RM) -f $(CURDIR)/*.map
	$(Q)$(RM) -rf $(OBJDIR)

# Header dependency generation
$(OBJDIR)/%.d: CFLAGS := $(CFLAGS)
$(OBJDIR)/%.d: $(SRCDIR)/%.c
#	$(call cmd_msg,DEPENDS,$@)
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) -MM -MG -MP -MT '$(@:%.d=%.o)' $< > $@

$(OBJDIR)/%.d: CXXFLAGS := $(CXXFLAGS)
$(OBJDIR)/%.d: $(SRCDIR)/%.cpp
#	$(call cmd_msg,DEPENDS,$@)
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)$(CXX) $(CXXFLAGS) -MM -MG -MP -MT '$(@:%.d=%.o)' $< > $@

# Compile c files
$(OBJDIR)/%.o: CFLAGS := $(CFLAGS)
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(call cmd_msg,CC,$<)
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

# Compile cpp files
$(OBJDIR)/%.o: CXXFLAGS := $(CXXFLAGS)
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(call cmd_msg,CXX,$<)
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@

# Assemble S files with GAS
$(OBJDIR)/%.o: ASFLAGS := $(ASFLAGS)
$(OBJDIR)/%.o: $(SRCDIR)/%.S
	$(call cmd_msg,AS,$<)
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)$(CC) -c $(ASFLAGS) -o $@ $<


#include jtag adapter rules
include $(ROOT)/build/jtag_adapters/$(JTAG_ADAPTER).mak


.PHONY: clean-$(TARGET) $(TARGET)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean-$(TARGET))
-include $(addprefix $(OBJDIR)/, $(OBJECTS:%.o=%.d))
endif
endif
endif
