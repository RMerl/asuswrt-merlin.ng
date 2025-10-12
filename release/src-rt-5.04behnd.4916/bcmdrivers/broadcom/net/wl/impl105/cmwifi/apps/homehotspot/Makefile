#***********************************************************************
#
#  Copyright (c) 2006  Broadcom Corporation
#  All Rights Reserved
#
#  <:license-private
#
#***********************************************************************/

#
# In most cases, you only need to modify this first section.
#

EXE  = wlhhs
OBJS = homehotspot.o homehotspot_radius.o
LIBS = -lcms_msg $(CMS_COMMON_LIBS) $(CMS_CORE_LIBS)
LIBS += -lnvram -lshared -lbcmcrypto
ifeq ($(DSLCPE_WLCSM_EXT),1)
LIBS +=-lwlcsm
endif

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common
CFLAGS += -Werror

all dynamic install: generic_exe_install

clean: generic_clean
	rm -f $(INSTALL_DIR)/bin/$(EXE)

WIRELESS_DRIVER_PATH:=$(WLSRCBASE)
INC_WIRELESS_DRIVER_PATH:=$(WIRELESS_DRIVER_PATH)/include

#
# Private apps and libs are allowed to include header files from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/libs/cms_core \
                         -I$(BUILD_DIR)/userspace/private/libs/cms_core/linux \
                         -I$(BUILD_DIR)/userspace/private/libs/cms_core/linux/device2 \
			 -I$(BUILD_DIR)/userspace/private/apps/erouterctld

# obviously, this app needs to access kernel headers
ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
                         -I$(BUILD_DIR)/shared/broadcom/include/$(BRCM_BOARD) \
                         -I$(BUILD_DIR)/shared/opensource/include/$(BRCM_BOARD) \
                         -I$(INC_WIRELESS_DRIVER_PATH) \
			 -I$(WIRELESS_DRIVER_PATH)/common/include \
                         -I$(WIRELESS_DRIVER_PATH)/include/bcmcrypto \
                         -I$(WIRELESS_DRIVER_PATH)/bcmcrypto \
                         -I$(WIRELESS_DRIVER_PATH)/shared \
                         -I$(SRCBASE_ROUTER)/shared \
                         -I$(SRCBASE_ROUTER)/nas \
                         -I$(WIRELESS_DRIVER_PATH)/shared/bcmwifi/include \
                         -I$(WIRELESS_DRIVER_PATH)/../components/shared

#
# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/lib/public:/lib/private

#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
#
wlhhs: $(OBJS)
	$(CC) -g -o $@ $^ -Os -fomit-frame-pointer $(CMS_LIB_PATH) $(LIBS) -L$(LIBDIR) -L$(EXTRALIBDIR) -lc -lgcc_s
#	$(CC) -o $@ $(OBJS) -Wl,-rpath,$(CMS_LIB_RPATH) $(CMS_LIB_PATH) $(LIBS)

#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

-include $(OBJS:.o=.d)
