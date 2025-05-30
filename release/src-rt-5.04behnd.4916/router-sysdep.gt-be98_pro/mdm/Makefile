#***********************************************************************
#
#  Copyright (c) 2006-2009  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************

LIB = libmdm.so
OBJS := Baseline_1.o

all install: conditional_build

clean: generic_clean
	rm -f $(INSTALL_DIR)/lib/private/$(LIB)



#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

ifneq ($(strip $(BRCM_VOICE_SUPPORT)),)
include $(BUILD_DIR)/make.voice
endif

ifneq ($(strip $(BUILD_BRCM_CMS)),)
ifneq ($(strip $(BUILD_TR98_PROFILES)),)
conditional_build: generic_private_lib_install
else
conditional_build:
	@echo "Skipping $(LIB) (BUILD_TR98_PROFILES not configured)"
endif
else
conditional_build:
	@echo "Skipping $(LIB) (BRCM_CMS not configured)"
endif


#
# Private apps and libs are allowed to include header files from the
# private and public directories
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I.\
                         -I $(BCM_FSBUILD_DIR)/public/include \
                         -I $(BCM_FSBUILD_DIR)/private/include \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR)

# the mdm lib needs special access to the cms_core lib
ALLOWED_INCLUDE_PATHS += -I../cms_core

# treat all warnings as errors
CUSTOM_CFLAGS += -Werror -Wfatal-errors


#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
# 
$(LIB): $(OBJS)
	$(CC) -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) -o $@ $(OBJS) -Wl,--no-whole-archive


# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)


# For debugging the Makefile
shell:
	bash -i
