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

# Determinte objects to be created
OBJECTS := $(ASOURCES:%.S=%.o)
OBJECTS += $(CCSOURCES:%.c=%.o)
OBJECTS += $(CXXSOURCES:%.cpp=%.o)

# define a name for linking against this lib
BINARY-$(TARGET) := $(OBJDIR)/$(TARGET).a
BINARY := $(OBJDIR)/$(TARGET).a

# Main targets
all: $(BINARY)
$(TARGET): $(BINARY)

$(OBJDIR)/$(TARGET).a: $(addprefix $(OBJDIR)/,$(OBJECTS))
	$(call cmd_msg,AR,$(@))
	$(Q)$(AR) rcs $@ $^

# Cleaning
clean: clean-$(TARGET)
clean-$(TARGET): OBJDIR:=$(OBJDIR)
clean-$(TARGET):
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

.PHONY: clean-$(TARGET) $(TARGET)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean-$(TARGET))
-include $(addprefix $(OBJDIR)/, $(OBJECTS:%.o=%.d))
endif
endif
endif
