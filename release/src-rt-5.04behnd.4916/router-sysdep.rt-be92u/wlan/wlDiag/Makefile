#
# In most cases, you only need to modify this first section.
#
EXE = wlDiagnostic
OBJS = wlDiagnostic.o
EID_FILE = eid_wlDiagnostic.txt


LIBS = -lcms_msg $(CMS_COMMON_LIBS)

all dynamic install: $(EXE) generic_exe_install generic_eid_file_install

clean: generic_clean generic_eid_file_clean
	rm -f $(INSTALL_DIR)/bin/$(EXE)



#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common
include $(BUILD_DIR)/make.wlan


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
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I$(BCM_FSBUILD_DIR)/public/include  \
                         -I$(BCM_FSBUILD_DIR)/private/include \
			 -I$(WIRELESS_IMPL_PATH)/main/components/proto/include \
                         -I$(WIRELESS_IMPL_PATH)/main/components/wlioctl/include \
                         -I$(WIRELESS_IMPL_PATH)/main/src/include \
                         -I$(WIRELESS_IMPL_PATH)/main/src/shared/bcmwifi/include \
                         -I$(WIRELESS_IMPL_PATH)/sys/components/proto/include \
                         -I$(WIRELESS_IMPL_PATH)/sys/components/wlioctl/include \
                         -I$(WIRELESS_IMPL_PATH)/sys/src/include \
                         -I$(WIRELESS_IMPL_PATH)/sys/src/shared/bcmwifi/include \
                         -I$(WIRELESS_IMPL_PATH)/dhd/components/proto/include \
                         -I$(WIRELESS_IMPL_PATH)/dhd/components/wlioctl/include \
                         -I$(WIRELESS_IMPL_PATH)/dhd/components/shared \
                         -I$(WIRELESS_IMPL_PATH)/dhd/components/shared/proto \
                         -I$(WIRELESS_IMPL_PATH)/dhd/src/include \
                         -I$(WIRELESS_IMPL_PATH)/dhd/src/shared/bcmwifi/include



#
# Public libs are allowed to link with libraries from the
# public directory.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/private/lib:/public/lib

ifneq ($(strip $(BUILD_BRCM_BDK)),)
# Remove all mdm_cbk_* and bcm_*_hal libs from CMS_CORE_LIBS.  The local
# Makefile will specify which mdm_cbk and hal libs it needs.
MDM_CORE_LIBS := $(patsubst -lmdm_cbk_%,,$(CMS_CORE_LIBS))
MDM_CORE_LIBS := $(patsubst -lbcm_%_hal,,$(MDM_CORE_LIBS))
MDM_CORE_LIBS += -lmdm_cbk_wifi
else
MDM_CORE_LIBS := $(CMS_CORE_LIBS)
endif

LIBS = $(MDM_CORE_LIBS) -lcms_msg $(CMS_COMMON_LIBS)

# treat all warnings as errors
CUSTOM_CFLAGS += -Werror -Wfatal-errors


#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
# 
$(EXE): $(OBJS)
	$(CC) $(BCM_LD_FLAGS) -o $@ $(OBJS) $(BCM_RPATH_LINK_OPTION) $(BCM_LIB_PATH) $(LIBS)


#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

-include $(OBJS:.o=.d)

