
#
# You probably don't need to modify anything in this Makefile.
#

.PHONY: install uninstall clean all scripts-install scripts-clean

default:all

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common
include $(BUILD_DIR)/make.wlan

DE_VER := 2_15

ifneq ($(strip $(BUILD_BRCM_BDK)),)
# Remove all mdm_cbk_* and bcm_*_hal libs from CMS_CORE_LIBS.  The local
# Makefile will specify which mdm_cbk and hal libs it needs.
MDM_CORE_LIBS := $(patsubst -lmdm_cbk_%,,$(CMS_CORE_LIBS))
MDM_CORE_LIBS := $(patsubst -lbcm_%_hal,,$(MDM_CORE_LIBS))
MDM_CORE_LIBS += -lmdm_cbk_wifi
else
MDM_CORE_LIBS := $(CMS_CORE_LIBS)
endif

EXE := wldataeld
EXEALT := wldataelm_ctl
OBJS = main.o debug.o lock.o timer.o util.o list.o timer_list.o
OBJS += collection.o i5_dm_access.o wbd_access.o


OBJS += database.o notify_queue.o
OBJS += encapsulation.o cms_helper.o cms_history.o

OBJS += history.o
OBJS += appevent_access.o
OBJS += local_wl_access.o local_wl_util.o

OBJS += ctl_main.o
# OBJS += ieee1905_cmsmdm.o
# Use bcmwifi from wifi driver source
BCMWIFI_OBJS += bcmutils.o bcmxtlv.o bcm_app_utils.o
BCMWIFI_OBJS += bcmwifi_rclass.o
BCMWIFI_OBJS += bcmwifi_channels.o

OBJS += $(BCMWIFI_OBJS)


LIBS = -L$(INSTALL_DIR)/lib -L$(INSTALL_DIR)/usr/lib -ldl -lpthread -lgen_util
LIBS += -ldemap

#/userspace/private/apps/wlan/components/apps/ieee1905
LIBS += -li5api
# -lieee1905
# -lshared

LIBS += -ljson-c
LIBS += -lwldataelm_util

CFLAGS += -DMULTIAP -DMULTIAPR2 $(USR_CFLAGS)

ifneq ($(strip $(BUILD_BCM_WLMLO)),)
OBJS += apmld.o
CFLAGS += -DBUILD_BCM_WLMLO
endif

ifneq ($(strip $(BUILD_BRCM_CMS))$(strip $(BUILD_BRCM_BDK)),)
LIBS += -lcms_msg $(CMS_COMMON_LIBS) -lcms_dal -lcms_msg $(CMS_COMMON_LIBS) $(MDM_CORE_LIBS)
endif

CFLAGS += -DDE_VER_$(DE_VER)

ifneq ($(strip $(BCA_CPEROUTER)),)
LIBS += -lnvram -lm -lshared
endif

all dynamic: $(EXE)

ifneq ($(strip $(BUILD_BRCM_CMS))$(strip $(BUILD_BRCM_BDK)),)
install: all
else
## # No data element support on TR098
ifneq ($(strip $(BUILD_TR98_PROFILES)),)
install: all
else
install: all scripts-install
endif
endif
	install -m 755 $(EXE) $(INSTALL_DIR)/bin
	(cd $(INSTALL_DIR)/bin; ln -sf $(EXE) $(EXEALT))

clean: generic_clean scripts-clean
	rm -f $(INSTALL_DIR)/bin/$(EXE) 
	rm -f $(INSTALL_DIR)/bin/$(EXEALT) 
	-rm -f *.o *.d
	-rm -f $(EXE)

scripts-install:

scripts-clean:

CFLAGS += -I$(HNDAPPS_DIR)/shared  -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include
CFLAGS += -I$(SRCBASE)/../components/math/include 
CFLAGS += -I$(HND_SRC)/router-sysdep/bcm_util/
CFLAGS += -I$(HND_SRC)/router-sysdep/gen_util/
CFLAGS += -I$(HND_SRC)/userspace/public/libs/jsonc/
CFLAGS += -I$(HND_SRC)/router-sysdep/sys_util/
CFLAGS += -I$(HND_SRC)/userspace/public/libs/cms_util/
CFLAGS += -I$(HND_SRC)/data-model/
CFLAGS += -I$(HND_SRC)/router-sysdep/bcm_flashutil/
CFLAGS += -I$(HND_SRC)/userspace/private/libs/wlsysutil/
CFLAGS += -I$(HND_SRC)/userspace/public/libs/cms_msg/

ALLOWED_INCLUDE_PATHS += -I. \
                         -I$(BCM_FSBUILD_DIR)/private/include \
                         -I$(BCM_FSBUILD_DIR)/private/include/cms_core \
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(INC_BCMDRIVER_PATH)/include \
                         -I$(BCM_FSBUILD_DIR)/public/include/json-c

# obviously, this app needs to access wireless driver headers
ALLOWED_INCLUDE_PATHS += -I$(WIRELESS_DRIVER_PATH)/include \
                         -I$(WIRELESS_DRIVER_PATH)/shared \
                         -I$(WIRELESS_DRIVER_PATH)/router/shared \
                         -I$(WIRELESS_DRIVER_PATH)/../components/bcmwifi/include \
                         -I$(WIRELESS_DRIVER_PATH)/shared/bcmwifi/include \
                         -I$(WIRELESS_DRIVER_PATH)/wl

# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/public/lib:/private/lib:/usr/lib
#

ALLOWED_INCLUDE_PATHS += -I$(COMPON_WIRELESS_DRIVER_PATH)/apps/ieee1905
ALLOWED_INCLUDE_PATHS += -I$(COMPON_WIRELESS_DRIVER_PATH)/apps/wbd2
ALLOWED_INCLUDE_PATHS += -I$(COMPON_WIRELESS_DRIVER_PATH)/apps/wbd2/shared
ALLOWED_INCLUDE_PATHS += -I$(COMPON_WIRELESS_DRIVER_PATH)/router/bshared/include
ALLOWED_INCLUDE_PATHS += -I$(COMPON_WIRELESS_DRIVER_PATH)/apps/appeventd


vpath %.c $(WIRELESS_DRIVER_PATH)/../components/bcmwifi/src
vpath %.c $(WIRELESS_DRIVER_PATH)/shared/bcmwifi/src
vpath %.c $(WIRELESS_DRIVER_PATH)/shared

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<



$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(BCM_LD_FLAGS) $(CMS_RPATH_OPTION) $(BCM_RPATH_LINK_OPTION) $(BCM_LIB_PATH) $(LIBS) -lc $(LIBS_CC_RT)

#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
