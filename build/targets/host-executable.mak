#current directory
#FIXME: this looks strange somehow...
CURDIR-$(TARGET) := $(SELF_DIR)$(TARGET)

# Variable mangling
OBJDIR-$(TARGET) := $(addprefix $(CURDIR-$(TARGET))/,$(OBJDIR))
SRCDIR-$(TARGET) := $(addprefix $(CURDIR-$(TARGET))/,$(SRCDIR))

# C compiler flags
CFLAGS-$(TARGET) := $(CFLAGS)
CFLAGS-$(TARGET) += $(addprefix -I,$(INCLUDES))
CFLAGS-$(TARGET) += $(GLOBAL_DEFINES) $(DEFINES)

# C++ compiler flags
CXXFLAGS-$(TARGET) := $(CXXFLAGS)
CXXFLAGS-$(TARGET) += $(addprefix -I,$(INCLUDES))
CXXFLAGS-$(TARGET) += $(GLOBAL_DEFINES) $(DEFINES)

# Assembler flags
ASFLAGS-$(TARGET) := $(ASFLAGS)
ASFLAGS-$(TARGET) += $(addprefix -I,$(INCLUDES))
ASFLAGS-$(TARGET) += $(GLOBAL_DEFINES) $(DEFINES)

# LD flags
LDFLAGS-$(TARGET) := $(LDFLAGS)

# Determinte objects to be created
OBJECTS-$(TARGET) := $(ASOURCES:%.S=%.o)
OBJECTS-$(TARGET) += $(CCSOURCES:%.c=%.o)
OBJECTS-$(TARGET) += $(CXXSOURCES:%.cpp=%.o)

# A name to reference tis target
BINARY-$(TARGET) := $(OBJDIR-$(TARGET))/$(TARGET).elf

# Main targets
all: $(BINARY-$(TARGET))
$(TARGET): $(BINARY-$(TARGET))
	
$(OBJDIR-$(TARGET))/$(TARGET).elf: LDFLAGS := $(LDFLAGS-$(TARGET))
$(OBJDIR-$(TARGET))/$(TARGET).elf: $(addprefix $(OBJDIR-$(TARGET))/,$(OBJECTS-$(TARGET)))
	$(call cmd_msg,LINK,$(@))
	$(Q)$(HOSTCC) $^ -o $@ $(LDFLAGS)

# Cleaning
clean: clean-$(TARGET)
clean-$(TARGET): CURDIR:=$(CURDIR-$(TARGET))
clean-$(TARGET): OBJDIR:=$(OBJDIR-$(TARGET))
clean-$(TARGET):
	$(Q)$(RM) -f $(CURDIR)/*.map
	$(Q)$(RM) -rf $(OBJDIR)

# Header dependency generation
$(OBJDIR-$(TARGET))/%.d: CFLAGS := $(CFLAGS-$(TARGET))
$(OBJDIR-$(TARGET))/%.d: $(SRCDIR-$(TARGET))/%.c
#	$(call cmd_msg,DEPENDS,$@)
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)$(HOSTCC) $(CFLAGS) -MM -MG -MP -MT '$(@:%.d=%.o)' $< > $@

$(OBJDIR-$(TARGET))/%.d: CXXFLAGS := $(CXXFLAGS-$(TARGET))
$(OBJDIR-$(TARGET))/%.d: $(SRCDIR-$(TARGET))/%.cpp
#	$(call cmd_msg,DEPENDS,$@)
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)$(HOSTCXX) $(CXXFLAGS) -MM -MG -MP -MT '$(@:%.d=%.o)' $< > $@

# Compile c files
$(OBJDIR-$(TARGET))/%.o: CFLAGS := $(CFLAGS-$(TARGET))
$(OBJDIR-$(TARGET))/%.o: $(SRCDIR-$(TARGET))/%.c
	$(call cmd_msg,HOSTCC,$<)
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)$(HOSTCC) $(CFLAGS) -c $< -o $@

# Compile cpp files
$(OBJDIR-$(TARGET))/%.o: CXXFLAGS := $(CXXFLAGS-$(TARGET))
$(OBJDIR-$(TARGET))/%.o: $(SRCDIR-$(TARGET))/%.cpp
	$(call cmd_msg,CXX,$<)
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)$(HOSTCXX) $(CXXFLAGS) -c $< -o $@

# Assemble S files with GAS
$(OBJDIR-$(TARGET))/%.o: ASFLAGS := $(ASFLAGS-$(TARGET))
$(OBJDIR-$(TARGET))/%.o: $(SRCDIR-$(TARGET))/%.S
	$(call cmd_msg,AS,$<)
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)$(HOSTCC) -c $(ASFLAGS) -o $@ $<

.PHONY: clean-$(TARGET) $(TARGET)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean-$(TARGET))
-include $(addprefix $(OBJDIR-$(TARGET))/, $(OBJECTS:%.o=%.d))
endif
endif
endif
