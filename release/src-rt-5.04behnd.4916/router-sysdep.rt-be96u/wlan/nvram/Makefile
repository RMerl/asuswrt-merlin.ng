#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
DBG_OBJS = wlcsmdbg.o wldbg.o wltest.o
KERNEL_NVRAM_FILE_NAME := kernel_nvram.setting
USER_NVRAM_FILE_NAME := user_nvram.setting

default: all

include $(BUILD_DIR)/make.common
include $(BUILD_DIR)/make.wlan

include $(TOP)/.config
-include $(LINUXDIR)/.config

ifneq ($(strip $(BUILD_BRCM_CMS)),)
LIBS = -lcms_msg $(CMS_COMMON_LIBS) -ldl
else
LIBS = -ldl -lnvram
endif
LIBS += -L$(TOP)/shared -lshared

ifeq ($(RTCONFIG_AMAS),y)
LIBS += -L$(TOP)/json-c/.libs -ljson-c
endif

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-BE82M GS-BE18000 GS-BE12000 GS7_PRO GT7))
LIBS += -L$(TOP)/libhostapi -lhostapi
endif

ifneq ($(strip $(BUILD_RDKWIFI)),)
rdkwifi_lib:
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/wlcsm BUILD_RDKWIFI_LIB=1
else
rdkwifi_lib:
	$(info "nothing to do")
endif

ifneq ($(strip $(WLCSM_DEBUG_TRACE)),)
WLCSM_DEBUG :=  1
CFLAGS += -DWLCSM_DEBUG
else
WLCSM_DEBUG :=  0
endif

# Always enable this for nvram kernelset
CFLAGS += -DNAND_SYS

ifneq ($(strip $(BUILD_BRCM_BDK)),)
# Remove all mdm_cbk_* and bcm_*_hal libs from CMS_CORE_LIBS.  The local
# Makefile will specify which mdm_cbk and hal libs it needs.
MDM_CORE_LIBS := $(patsubst -lmdm_cbk_%,,$(CMS_CORE_LIBS))
MDM_CORE_LIBS := $(patsubst -lbcm_%_hal,,$(MDM_CORE_LIBS))
MDM_CORE_LIBS += -lmdm_cbk_wifi
else
MDM_CORE_LIBS := $(CMS_CORE_LIBS)
endif

LIBS += -L$(INSTALL_DIR)/lib -L$(INSTALL_DIR)/usr/lib -ldl

ifneq ($(strip $(BUILD_BRCM_CMS))$(strip $(BUILD_BRCM_BDK)),)
 ifneq ($(strip $(BUILD_PMD)),)
 LIBS += $(CMS_OPTION_RPATH) -L$(BCM_FSBUILD_DIR)/private/lib -lpmd -L$(BCM_FSBUILD_DIR)/public/lib -ljson-c
 endif
 LIBS += -lcms_msg $(CMS_COMMON_LIBS) -lcms_dal -lcms_msg $(CMS_COMMON_LIBS) $(MDM_CORE_LIBS)
 ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
  LIBS += -lstaged -ljson-c -lpthread
  ifneq ($(strip $(BUILD_CUSTOMER)),)
   LIBS += -lwlmdm_customer
  else
   LIBS += -lwlmdm
  endif
 endif
else
 # LIBS += -lwlsysutil
 ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
  LIBS += -lstaged -ljson-c -lpthread -lbcm_util -lsys_util
 endif
endif

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

# obviously, this app needs to access kernel headers
ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
                         -I$(SHARED_DIR)/broadcom/include/$(BRCM_BOARD) \
                         -I$(SHARED_DIR)/opensource/include/$(BRCM_BOARD) \
                         -I$(INC_WIRELESS_DRIVER_PATH) \
                         -I$(COMM_INC_WIRELESS_DRIVER_PATH) \
                         -I$(WIRELESS_DRIVER_PATH)/bcmcrypto \
                         -I$(WIRELESS_DRIVER_PATH)/shared \
                         -I$(WIRELESS_DRIVER_PATH)/router/shared

CFLAGS += -DDSLCPE_WLCSM_EXT -DKERNEL_NVRAM_FILE_NAME='"/data/.$(KERNEL_NVRAM_FILE_NAME)"'
ALLOWED_INCLUDE_PATHS += -I$(TOP_PLATFORM)/wlcsm/include
ifeq ($(strip $(BUILD_RDKWIFI)),)
WLCSM_LIB := $(TOP_PLATFORM)/wlcsm/libwlcsm.a
SHARED_LIB := -L$(TOP)/shared -lshared
LIBS += $(WLCSM_LIB)
LIBDIR = $(INSTALL_DIR)/lib
endif
ifneq ($(findstring CONFIG_DUMP_PREV_OOPS_MSG=, $(shell cat $(LINUXDIR)/.config)),)
CFLAGS += -DDUMP_PREV_OOPS_MSG
endif
ifeq ($(BUILD_NAME),GT10)
CFLAGS += -DGT10
endif
CFLAGS += -DWL_DEFAULT_NUM_SSID=$(CONFIG_BCM_DEFAULT_NUM_MBSS)
LDFLAGS +=-lc $(LIBS_CC_RT) 
#
# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ifneq ($(strip $(BUILD_RDKWIFI)),)
CFLAGS +=-DBUILD_RDKWIFI 
DBG_LDFLAGS += $(LDFLAGS) -lwldm  -lnvram -lshared $(LIBS)
endif
ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
DBG_LDFLAGS += $(LDFLAGS) -lnvram -lshared $(LIBS)
endif

ALLOWED_LIB_DIRS := /lib:/lib/public:/lib/private

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

ifeq ($(strip $(WLCSM_DEBUG)),1)
OBJS = nvram_shared.o main.o brcm_generic_lock.o wlcsmdbg.o wldbg.o wltest.o
else
OBJS = nvram_shared.o main.o brcm_generic_lock.o
endif
ifeq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
ifneq ($(PREBUILT_BCMBIN),1)
all_tags += libnvram nvram  $(KERNEL_NVRAM_FILE_NAME)
else
all_tags:=
endif
else
all_tags:=
endif

wlcsmdbg_tools:  wlcsmdbg wldbg wltest

all: $(all_tags)
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/init.d
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/rc3.d
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/wlan
	mkdir -p $(INSTALL_DIR)/usr/sbin
ifeq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
	install -m 755 nvram $(INSTALL_DIR)/bin	
	install -m 755 libnvram.so $(INSTALL_DIR)/lib
endif

install:
	@echo done

dynamic: all

ifeq ($(wildcard ./*.c),)
libnvram:
	cp -f ./prebuilt/libnvram.so .
else
libnvram: nvram_shared.o brcm_generic_lock.o $(WLCSM_LIB)
	$(LD) -shared -soname libnvram.so -o libnvram.so nvram_shared.o brcm_generic_lock.o  -L$(LIBDIR) $(WLCSM_LIB) -lc -lpthread
	install -m 755 libnvram.so $(INSTALL_DIR)/lib
endif

ifeq ($(wildcard ./*.c),)
nvram:
	cp -f ./prebuilt/nvram .
else
nvram: libnvram main.o $(WLCSM_LIB)
ifneq ($(strip $(BUILD_BRCM_CMS)),)
	$(CC) -o $@ main.o -s -Os -fomit-frame-pointer $(CMS_LIB_PATH) -L$(LIBDIR) -L$(EXTRALIBDIR) -lc $(LIBS_CC_RT) -lpthread $(LIBS)
else
	$(CC) -o $@ main.o -s -Os -fomit-frame-pointer -L$(LIBDIR) -L$(EXTRALIBDIR) -lc $(LIBS_CC_RT) -lpthread $(LIBS) $(WLCSM_LIB)
endif
endif

clean:
	-rm -f *.o
	-rm -f libnvram.so nvram $(OBJS)
	-rm -f wlcsmdbg wldbg wltest

ifneq ($(strip $(WLCSM_DEBUG_TRACE)),)

wlcsmdbg: rdkwifi_lib wlcsmdbg.o
	$(CC) -o $@ wlcsmdbg.o -fomit-frame-pointer -L$(INSTALL_DIR)/lib -lwlcsm  $(DBG_LDFLAGS) $(LIBS)
	install -m 755 wlcsmdbg $(INSTALL_DIR)/bin

wldbg:  wldbg.o
	$(CC) -o $@ wldbg.o -fomit-frame-pointer -L$(INSTALL_DIR)/lib -lwlcsm  $(DBG_LDFLAGS) $(LIBS)
	install -m 755 wldbg $(INSTALL_DIR)/bin

wltest:  wltest.o
	$(CC) -o $@ wltest.o -fomit-frame-pointer -L$(INSTALL_DIR)/lib -lwlcsm  $(DBG_LDFLAGS) $(LIBS)
	install -m 755 wltest $(INSTALL_DIR)/bin
endif

#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

#-include $(OBJS:.o=.d)
