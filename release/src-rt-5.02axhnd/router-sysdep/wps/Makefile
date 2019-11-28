#
# Broadcom Linux Router WPS Makefile
#
# Copyright 2019 Broadcom
#
# This program is the proprietary software of Broadcom and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
# WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
# THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use
# all reasonable efforts to protect the confidentiality thereof, and to
# use this information only in connection with your use of Broadcom
# integrated circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
# REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
# OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
# DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
# NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
# ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
# CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
# OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
# BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
# SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
# IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
# IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
# ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
# OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
# NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
# <<Broadcom-WL-IPTag/Proprietary:>>
#
# $Id: Makefile 772949 2019-03-07 23:04:06Z $
#

REBUILD = $(shell if [ -f "prebuilt/wps_monitor" ]; then echo 0; else echo 1; fi)

ifneq (true,$(parallel))
  .NOTPARALLEL:
endif

# Build type
export BLDTYPE = release
#export BLDTYPE = debug

include $(TOP)/.config

ifeq ($(CONFIG_STBAP),y)
CONFIG_WL_USE_APSTA=y
else
# linux kernel config
-include $(LINUXDIR)/.config
endif

ifneq ($(BCA_HNDROUTER),)
# 4.1 kernel config does not have CONFIG_WL_USE_xxx so force it to APSTA to enable all functionality
CONFIG_WL_USE_APSTA=y
endif

export WPSSRC = $(SRCBASE)/../components/apps/wps

# Get wireless driver support mode
ifneq ($(CONFIG_WL_USE_STA)$(CONFIG_WL_USE_STA_ONCHIP_G)$(CONFIG_WL_USE_APSTA)$(CONFIG_WL_USE_APSTA_ONCHIP_G),)
  ifneq ($(CONFIG_WL_USE_STA)$(CONFIG_WL_USE_STA_ONCHIP_G),)
    export WL_MODE = sta
    export WPS_CONF_FILE = wpsconfig_router_sta
    WPS_LIB_FILE = wps_sta
  else
    export WL_MODE = apsta
    export WPS_CONF_FILE = wpsconfig_router_apsta
    WPS_LIB_FILE = wps_apsta
  endif
else
  export WL_MODE = ap
  export WPS_CONF_FILE = wpsconfig_router_ap
  WPS_LIB_FILE = wps_ap
endif

# Include wps common make file
include $(WPSSRC)/common/config/$(WPS_CONF_FILE)
ifneq ($(CONFIG_WFI),y)
WPS_WFI = 0
endif
ifneq ($(CONFIG_LIBUPNP),y)
WPS_UPNP_DEVICE = 0
endif
ifeq ($(CONFIG_NFC),y)
export WPS_NFC_DEVICE = 1
endif

ifeq ($(CONFIG_BCM47XX),y)
WPS_LED = 1
WPS_HWPB = 1
CFLAGS += -DWPS_LED -DWPS_HWPB
endif

include $(WPSSRC)/common/config/wps.mk
EXTRA_CFLAGS = $(WPS_FLAGS)

# Generate list of supported variants
VLIST  = AP STA APSTA

# Set suffix and config based on the selected variant
WPS_SUFFIX      := $(strip $(foreach v,$(VLIST),$(if $(CONFIG_WL_USE_$(v)),$(v))))

# Module file names use lowercase of the suffix
wps_suffix      := $(shell echo "$(WPS_SUFFIX)" | tr A-Z a-z)

# Always do variants build
wpssubdirs := $(addprefix wps_,$(shell echo "$(VLIST)" | tr A-Z a-z))

#
# Set CFLAGS
#
CFLAGS += -DWPS_AP_M2D

# Enable WWTP option cause the security hole described in WSC 2.0 specification section 4.3.1
# But WPS 2 Test plan 4.1.13 still need this feature support.
CFLAGS += -DWPS_ADDCLIENT_WWTP

CFLAGS += -Wall -Wnested-externs -D_REENTRANT -D__linux__
#CFLAGS += -Werror

ifeq ($(BLDTYPE), debug)
CFLAGS += -g
endif

# trunk uses bcmcrypto component
CFLAGS += -DBCMCRYPTO_COMPONENT

CFLAGS += -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/../components/shared -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/common/include -I$(WPSSRC)/common/include	\
	-I$(TOP)/libbcm -I$(TOP_PLATFORM)/eapd	\
	-I$(TOP)/shared -I$(WPSSRC)/brcm_apps/include	\
	${WLAN_ComponentIncPathA}

CFLAGS += -I$(SRCBASE)/../components/router/shared -I$(SRCBASE)/shared/bcmwifi/include -I$(SRCBASE)/../components/router-sysdep/bcmdrv/include
CFLAGS += -I$(SRCBASE)/../components/bcmcrypto/include -I$(SRCBASE)/../components/router-sysdep/libupnp$(BCMEX)$(EX7)/include

ifneq ($(strip $(BCA_HNDROUTER)),)
CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
endif

LDFLAGS += $(EXTRA_LDFLAGS)

WPSM_LIBS = -lnvram -lshared -lbcmcrypto -lm -lpthread
ifeq ($(WPS_HWPB), 1)
ifeq ($(CONFIG_LIBBCMSTB),y)
WPSM_LIBS += -lbcmstb
else
WPSM_LIBS += -lbcm
endif
endif

ifeq ($(CONFIG_LIBBCMSTB),y)
CFLAGS  += -DUSE_LIBBCMSTB
CFLAGS	+= -I$(SRCBASE)/router/libbcmstb
ifeq ($(CONFIG_GPIO_USE_NEXUS),y)
ifeq ("$(NEXUS_TOP)","")
$(error NEXUS_TOP is not defined)
endif
ifeq ("$(NEXUS_PLATFORM)","")
$(error NEXUS_PLATFORM is not defined)
endif
CFLAGS += -L$(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/nexus/bin
WPSM_LIBS += -lnexus -lpthread
endif
endif

# Set link libraries search path
export LIBPATH  = -L$(TOP)/libbcmcrypto -L$(INSTALLDIR)/libbcmcrypto/usr/lib
export LIBPATH += -L$(TOP)/libbcm -L$(INSTALLDIR)/libbcm/usr/lib
export LIBPATH += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib
export LIBPATH += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -L$(INSTALLDIR)/nvram/usr/lib ${EXTRA_NV_LDFLAGS}
export LIBPATH += -L$(TOP_PLATFORM)/wps$(BCMEX)$(EX7)/lib
export LIBPATH += -L$(TOP_PLATFORM)/wps$(BCMEX)$(EX7)/prebuilt
#ifeq ($(WPS_UPNP_DEVICE),1)
# Always use LIBUPNP
export LIBPATH += -L$(TOP_PLATFORM)/libupnp$(BCMEX)$(EX7) -L$(INSTALLDIR)/libupnp/usr/lib
ifneq ($(CMWIFI),)
WPSM_LIBS += -lwlupnp
else
WPSM_LIBS += -lupnp
endif
CFLAGS += -I$(TOP_PLATFORM)/libupnp$(BCMEX)$(EX7)/include -I$(WPSSRC)/brcm_apps/upnp/WFADevice
#endif

ifeq ($(WPS_NFC_DEVICE),1)
export LIBPATH += -L$(TOP)/nfc -L$(INSTALLDIR)/nfc/usr/lib
WPSM_LIBS += -lnsa -lipcnsa -lpthread
CFLAGS += -I$(SRCBASE)/wps/brcm_apps/nfc
CFLAGS += -I$(BASEDIR)/components/apps/nfc/3rdparty/embedded/nsa_examples/linux/libnsa/include
CFLAGS += -include $(BASEDIR)/components/apps/nfc/3rdparty/embedded/nsa_examples/linux/libnsa/include/buildcfg.h
endif

ifeq ($(RTCONFIG_AMAS),y)
CFLAGS += -DAMAS
endif

WPS_OSL := $(WPS_ROUTERHALFILES)
WPS_OSL += brcm_apps/linux/wps_linux_main.c

WPS_OSL += brcm_apps/linux/wps_linux_osl.c
ifneq ($(WL_MODE), ap)
WPS_OSL += brcm_apps/linux/wps_enr_linux_osl.c
endif

WPS_OSLOBJS := $(foreach file, $(WPS_OSL), \
	 $(patsubst %.c,obj/wps/%.o,$(file)))

WPS_DIRS := $(foreach file, $(WPS_OSLOBJS), \
	 $(dir $(file)))

WPS_DIRLIST = $(sort $(WPS_DIRS))

 ifneq ($(wildcard ./prebuilt),)
 all: osl
	cp prebuilt/wps_monitor ./
 else
 all: mkversion sta_build_check dirs variants_build osl wps_monitor
 endif

 osl:
	 mkdir -p $(WPS_DIRLIST)
	 @echo "==>$(WPS_CONF_FILE)"

 ifneq ($(wildcard ./prebuilt),)
 objs: | dirs
 else
 objs: variants_build | dirs
 endif
	+$(MAKE) parallel=true $(WPS_OSLOBJS)

$(WPS_OSLOBJS) : obj/wps%.o: $(addprefix $(WPSSRC),%.c)
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $<


# Variants build rules
variants_build: $(patsubst %, _dir_%, $(wpssubdirs))
$(patsubst %, _dir_%, $(wpssubdirs)):
	$(if $(wildcard $(patsubst _dir_%, %, $@)),+$(MAKE) -C $(patsubst _dir_%, %, $@))

# Set link libraries search path
export LIBPATH += -L. -L./prebuilt


sta_build_check:
ifeq ($(WL_MODE),sta)
ifeq ($(WPS_UPNP_DEVICE),1)
	@echo "ERROR: UPNP_WFA_DEVICE is not supported on STA mode"
	@exit 1
endif
endif
	@true


dirs :
	mkdir -p lib

mkversion :
	$(TOP_PLATFORM)/wps/mkversion.sh $(WPSSRC)/common/include/version.h.in $(WPSSRC)/common/include/wps_version.h "$(PROD_TAG)"

# target
# force compile wps_monitor in case the library changed.
wps_monitor : objs FORCE
	$(CC) $(CFLAGS) $(LIBPATH) $(WPS_OSLOBJS) -l$(WPS_LIB_FILE) $(WPSM_LIBS) $(LDFLAGS) -o $@

FORCE:


clean:
	find . \( -name '*.[od]' -o -name '*.tmp' \) -type f -print | xargs rm -f
	find -name obj | xargs rm -rf
	rm -rf lib
ifeq ($(REBUILD),1)
	find -name wps_monitor | xargs rm -f
endif

# install
install: all
	install -d $(INSTALLDIR)/wps/bin
ifeq ($(REBUILD),0)
	install prebuilt/wps_monitor $(INSTALLDIR)/wps${BCMEX}$(EX7)/bin
else
	install wps_monitor $(INSTALLDIR)/wps/bin
endif
	$(STRIP) $(INSTALLDIR)/wps/bin/wps_monitor

.PHONY: all clean install libs dirs osl
