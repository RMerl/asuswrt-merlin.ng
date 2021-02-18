#***********************************************************************
#
#  Copyright (c) 2007  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/


#
# In most cases, you only need to modify this first section.
#
LIB = libbcm_boardctl.so

OBJS = 

all: sanity_check $(LIB) generic_public_lib_install

install:
	@echo done

clean: generic_oalclean generic_clean
	rm -f $(INSTALL_DIR)/lib/public/$(LIB)



#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common



#
# Public libs are only allowed to include header files from the
# public directory.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR)


# treat all warnings as errors
CUSTOM_CFLAGS += -Werror -Wfatal-errors


#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
#
ifneq ($(PREBUILT_BCMBIN),1)
$(LIB): generic_oaldir $(OBJS)
	$(CC) -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) -o $@ $(OBJS) $(OALDIR)/oal.a -Wl,--no-whole-archive
else
$(LIB): 
	-cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/$(PRBM_DIR)/$@ ./
endif


#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#
# No objects at this level, so no dependencies needed
#include $(BUILD_DIR)/make.deprules

#-include $(OBJS:.o=.d)
