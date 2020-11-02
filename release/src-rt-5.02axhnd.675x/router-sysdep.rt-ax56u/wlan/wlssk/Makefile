
#
# You probably don't need to modify anything in this Makefile.
#

.PHONY: install uninstall clean all scripts-install scripts-clean


default:all

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common

IMPL=$(shell readlink $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP) | sed 's/[^0-9]//g')
WL_IMPL_PLUS= $(strip $(shell if [ "$(IMPL)" -ge 24 ]; then echo Y; fi ;))
WL_SRC_BASE:=/main/src
WIRELESS_DRIVER_PATH=$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)
INC_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/include
COMM_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/common/include

EXE := wlssk
OBJS = main.o debug.o wlssk.o service.o event.o wlsyscall.o

LIBS = -L$(INSTALL_DIR)/lib -L$(INSTALL_DIR)/usr/lib -ldl

ifneq ($(strip $(BUILD_BRCM_CMS)),)
 OBJS += cms.o
 CFLAGS += -DBUILD_BRCM_CMS
 ifneq ($(strip $(BUILD_PMD)),)
 LIBS += -Wl,-rpath,$(BCM_FSBUILD_DIR)/private/lib:$(BCM_FSBUILD_DIR)/public/lib -L$(BCM_FSBUILD_DIR)/private/lib -lpmd -L$(BCM_FSBUILD_DIR)/public/lib -ljson-c
 endif
 LIBS += -lcms_msg $(CMS_COMMON_LIBS) -lcms_dal -lcms_cli -lcms_msg $(CMS_COMMON_LIBS) $(CMS_CORE_LIBS)
 ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
  LIBS += -lwlmdm -lstaged -ljson-c -lpthread
 endif
else
 LIBS += -lwlsysutil
 ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
  LIBS += -lstaged -ljson-c -lpthread -lbcm_util -lsys_util
 endif
endif

ifneq ($(strip $(BUILD_HND_EAP)),)
CFLAGS += -DBUILD_HND_EAP
endif

ifeq ($(strip $(BCA_CPEROUTER)),)
LIBS += -lwlctl -lnvram -lm  -lwlbcmshared
else
LIBS += -lnvram -lm -lshared
CFLAGS+= -DDSLCPE_WLCONF_CMD
ifneq ($(strip $(BUILD_BRCM_HOSTAPD)),)
CFLAGS += -DCONFIG_HOSTAPD  
LIBS += -lbcmcrypto -lnv2hapdcfg -L$(INSTALL_DIR)/usr/lib
endif
endif
LIBS += -lbcm_flashutil -lbcm_boardctl -lgen_util -lbcm_util -lsys_util


all dynamic: $(EXE)

ifneq ($(strip $(BUILD_BRCM_CMS)),)
install: all
else
install: all scripts-install
endif
	install -m 755 $(EXE) $(INSTALL_DIR)/bin
ifneq ($(strip $(BCA_CPEROUTER)),)
	-ln -sf /usr/sbin/wl  $(INSTALL_DIR)/bin/wlctl
	-ln -sf /usr/sbin/dhd  $(INSTALL_DIR)/bin/dhdctl
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

ifneq ($(strip $(HNDAPPS_IN_COMP)),)
CFLAGS += -I$(HNDAPPS_DIR)/shared  -I$(HNDAPPS_DIR)/../wlioctl/include -I$(HNDAPPS_DIR)/../proto/include
CFLAGS += -I$(HNDAPPS_DIR)/../math/include 
CFLAGS += -I$(HNDAPPS_DIR)/dpsta -I$(HNDAPPS_DIR)/shared
endif

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
						 -I$(BUILD_DIR)/shared/broadcom/include/$(BRCM_BOARD) \
						 -I$(BUILD_DIR)/shared/opensource/include/$(BRCM_BOARD) \
						 -I$(INC_WIRELESS_DRIVER_PATH) \
						 -I$(COMM_WIRELESS_DRIVER_PATH) \
						 -I$(WIRELESS_DRIVER_PATH)/bcmcrypto \
						 -I$(WIRELESS_DRIVER_PATH)/include \
						 -I$(WIRELESS_DRIVER_PATH)/shared \
						 -I$(WIRELESS_DRIVER_PATH)/router/shared \
						 -I$(WIRELESS_DRIVER_PATH)/shared/bcmwifi/include


# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/lib/public:/lib/private:/usr/lib
#
ifeq ($(strip $(BCA_CPEROUTER)),)
# make endian adjustment
CFLAGS += -DDSLCPE_ENDIAN
endif

ifeq ($(strip $(BCA_CPEROUTER)),)
CFLAGS +=-DWIRELESS -DSUPPORT_MIMO -DDSLCPE_SHLIB -DBCMWPA2
else
CFLAGS +=-DWIRELESS -DSUPPORT_MIMO -DBCMWPA2
endif
CFLAGS += -DWMF
CFLAGS += -DDUCATI
ifneq ($(strip $(BUILD_WL_IDLE_PWRSAVE)),)
   CFLAGS += -DIDLE_PWRSAVE
endif

ifeq ($(BUILD_WLHSPOT),y)
CFLAGS += -DHSPOT_SUPPORT
endif

ifeq ($(strip $(CONFIG_EXTACS)),y)
CFLAGS += -DEXT_ACS
endif

ifeq ($(EXT_WPS_VER2),y)
CFLAGS += -DWPS_V2
endif

ifneq ($(strip $(BUILD_WSC)), )
CFLAGS += -DSUPPORT_WSC
ifeq ($(strip $(BUILD_WSC_CONFIG_UPNP_DISABLE)),1)
CFLAGS += -DWSC_CONFIG_UPNP_DISABLE
endif
endif

ifeq ($(EXT_WL11AC),y)
CFLAGS += -DWL11AC
endif

ifneq ($(strip $(BUILD_IEEE1905)),)
CFLAGS += -DDSLCPE_1905
ALLOWED_INCLUDE_PATHS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/wps/brcm_apps/include
endif

ifneq ($(strip $(CONFIG_MFP)),)
CFLAGS += -DMFP
endif

ifneq ($(strip $(CONFIG_RPCAPD)),)
CFLAGS += -D__CONFIG_RPCAPD__
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
LDFLAGS += -Wl,-rpath-link=$(LIB_RPATH)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<



$(EXE): $(OBJS) 
	$(CC) $(BCM_LD_FLAGS) -o $@ $(OBJS) -s -Os -fomit-frame-pointer $(CMS_LIB_PATH) $(LIBS) -L$(LIBDIR) -L$(EXTRALIBDIR) -lc -lgcc_s $(LDFLAGS)

#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

#-include $(OBJS:.o=.d)
