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

ifeq ($(strip $(DESKTOP_LINUX)),y)
BCM_BLT32_FLAGS := $(BCM_LD_FLAGS)
endif

ifneq ("$(wildcard $(KERNEL_DIR)/.config)","")
include $(KERNEL_DIR)/.config
endif

LIBS = -ldl -lstaged -ljson-c

ifneq ($(strip $(BUILD_BRCM_CMS)),)
LIBS += -lwlmdm -lcms_msg $(CMS_COMMON_LIBS) $(CMS_CORE_LIBS)
NVRAM_LIBS = -lnvram -lcms_util
else
LIBS += -lbcm_util -lsys_util
CFLAGS += -DNOCMS
NVRAM_LIBS = -lnvram -lbcm_util -lsys_util
endif

ifneq ($(strip $(BUILD_BRCM_HNDROUTER_ALONE)),)
CFLAGS += -DBCA_HNDROUTER
endif

ifneq ($(strip $(BUILD_UNIFIED_WLMNGR)),)
CFLAGS +=-DSUPPORT_UNIFIED_WLMNGR
endif

ifneq ($(strip $(CONFIG_MTD_NAND)),)
CFLAGS += -DNAND_SYS
endif

ifneq ($(strip $(BUILD_BTEST_TOOLS)),)
CFLAGS += -DBTEST
endif

ifeq ($(strip $(DISABLE_NOR_RAW_PARTITION)),y)
CFLAGS += -DNAND_SYS
endif

#
# this flag is to have the capability to determine if a nvram 
# which coming from "nvram set/get" is a "kernel nvram". 
# If it's identified as kernel nvram, then re-direct to "nvram kset/kget" implicitly.
#
#CFLAGS += -DKERNEL_NVRAM_AUTO_SET

WL_SRC_BASE:= /main/src
WIRELESS_DRIVER_PATH=$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)
INC_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/include

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
                         -I$(BUILD_DIR)/userspace/private/apps/wlan/wlmngr \
                         -I$(INC_WIRELESS_DRIVER_PATH) \
                         -I$(BUILD_DIR)/userspace/private/libs/staged \
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(BCM_FSBUILD_DIR)/public/include/json-c

ifneq ($(strip $(BUILD_BRCM_CMS)),)
ALLOWED_INCLUDE_PATHS += -I$(BCM_FSBUILD_DIR)/private/include
endif

KERNEL_NVRAM_FILE_NAME := kernel_nvram.setting
CFLAGS += -DKERNEL_NVRAM_FILE_NAME='"/data/.$(KERNEL_NVRAM_FILE_NAME)"'

ifeq ($(strip $(BUILD_BRCM_CMS)),)
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
ifneq ($(strip $(BUILD_BRCM_CMS)),)
OBJS += nvram_api.o
else
OBJS += nvram_api_nonCms.o
endif


all: $(LIB) $(CMD)
	install -m 755 $(CMD) $(INSTALL_DIR)/bin

dynamic: all

ifneq ($(strip $(BUILD_BRCM_CMS)),)
$(LIB): $(OBJS) $(BUILD_DIR)/userspace/private/libs/wlmdm/libwlmdm.so
else
$(LIB): $(OBJS)
endif
	$(CC) $(CFLAGS) $(BCM_BLT32_FLAGS) -shared -Wl,-soname,$(LIB) -o $(LIB) $(OBJS) -L$(LIBDIR) $(CMS_LIB_PATH) -lc $(LIBS)
	mkdir -p $(BCM_FSBUILD_DIR)/private/include
	install -m 644 nvram_api.h $(BCM_FSBUILD_DIR)/private/include
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
