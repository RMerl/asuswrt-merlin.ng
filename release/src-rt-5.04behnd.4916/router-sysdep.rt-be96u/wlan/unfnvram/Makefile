#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#

default: all

LIB := libnvram.so
CMD := nvram

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common
include $(BUILD_DIR)/make.wlan


ifeq ($(strip $(DESKTOP_LINUX)),y)
BCM_BLT32_FLAGS := $(BCM_LD_FLAGS)
endif

LIBS = -ldl -lstaged

ifneq ($(strip $(BUILD_BRCM_BDK)),)
# Remove all mdm_cbk_* and bcm_*_hal libs from CMS_CORE_LIBS.  The local
# Makefile will specify which mdm_cbk and hal libs it needs.
MDM_CORE_LIBS := $(patsubst -lmdm_cbk_%,,$(CMS_CORE_LIBS))
MDM_CORE_LIBS := $(patsubst -lbcm_%_hal,,$(MDM_CORE_LIBS))
MDM_CORE_LIBS += -lmdm_cbk_wifi 
else
MDM_CORE_LIBS := $(CMS_CORE_LIBS)
endif

ifeq ($(strip $(BUILD_RDKWIFI)),)
ifneq ($(strip $(BUILD_BRCM_BDK)),)
LIBS += -lbcm_zbus_intf
ifneq ($(strip $(BUILD_DBUS)),)
LIBS += -lbcm_dbus_intf -lgio-2.0
else
LIBS += -lbcm_ubus_intf
LIBS += -lubox -lubus -lblobmsg_json
endif
endif
endif
ifneq ($(strip $(BUILD_CUSTOMER)),)
CFLAGS+=-DCUSTOMER_BUILD
endif

ifneq ($(strip $(BUILD_BRCM_CMS))$(strip $(BUILD_BRCM_BDK)),)
LIBS += -lcms_msg $(CMS_COMMON_LIBS) $(MDM_CORE_LIBS)
 ifneq ($(strip $(BUILD_CUSTOMER)),)
  LIBS += -lwlmdm_customer
 else
  LIBS += -lwlmdm
 endif
NVRAM_LIBS = -lnvram -lcms_util $(LIBS)
else
LIBS += -lbcm_util -lsys_util
CFLAGS += -DBASESHELL_BUILD
NVRAM_LIBS = -lnvram -lbcm_util -lsys_util
endif

# Always enable this for nvram kernelset
CFLAGS += -DNAND_SYS

ifneq ($(strip $(BUILD_RDKWIFI)),)
CFLAGS += -DBUILD_RDKWIFI
ifneq ($(wildcard $(WIRELESS_IMPL_PATH)/cmwifi/apps/nvram),)
CFLAGS += -DWLAN_CM_NVRAM_EXT -DCMWIFI_NVRAM_ENC
WLAN_CM_NVRAM_EXT=y
endif
endif

ifneq ($(strip $(RDK_BUILD)),)
CFLAGS += -DRDK_BUILD $(USR_CFLAGS)
endif

ifneq ($(strip $(BUILD_BTEST_TOOLS)),)
CFLAGS += -DBTEST
endif

ifneq ($(strip $(BUILD_DISTRIBUTED_MDM)),)
CFLAGS += -DBUILD_DISTRIBUTED_MDM
endif

#
# this flag is to have the capability to determine if a nvram 
# which coming from "nvram set/get" is a "kernel nvram". 
# If it's identified as kernel nvram, then re-direct to "nvram kset/kget" implicitly.
#
#CFLAGS += -DKERNEL_NVRAM_AUTO_SET

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
                         -I$(INC_WIRELESS_DRIVER_PATH) \
                         -I$(BUILD_DIR)/userspace/private/libs/staged \
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(BCM_FSBUILD_DIR)/public/include/json-c

ifneq ($(strip $(BUILD_BRCM_CMS))$(strip $(BUILD_BRCM_BDK)),)
ALLOWED_INCLUDE_PATHS += -I$(BCM_FSBUILD_DIR)/private/include
endif

KERNEL_NVRAM_FILE_NAME := kernel_nvram.setting
CFLAGS += -DKERNEL_NVRAM_FILE_NAME='"/data/.$(KERNEL_NVRAM_FILE_NAME)"'

ifeq ($(strip $(BUILD_BRCM_CMS))$(strip $(BUILD_BRCM_BDK)),)
USER_NVRAM_FILE_NAME := user_nvram.setting
CFLAGS += -DUSER_NVRAM_FILE_NAME='"/data/.$(USER_NVRAM_FILE_NAME)"'
endif

ALLOWED_INCLUDE_PATHS += -I$(BUILD_DIR)/userspace/private/libs/wlcsm/include
#
# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/lib/public:/lib/private
LIB_RPATH = $(INSTALL_DIR)$(subst :,:$(INSTALL_DIR),$(ALLOWED_LIB_DIRS))

%.o: %.c
	$(CC) -c $(CFLAGS) $(BCM_BLT32_FLAGS) -o $@ $<

OBJS = nvram_common.o nvram_netlink.o nvram_file.o nvram_utils.o nvram_hash.o
ifneq ($(strip $(BUILD_BRCM_CMS))$(strip $(BUILD_BRCM_BDK)),)
OBJS += nvram_api.o
else
OBJS += nvram_api_nonCms.o
LIBS += -ljson-c
endif

ifneq ($(strip $(WLAN_CM_NVRAM_EXT)),)
OBJS += $(WIRELESS_IMPL_PATH)/cmwifi/apps/nvram/nvram_ext.o
ALLOWED_INCLUDE_PATHS += -I$(WIRELESS_IMPL_PATH)/cmwifi/apps/nvram
endif

ifeq ($(strip $(BUILD_RDKWIFI)),)
ifneq ($(strip $(BUILD_BRCM_BDK)),)
OBJS += nvram_zbus_out.o nvram_zbus_in.o
ifneq ($(strip $(BUILD_DBUS)),)
OBJS += nvram_dbus_out.o nvram_dbus_in.o
ALLOWED_INCLUDE_PATHS += -I $(BCM_FSBUILD_DIR)/public/lib/glib-2.0/include \
                         -I $(BCM_FSBUILD_DIR)/public/include/glib-2.0
CFLAGS += -DBUILD_DBUS
else
ifneq ($(strip $(BUILD_UBUS)),)
OBJS += nvram_ubus_out.o nvram_ubus_in.o
CFLAGS += -DBUILD_UBUS
endif
endif
endif
endif


all: $(LIB) $(CMD)
	mkdir -p $(INSTALL_DIR)/etc/init.d
	mkdir -p $(INSTALL_DIR)/etc/rc3.d
	mkdir -p $(INSTALL_DIR)/etc/wlan
	mkdir -p $(INSTALL_DIR)/usr/sbin
	mkdir -p $(INSTALL_DIR)/usr/bin
	install -m 755 $(CMD) $(INSTALL_DIR)/bin
	-install -m 644 $(WIRELESS_IMPL_PATH)/cmwifi/release/apps_defaults.txt $(INSTALL_DIR)/etc/wlan 2>/dev/null

dynamic: all

ifneq ($(strip $(BUILD_BRCM_CMS))$(strip $(BUILD_BRCM_BDK)),)
 ifneq ($(strip $(BUILD_CUSTOMER)),)
  $(LIB): $(OBJS) $(BUILD_DIR)/userspace/private/libs/wlmdm_customer/libwlmdm_customer.so
 else
  $(LIB): $(OBJS) $(BUILD_DIR)/userspace/private/libs/wlmdm/libwlmdm.so
 endif
else
$(LIB): $(OBJS)
endif
	$(CC) $(CFLAGS) $(BCM_BLT32_FLAGS) -shared -Wl,-soname,$(LIB) -o $(LIB) $(OBJS) -L$(LIBDIR) $(CMS_LIB_PATH) -lc $(LIBS)
	mkdir -p $(BCM_FSBUILD_DIR)/private/include
	install -m 644 nvram_api.h $(BCM_FSBUILD_DIR)/private/include
ifneq ($(strip $(WLAN_CM_NVRAM_EXT)),)
	install -m 644 $(WIRELESS_IMPL_PATH)/cmwifi/apps/nvram/nvram_ext.h $(BCM_FSBUILD_DIR)/private/include
endif
	install -m 644 nvram_zbus_in.h $(BCM_FSBUILD_DIR)/private/include
	install -m 755 $(LIB) $(INSTALL_DIR)/lib


$(CMD): nvram_cmd.o $(LIB)
	$(CC) $(CFLAGS) $(BCM_BLT32_FLAGS) -Wl,-rpath-link=$(LIB_RPATH) $(CMS_LIB_PATH) -o $@ nvram_cmd.o $(NVRAM_LIBS)

clean:
	-rm -f *.o
	-rm -f $(LIB) $(CMD) $(OBJS)
	-rm -f $(INSTALL_DIR)/lib/$(LIB)
	-rm -f $(INSTALL_DIR)/bin/$(CMD)
	-rm -f $(BCM_FSBUILD_DIR)/private/include/nvram_api.h

#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

#-include $(OBJS:.o=.d)
