#
# In most cases, you only need to modify this first section.
#
default:all

WLAN_USERSPACE_CFG = 1

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common

CSM_HEADER := $(BUILD_DIR)/bcmdrivers/broadcom/char/wlcsm_ext/bcm9$(BRCM_CHIP)/include/wlcsm_linux.h
IMPL=$(shell readlink $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP) | sed 's/[^0-9]//g')
WL_IMPL_PLUS= $(strip $(shell if [ "$(IMPL)" -ge 24 ]; then echo Y; fi ;))
WL_SRC_BASE:=/main/src
WIRELESS_DRIVER_PATH=$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)
INC_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/include
COMM_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/common/include

ifneq ($(strip $(HNDAPPS_IN_COMP)),)
WL_ROUTER_CFG_FILE := $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)/$(HNDAPPS_BASE)/.config
else
WL_ROUTER_CFG_FILE := $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)/main/src/router/.config
endif

ifneq ($(strip $(BCA_CPEROUTER)),)
-include  $(WL_ROUTER_CFG_FILE)
else
#include from impl instead of softlink directory to make sure when run clean, it always get the righ definiations
-include $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)$(WL_SRC_BASE)/wl/config/wlconfig_lx_wl_dslcpe
-include $(WL_ROUTER_CFG_FILE)
#-include $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)/$(DHD_EXE)/dhd/config/dhdconfig_lx_dhd_dslcpe
-include $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)$(WL_SRC_BASE)/dslcpe_wlan_minor_version
endif

EXE  = wlmngr2  wlevt2
OBJS = wlmngr.o  wlsyscall.o wlutil.o  wlmngr_specialvar_handlers.o wlmngr_wps_var_handlers.o  wlmngr_restart_ctl.o
ifeq ($(strip $(BCA_CPEROUTER)),)
OBJS += wl.o wl_linux.o shutils.o wlconf.o defaults.o bcmwifi_channels.o nvparse.o
else
vpath %.c $(WIRELESS_DRIVER_PATH)/shared
OBJS += bcmutils.o
endif

PURE181=$(or $(strip $(BUILD_DM_PURE181)),$(strip $(BUILD_DM_DETECT)))

ifneq ($(strip $(BUILD_BRCM_CMS)),)
LIBS = -lcms_msg $(CMS_COMMON_LIBS) -lcms_dal -lcms_cli -lcms_msg $(CMS_COMMON_LIBS) -ldl $(CMS_CORE_LIBS)
ifneq ($(strip $(BUILD_PMD)),)
LIBS += -Wl,-rpath,$(BCM_FSBUILD_DIR)/private/lib:$(BCM_FSBUILD_DIR)/public/lib -L$(BCM_FSBUILD_DIR)/private/lib -lpmd -L$(BCM_FSBUILD_DIR)/public/lib -ljson-c
endif
else
LIBS = -ldl 
endif
ifeq ($(strip $(BCA_CPEROUTER)),)
LIBS += -lwlctl -lnvram -lpthread -lm  -lwlbcmshared
else
LIBS += -lnvram -lpthread -lm -lshared -L$(INSTALL_DIR)/usr/lib
CFLAGS+= -DDSLCPE_WLCONF_CMD -DBCA_CPEROUTER
ifneq ($(strip $(BUILD_BRCM_HOSTAPD)),)
CFLAGS += -DCONFIG_HOSTAPD  
LIBS += -lbcmcrypto -lnv2hapdcfg -L$(INSTALL_DIR)/usr/lib
EXE += wlhostapd
endif
ifneq ($(strip $(BUILD_BCM_WLAN_DPDCTL)),)
CFLAGS += -DBUILD_BCM_WLAN_DPDCTL
endif
endif

ifeq ($(CONFIG_AIRIQ),y)
CFLAGS += -DWL_AIR_IQ
endif

ifneq ($(strip $(BUILD_BSTREAM_IQOS)),)
ifneq ($(wildcard $(WIRELESS_DRIVER_PATH)/../components/vendor/trend/iqos),)
CFLAGS += -DWL_BSTREAM_IQOS
endif
endif

all dynamic: $(EXE) 

install: all
	install -m 755 $(EXE) $(INSTALL_DIR)/bin
ifneq ($(strip $(BCA_CPEROUTER)),)
	-ln -sf /usr/sbin/wl  $(INSTALL_DIR)/bin/wlctl
	-ln -sf /usr/sbin/dhd  $(INSTALL_DIR)/bin/dhdctl
endif

clean: generic_clean
	rm -f $(INSTALL_DIR)/bin/$(EXE) 
	-rm -f *.o
	-rm -f $(EXE)

ifneq ($(strip $(HNDAPPS_IN_COMP)),)
CFLAGS += -I$(HNDAPPS_DIR)/shared  -I$(HNDAPPS_DIR)/../wlioctl/include -I$(HNDAPPS_DIR)/../proto/include
CFLAGS += -I$(HNDAPPS_DIR)/../math/include 
endif

ifeq ($(CONFIG_FBT),y)
export CFLAGS  += -DWLHOSTFBT
endif
#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CFLAGS += $(SSP_TYP_COMPILER_OPTS)
LIBS += $(SSP_TYP_LIBS)
SSP_LIB_FLAGS = $(SSP_TYP_LIBS) $(SSP_LIB_INC)

#
# Private apps and libs are allowed to include header files from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I.\
                         -I../wlmngr  \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/libs/cms_core \
                         -I$(BUILD_DIR)/userspace/private/libs/cms_core/linux 

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

LIBS += -lwlcsm -lwlcsm_dm 
LIBS += -lbcm_flashutil -lbcm_boardctl -lgen_util -lbcm_util -lsys_util

ifeq ($(CONFIG_WBD),y)
CFLAGS += -DBCM_WBD -DMULTIAP
endif

ifeq ($(CONFIG_BCM_APPEVENTD),y)
export CFLAGS += -DBCM_APPEVENTD
endif

ifneq ($(strip $(BUILD_BRCM_CMS)),)
ifneq ($(strip $(PURE181)),)
LIBS += -lwlcsm_dm_tr181
CFLAGS += -DPURE181 -DSYS_DM_NAME='tr181'
else
ifneq ($(strip $(BUILD_UNIFIED_WLMNGR)),)
LIBS += -lwlcsm_dm_tr98 
CFLAGS += -DSYS_DM_NAME='tr98'
endif
endif
else
LIBS += -lwlcsm_dm_nocms
CFLAGS += -DNO_CMS -DSYS_DM_NAME='nocms'
endif
CFLAGS += -DDSLCPE_WLCSM_EXT
ALLOWED_INCLUDE_PATHS += -I$(WIRELESS_DRIVER_PATH)/wl/wlcsm_ext/include
ALLOWED_INCLUDE_PATHS += -I$(BUILD_DIR)/userspace/private/libs/wlcsm/include
ALLOWED_INCLUDE_PATHS += -I$(BUILD_DIR)/userspace/private/apps/wlan/wlcsm_dm/include

ifneq ($(strip $(WL_IMPL_PLUS)),)
CFLAGS+=-DWL_IMPL_PLUS
endif

ifeq ($(DSLCPE_CONFIG_MIPS),1)
CFLAGS+=-DCPU_MIPS
endif

CFLAGS += -DDSLCPE_DONGLEHOST_MFG

# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_LIB_DIRS := /lib:/lib/public:/lib/private
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


# treat all warnings as errors
#CUSTOM_CFLAGS += -Wno-error=unused-function -Wno-error=unused-but-set-variable -Werror -Wfatal-errors  # NEW_FORBID_WARNINGS

vpath %.c $(WIRELESS_DRIVER_PATH)/router/wlconf/
vpath %.c $(WIRELESS_DRIVER_PATH)/router/shared/
vpath %.c $(WIRELESS_DRIVER_PATH)/shared/bcmwifi/src/

WLCONF_CFLAGS = $(filter-out -DDSLCPE,$(CFLAGS))
WLCONF_CFLAGS += -DDSLCPE_WLCONF_MAIN

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
# 


wlmngr2: $(OBJS) wldaemon.o
	$(CC) -o $@ wldaemon.o $(OBJS) -s -Os -fomit-frame-pointer $(CMS_LIB_PATH) $(LIBS) -L$(LIBDIR) -L$(EXTRALIBDIR) -lc -lgcc_s 
	#$(CC) -o $@ wldaemon.o $(OBJS) -fomit-frame-pointer $(CMS_LIB_PATH) $(LIBS) -L$(LIBDIR) -L$(EXTRALIBDIR) -lc -lgcc_s 

ifneq ($(strip $(BUILD_BRCM_HOSTAPD)),)
wlhostapd: wlhostapd.o wlsyscall.o
	$(CC) -o $@ wlhostapd.o wlsyscall.o -s -Os -fomit-frame-pointer $(CMS_LIB_PATH) $(LIBS) -L$(LIBDIR) -L$(EXTRALIBDIR) -lc -lgcc_s 
endif

wlevt2: wlevt.o
ifneq ($(strip $(BUILD_BRCM_CMS)),)
	$(CC) -o $@ $^ -s -Os -fomit-frame-pointer $(CMS_LIB_PATH) -L$(LIBDIR) -L$(EXTRALIBDIR) -lwlcsm -lc -lgcc_s -lcms_msg $(CMS_COMMON_LIBS) $(SSP_LIB_FLAGS)
else
	$(CC) -o $@ $^ -s -Os -fomit-frame-pointer $(CMS_LIB_PATH) -L$(LIBDIR) -L$(EXTRALIBDIR) -lwlcsm -lc -lgcc_s  $(SSP_LIB_FLAGS)
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
