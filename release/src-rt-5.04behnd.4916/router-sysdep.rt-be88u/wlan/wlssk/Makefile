
#
# You probably don't need to modify anything in this Makefile.
#

.PHONY: install uninstall clean all scripts-install scripts-clean


default:all

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common
include $(BUILD_DIR)/make.wlan


ifneq ($(strip $(BUILD_RDKWIFI)),)
CFLAGS += $(USR_CFLAGS)
ifneq ($(wildcard $(WIRELESS_IMPL_PATH)/cmwifi/apps/nvram),)
CFLAGS += -DWLAN_CM_NVRAM_EXT -DCMWIFI_NVRAM_ENC
endif
CMWIFI_SETUP_SCRIPT=${WIRELESS_IMPL_PATH}/cmwifi/apps/util/script/_wifi_setup.sh
ifeq ($(wildcard $(CMWIFI_SETUP_SCRIPT)),)
CMWIFI_SETUP_SCRIPT=${WIRELESS_IMPL_PATH}/cmwifi/apps/util/script/wifi_setup.sh
endif
search_patch=\"restore_defaults\")
CMWIFI_DEFAULT_SETUP=$(shell grep "${search_patch}" ${CMWIFI_SETUP_SCRIPT})
ifneq ($(strip $(CMWIFI_DEFAULT_SETUP)),)
CFLAGS += -DCMWIFI_DEFAULT_SETUP
endif
endif

ifneq ($(strip $(BUILD_BRCM_BDK)),)
# Remove all mdm_cbk_* and bcm_*_hal libs from CMS_CORE_LIBS.  The local
# Makefile will specify which mdm_cbk and hal libs it needs.
MDM_CORE_LIBS := $(patsubst -lmdm_cbk_%,,$(CMS_CORE_LIBS))
MDM_CORE_LIBS := $(patsubst -lbcm_%_hal,,$(MDM_CORE_LIBS))
MDM_CORE_LIBS += -lmdm_cbk_wifi
else
MDM_CORE_LIBS := $(CMS_CORE_LIBS)
endif

EXE := wlssk

ifneq ($(strip $(BUILD_OPENSYNC_GW)),)
OBJS = main_dummy.o wlssk_mdm.o
LIBS += -lcms_msg $(CMS_COMMON_LIBS) -lcms_dal -lcms_msg $(CMS_COMMON_LIBS) $(MDM_CORE_LIBS)
else ifeq ($(strip $(BUILD_OPENSYNC)),y)
OBJS = main_dummy.o
else
OBJS = main.o debug.o wlssk.o service.o event.o wlsyscall.o defaults.o

LIBS = -L$(INSTALL_DIR)/lib -L$(INSTALL_DIR)/usr/lib -ldl -lwldpd

ifneq ($(strip $(BUILD_BRCM_CMS))$(strip $(BUILD_BRCM_BDK)),)
 OBJS += cms.o wlssk_mdm.o cms_linkstatus.o
ifeq ($(strip $(RDK_BUILD)),)
 OBJS += thread.o
else
 CFLAGS += $(USR_CFLAGS)
endif
 LIBS += -lssk_util
 LIBS += -lcms_msg $(CMS_COMMON_LIBS) -lcms_dal -lcms_msg $(CMS_COMMON_LIBS) $(MDM_CORE_LIBS)
 ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
  LIBS += -lstaged -ljson-c -lpthread
  ifneq ($(strip $(BUILD_CUSTOMER)),)
   LIBS += -lwlmdm_customer -lssk_util_customer -lmdm_cbk_customer
  else
   LIBS += -lwlmdm
  endif
 endif
else     # BASE_SHELL
 LIBS += -lwlsysutil
 ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
  LIBS += -lstaged -ljson-c -lpthread -lbcm_util -lsys_util
 endif
 CFLAGS += -DBUILD_BRCM_BASE_SHELL
endif

ifneq ($(strip $(BUILD_HND_EAP)),)
CFLAGS += -DBUILD_HND_EAP
endif

ifeq ($(WLTEST),1)
CFLAGS += -DWLTEST
endif

ifneq ($(strip $(BRCM_ENGR_ONLY)),)
CFLAGS += -DBRCM_ENGR_ONLY
endif

LIBS += -lnvram -lm -lshared
ifneq ($(strip $(BUILD_BRCM_HOSTAPD)),)
CFLAGS += -DCONFIG_HOSTAPD  
LIBS += -lbcmcrypto -lnv2hapdcfg -L$(INSTALL_DIR)/usr/lib
endif
ifneq ($(strip $(BUILD_BRCM_OPENWRT)),)
CFLAGS += -DCONFIG_BRCM_OPENWRT
endif
LIBS += -lbcm_flashutil -lbcm_boardctl -lbcm_util -lsys_util -lgen_util

ifneq ($(strip $(BUILD_WLDATAELD)),)
CFLAGS += -DBUILD_WLDATAELD
endif
endif

ifeq ($(strip $(BUILD_OPENWRT_NATIVE)),)
all dynamic: $(EXE)
ifneq ($(strip $(BUILD_BRCM_CMS))$(strip $(BUILD_BRCM_BDK)),)
install: all
else
install: all scripts-install
endif
	install -m 755 $(EXE) $(INSTALL_DIR)/bin
	-ln -sf /usr/sbin/wl  $(INSTALL_DIR)/bin/wlctl
	-ln -sf /usr/sbin/dhd  $(INSTALL_DIR)/bin/dhdctl
else
all dynamic:
	@echo "Skip building $(EXE) with native openwrt build."
install:
	@echo "Skip installing $(EXE) and scripts with native openwrt build."
endif


clean: generic_clean scripts-clean
	rm -f $(INSTALL_DIR)/bin/$(EXE) 
	-rm -f *.o
	-rm -f $(EXE)

scripts-install:
	@echo "wlssk scripts-install: wlssk.sh"
	mkdir -p $(INSTALL_DIR)/etc/init.d
	mkdir -p $(INSTALL_DIR)/etc/rc3.d
	install scripts/wlssk.sh $(INSTALL_DIR)/etc/init.d
	(cd $(INSTALL_DIR)/etc/rc3.d; rm -f S81wlssk.sh ; ln -s ../init.d/wlssk.sh S81wlssk.sh)
	(cd $(INSTALL_DIR)/etc/rc3.d; rm -f K50wlssk.sh ; ln -s ../init.d/wlssk.sh K50wlssk.sh)

scripts-clean:
	rm -f $(INSTALL_DIR)/etc/init.d/wlssk.sh
	rm -f $(INSTALL_DIR)/etc/rc3.d/S81wlssk.sh
	rm -f $(INSTALL_DIR)/etc/rc3.d/K50wlssk.sh

CFLAGS += -I$(HNDAPPS_DIR)/shared  -I$(HNDAPPS_DIR)/../wlioctl/include -I$(HNDAPPS_DIR)/../proto/include
CFLAGS += -I$(HNDAPPS_DIR)/../math/include 

ALLOWED_INCLUDE_PATHS += -I. \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I$(BCM_FSBUILD_DIR)/private/include \
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(INC_BCMDRIVER_PATH)/include

# obviously, this app needs to access kernel headers
ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
						 -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
						 -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
						 -I$(SHARED_DIR)/broadcom/include/$(BRCM_BOARD) \
						 -I$(SHARED_DIR)/opensource/include/$(BRCM_BOARD) \
						 -I$(INC_WIRELESS_DRIVER_PATH) \
						 -I$(WIRELESS_DRIVER_PATH)/bcmcrypto \
						 -I$(WIRELESS_DRIVER_PATH)/include \
						 -I$(WIRELESS_DRIVER_PATH)/shared \
						 -I$(WIRELESS_DRIVER_PATH)/router/shared \
						 -I$(WIRELESS_DRIVER_PATH)/../components/bcmwifi/include \
						 -I$(WIRELESS_DRIVER_PATH)/shared/bcmwifi/include


# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/public/lib:/private/lib:/usr/lib
#

CFLAGS +=-DWIRELESS -DSUPPORT_MIMO -DBCMWPA2
ifneq ($(strip $(BUILD_BCM_WLAN_DPDCTL)),)
CFLAGS += -DBUILD_BCM_WLAN_DPDCTL
endif

CFLAGS += -DWMF
CFLAGS += -DDUCATI
ifneq ($(strip $(BUILD_WL_IDLE_PWRSAVE)),)
   CFLAGS += -DIDLE_PWRSAVE
endif

ifeq ($(BUILD_WLHSPOT),y)
CFLAGS += -DHSPOT_SUPPORT
endif

ifneq ($(strip $(BUILD_WSC)), )
CFLAGS += -DSUPPORT_WSC
ifeq ($(strip $(BUILD_WSC_CONFIG_UPNP_DISABLE)),1)
CFLAGS += -DWSC_CONFIG_UPNP_DISABLE
endif
endif

ifneq ($(BUILD_BRCM_AIRIQ),)
CFLAGS += -DWL_AIR_IQ
endif

ifneq ($(strip $(BUILD_BSTREAM_IQOS)),)
ifneq ($(wildcard $(WIRELESS_DRIVER_PATH)/../components/vendor/trend/iqos),)
CFLAGS += -DWL_BSTREAM_IQOS
endif
endif

LIB_RPATH = $(INSTALL_DIR)$(subst :,:$(INSTALL_DIR),$(ALLOWED_LIB_DIRS))
LDFLAGS += $(BCM_RPATH_LINK_OPTION) -Wl,-rpath-link=$(LIB_RPATH)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<



$(EXE): $(OBJS) 
	$(CC) $(BCM_LD_FLAGS) -s -Os -fomit-frame-pointer $(CMS_LIB_PATH) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) -lc $(LIBS_CC_RT)

#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

#-include $(OBJS:.o=.d)
