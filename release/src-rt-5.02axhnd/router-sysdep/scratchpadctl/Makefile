#***********************************************************************
#
#  Copyright (c) 2013  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/

#
# In most cases, you only need to modify this first section.
#
EXE = scratchpadctl
OBJS = scratchpadctl.o
LIBS = 

all dynamic install: conditional_build

clean: generic_clean
	rm -f $(INSTALL_DIR)/bin/$(EXE)
	rm -f $(INSTALL_DIR)/bin/pspctl



#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

include $(BUILD_DIR)/make.common


# this is a tiny program, just always build it
conditional_build: sanity_check $(EXE) generic_exe_install
	(cd $(INSTALL_DIR)/bin; rm -f pspctl; ln -s scratchpadctl pspctl)



#
# Public apps are allowed to include header files from the
# public directory.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)



#
# Public libs are allowed to link with libraries from the
# public directory.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/lib/public





#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
# 
ifneq ($(PREBUILT_BCMBIN),1)
$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) -Wl,-rpath,$(CMS_LIB_RPATH) $(CMS_LIB_PATH) $(LIBS)
else
$(EXE):
	cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/$(PRBM_DIR)/$@ ./
endif


#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

-include $(OBJS:.o=.d)


