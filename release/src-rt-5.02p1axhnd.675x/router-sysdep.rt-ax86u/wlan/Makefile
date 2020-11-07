#
# To add an app, add it to APP_DIRS, and write the target rule in the
# last section of this Makefile
#
export BCA_HNDROUTER=y

export BASEDIR=$(shell pwd)
export SRCDIR=$(BASEDIR)/src
# because we use symbol link, ../../ operation will make it to physical parents, so
# we need to change the directory to symbol link directory
export COMPDIR=$(BASEDIR)/components

default: all

WLAN_USERSPACE_CFG = 1

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common

# DSLCPE
export INSTALLDIR=$(INSTALL_DIR)
CFLAGS += $(SSP_TYP_COMPILER_OPTS)
LIBS += $(SSP_TYP_LIBS)  $(SSP_LIB_INC)
# end DSLCPE

export CMS_CORE_LIBS
export CMS_COMMON_LIBS
export BRCM_1905_TOPOLOGY_WEB_PAGE

ifneq ("$(wildcard $(KERNEL_DIR)/.config)","")
include $(KERNEL_DIR)/.config
endif
IMPL=$(strip $(shell readlink $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP) | sed 's/[^0-9]//g'))
ifeq ($(IMPL),)
# the symbolic link of bcm9$(BRCM_CHIP) has been removed/cleaned, set IMPL to an integer to avoid warning message
IMPL=0
endif
WL_BASE=$(shell if [ "$(IMPL)" -ge 22 ]; then echo /main; fi ;)
WL_SRC_BASE=$(shell if [ "$(IMPL)" -ge 22 ]; then echo /main/src; fi ;)
DHD_WL_EXE=$(shell if [ "$(IMPL)" -ge 22 ]; then echo dhd/src/wl/exe; else echo wl/exe; fi ;)
DHD_EXE=$(shell if [ "$(IMPL)" -ge 22 ]; then echo dhd/src; else echo dhd; fi ;)

export SRCBASE=$(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)/$(WL_SRC_BASE)
export TOP=$(SRCBASE)/router
export INSTALLDIR=$(INSTALL_DIR)
export CFLAGS += -DDSLCPE -DDSLCPE_ENDIAN

# contains e.g. WlGetDriverCfg.sh
export APPS_UTILS_DIR := $(COMPDIR)/apps/utils

ifneq ($(strip $(BUILD_BRCM_CMS)),)
# Wifi 1905 in WBD2 has not test CMS yet,disable it for now 
# Comment out it when enable CMS for 1905
export BUILD_1905_NOCMS=y
else 
BUILD_PURE181_PROFILES=y
endif

ifneq ($(strip $(HNDAPPS_IN_COMP)),)
WL_ROUTER_CFG_FILE := $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)/$(HNDAPPS_BASE)/.config
else
WL_ROUTER_CFG_FILE := $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)/main/src/router/.config
endif

#include from impl instead of softlink directory to make sure when run clean, it always get the righ definiations
-include $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)$(WL_SRC_BASE)/wl/config/wlconfig_lx_wl_dslcpe
-include $(WL_ROUTER_CFG_FILE)
#-include $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)/$(DHD_EXE)/dhd/config/dhdconfig_lx_dhd_dslcpe
-include $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)$(WL_SRC_BASE)/dslcpe_wlan_minor_version

ifneq ($(BUILD_HND_MFG),)
export BUILD_MFG := 1
export WLTEST := 1
endif

ifeq ($(EXT_DONGLE),y)
export EXT_DONGLE
endif

ifeq ($(DSLCPE_WLCSM_EXT),1)
CFLAGS += -DDSLCPE_WLCSM_EXT
export DSLCPE_WLCSM_EXT=1
endif

export DSLCPE_MK=1
# COMMENT:(JXU)- when switchin to impl20, no GNUmakefile is there, we always use Makefile.
#ifeq ($(shell if [ $(SVN_IMPL) -ge 20 ]; then echo 1; else echo 0; fi),1)
#WLCTL_MAKEFILE := GNUmakefile
#DHDCTL_MAKEFILE := GNUmakefile
#else
WLCTL_MAKEFILE := Makefile.dslcpe
DHDCTL_MAKEFILE := Makefile.dslcpe
#endif

DSLCPE_WLAN_VERSION=cpe$(BRCM_VERSION).$(BRCM_VERSION)$(BRCM_RELEASE).$(DSLCPE_WLAN_MINOR_VERSION)
CFLAGS += -DDSLCPE_WLAN_VERSION=\"$(DSLCPE_WLAN_VERSION)\"
CFLAGS += $(BRCM_WERROR_CFLAGS)

# WLMNGR2_APP = wlmngr2


WLLIB_APPS = nvram wlctl dhdctl
# WLLIBS += nvram bcmwlcrypto bcmwlshared bshared $(WLLIB_APPS)
WLLIBS += nvram bcmwlshared bshared $(WLLIB_APPS)


ROUTER_SHARED_DIR = $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/shared
BCMCRYPTO_DIR = $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/bcmcrypto

WLANAPP_DIRS = acsd epittcp libupnp wps ses wapi ias hspot_ap bsd ssd toad eventd aspmd appeventd utils dhd_monitor

ifeq ($(strip $(BRCM_IKOS)),)
ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
WLANAPP_DIRS += wlssk 
ifneq ($(strip $(BUILD_BRCM_CMS)),)
WLANAPP_DIRS += wlDiag
endif
export BUILD_BRCM_UNFWLCFG
else
WLANAPP_DIRS += $(WLMNGR2_APP)
# WLLIBS += wlcsm_dm
EXTRA_LDFLAGS +=  -lwlcsm
export EXTRA_LDFLAGS
endif
endif #BRCM_IKOS

WLANAPP_DIRS += util scripts
#
# We need to declare these directory names as phony targets,
# otherwise make will think the directories themselves are
# the targets and will never execute the cmds.
# See GNU Make, 4.6 
#

ifneq ($(strip $(WL_BASE)),)
.PHONY: setuplink $(WLLIBS) $(WLANAPP_DIRS) wifi_apps wifi_apps_clean
else
.PHONY: $(WLLIBS) $(WLANAPP_DIRS)
endif


$(WLANAPP_DIRS): $(WLLIBS)

ifneq ($(PREBUILT_BCMBIN),1)
ifneq ($(strip $(WL_BASE)),)
#when it is impl22 or above, it will instal www for wirless router. 
#all: setuplink $(WLANAPP_DIRS) www_install
all: setuplink $(WLANAPP_DIRS)
else
all: $(WLANAPP_DIRS)
endif
else
all:
	-cp -f $(TOP_PLATFORM)/wlan/nvram/$(PRBM_DIR)/libnvram.so nvram/
endif
install:
	@echo done


#
# Cleaning rules
# See example in GNU Make, 4.6
# (what a pain, upnp does not have a top level Makefile, so
# we have to take upnp out of APP_DIRS for the clean.)
#
clean:  wifi_apps_clean 
ifneq ($(strip $(HNDAPPS_IN_COMP)),)
ifneq ($(strip $(BCA_CPEROUTER)),)
	-rm -rf ./components/router/config/*bca-cperouter*
endif
else
ifneq ($(strip $(BCA_CPEROUTER)),)
	-rm -rf ./src/router/config/*bca-cperouter*
endif
endif
	-rm -rf src
	-rm -rf components
	$(MAKE) -C wlcsm_dm clean
	$(foreach dir, $(WLANAPP_DIRS) $(WLLIB_APPS) bshared $(ROUTER_SHARED_DIR) $(BCMCRYPTO_DIR), \
	  if [ -e $(dir) ]; then \
	   if [ -e $(dir)/Makefile.dslcpe ]; then \
	  $(MAKE) -C $(dir) -f Makefile.dslcpe clean; \
	  else \
	  $(MAKE) -C $(dir) clean; \
	  fi;  fi;\
	  if [ -L $(dir) ]; then rm -f $(dir); fi; )


#
all: $(WLANAPP_DIRS) wifi_apps

ifneq ($(findstring $(IMPL),$(NIGHTLY_IMPLS)),)
BUILD_NAS=
BUILD_WSC=
endif

ifeq ($(EXT_ACS),y)
CFLAGS += -DEXT_ACS
export EXT_ACS
export CONFIG_BCMDCS=y
endif

ifeq ($(EXT_WPS_VER2),y)
export EXT_WPS_VER2
endif

ifeq ($(EXT_WL11AC),y)
export EXT_WL11AC
endif

ifeq ($(EXT_BCMBSD),y)
export CONFIG_BCMBSD=y
endif

ifeq ($(EXT_SSD),y)
export CONFIG_BCMSSD=y
endif

ifeq ($(EXT_MFP),y)
export MFP=y
endif

ifeq ($(EXT_DSLCPE_WPS_SEC_CLONE),y)
export DSLCPE_WPS_SEC_CLONE=y
endif

ifeq ($(CONFIG_WBD),y)
export CFLAGS += -DBCM_WBD
endif

ifeq ($(CONFIG_FBT),y)
export CONFIG_FBT=y
export CFLAGS  += -DWLHOSTFBT
endif


EXT_CPU_ARCH_NAME := $(subst \",,$(CONFIG_BCM_CPU_ARCH_NAME))
#"#
export EXT_CPU_ARCH_NAME

bcmwlcrypto:
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/libbcmcrypto -f Makefile.dslcpe  dynamic install

bcmwlshared: 
	if [ -f $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/shared/Makefile.dslcpe ]; then \
		$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/shared -f Makefile.dslcpe dynamic install; \
	fi

##############################################################
# 
# Rules for inidividual apps
# apps can be compiled or not depending on the profile (which
# is modified via make menuconfig)
#
##############################################################

#
#wireless wl command, a soft link directory
#
ifneq ($(strip $(BUILD_WLCTL)),)
export BUILD_WLCTL_SHLIB=1
wlctl:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ $@ ; \
	elif [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/$(DHD_WL_EXE) ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/$(DHD_WL_EXE) $@ ; \
	fi; \
	if [ -e $(@)/$(WLCTL_MAKEFILE) ]; then \
	$(MAKE) -f $(WLCTL_MAKEFILE) -C $@ $(BUILD_WLCTL) ;\
	fi
else
wlctl:
	@echo "skipping $@ (not configured)"
endif

#
#wireless dhd command, a soft link directory
#
ifneq ($(strip $(BUILD_DHDCTL)),)
dhdctl:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ $@ ; \
	elif [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/$(DHD_EXE)/dhd/exe ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/$(DHD_EXE)/dhd/exe $@ ; \
	fi; \
	if [ -e $(@)/$(DHDCTL_MAKEFILE) ]; then \
	$(MAKE) -f $(DHDCTL_MAKEFILE) -C $@ $(BUILD_DHDCTL) ;\
	fi
else
dhdctl:
	@echo "skipping $@ (not configured)"
endif

#	
#wireless dhd monitor, a soft link directory
#
export BUILD_DHD_MONITOR=y
ifneq ($(strip $(BUILD_DHD_MONITOR)),)
dhd_monitor:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ $@ ; \
	elif [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	fi; \
	if [ -e $(@)/Makefile.dslcpe ]; then \
	$(MAKE) -C $@ -f Makefile.dslcpe install ;\
	fi
else
dhd_monitor:
	@echo "skipping $@ (not configured)"
endif

#acsd, a soft link directory
#
ifneq ($(strip $(EXT_ACS)),)
acsd:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	if [ -e $@/Makefile.dslcpe ]; then \
	   $(MAKE) -C $@ -f Makefile.dslcpe install;\
	fi; \
	fi
else
acsd:
	@echo "skipping $@ (not configured)"
endif

#
#bsd, a soft link directory
#
ifneq ($(strip $(EXT_BCMBSD)),)
bsd:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/bsd ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/bsd bsd ; \
	fi; \
	if [ -e $@/Makefile.dslcpe ]; then \
	$(MAKE) -C $@ -f Makefile.dslcpe install ;\
	fi
else
bsd:
	@echo "skipping $@ (not configured)"
endif


#
#epi_ttcp
#
export BUILD_EPITTCP=dynamic
ifneq ($(strip $(BUILD_EPITTCP)),)
epittcp:
	$(MAKE) -C $@ $(BUILD_EPITTCP)
else
epittcp:
	@echo "skipping $@ (not configured)"
endif


#
#nvram emulation
#
export BUILD_NVRAM=dynamic
ifneq ($(strip $(BUILD_NVRAM)),)
nvram:
ifneq ($(PREBUILT_BCMBIN),1)
	$(MAKE) -C $@ $(BUILD_NVRAM)
else
	-cp -f $(TOP_PLATFORM)/wlan/nvram/$(PRBM_DIR)/libnvram.so nvram/
	-cp -f $(TOP_PLATFORM)/wlan/nvram/$(PRBM_DIR)/nvram nvram/
	$(MAKE) -C $@ all
endif
else
nvram:
	@echo "skipping $@ (not configured)"
endif

ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
unfnvram:
	$(MAKE) -C $@ $(BUILD_BRCM_UNFWLCFG)
else
unfnvram:
	@echo "skipping $@ (not configured)"
endif

scripts: 
	$(MAKE) -C $@ install

$(WLMNGR2_APP): wlcsm_dm
	$(MAKE) -C $@ all install


wlssk:
	$(MAKE) -C $@ all install

util:
ifneq ($(PREBUILT_BCMBIN),1)
	$(MAKE) -C $@ all install
else
	-cp -f $(TOP_PLATFORM)/wlan/util/$(PRBM_DIR)/nvramUpdate util/
	$(MAKE) -C $@ install
endif

wlDiag:
	$(MAKE) -C $@ all install

ifeq ($(CONFIG_MIPS),y)
export DSLCPE_CONFIG_MIPS=1
endif

ifeq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
CFLAGS += -DDSLCPE_WLCSM_EXT
export DSLCPE_WLCSM_EXT=1
endif
wlcsm_dm:
	# "make regenerate_dm_files" first to avoid parallel compiling issue.
	$(MAKE) -C $@ regenerate_dm_files
	$(MAKE) -C $@ install 

setuplink: 
	if [ ! -d src ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/impl$(IMPL)$(WL_SRC_BASE) src ; \
	fi; \
	if [ ! -d components ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/impl$(IMPL)$(WL_BASE)/components components ; \
	fi

wifi_apps: setuplink $(WLLIBS)
	if [ -e src/router/Makefile.dslcpe ]; then \
	$(MAKE) -C src/router -f Makefile.dslcpe install; \
	fi
wifi_apps_clean:
	$(info "base dir is $(BASEDIR)")
	if [ -e src/router/Makefile.dslcpe ]; then \
	$(MAKE) -i -C src/router -f Makefile.dslcpe clean; \
	fi

ifeq ($(BUILD_WLHSPOT),y)

export CFLAGS += -DNAS_GTK_PER_STA -DHSPOT_OSEN
export ICONPATH := /webs/wlrouter/hspot
export CFLAGS += -DICONPATH=\"$(ICONPATH)\"

hspot_ap: $(WLLIBS)
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	fi; \
	if [ -e $@/Makefile.dslcpe ]; then \
	$(MAKE) -C $@ -f Makefile.dslcpe install ;\
	fi
else
hspot_ap:
	@echo "skipping $@ (not configured)"
endif
#wps, a soft link directory
#
ifneq ($(strip $(BUILD_WSC)),)
export BUILD_WSC_CONFIG_UPNP_DISABLE=0
# BUILD_WSC_CONFIG_UPNP_DISABLE=1 to disable UPNP w/ AP config mode
# BUILD_WSC_CONFIG_UPNP_DISABLE=0 should be 0 for WiFi test
wps: libupnp
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ $@ ; \
	fi; \
	if [ -e $@/Makefile.dslcpe ]; then \
	$(MAKE) -C $@ -f Makefile.dslcpe install ;\
	fi
else
wps:
	@echo "skipping $@ (not configured)"
endif

ifneq ($(strip $(BUILD_WSC)),)
libupnp:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ $@ ; \
	elif [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	fi; \
	if [ -e $@/Makefile.dslcpe ]; then \
	$(MAKE) -C $@ $(BUILD_WSC) -f Makefile.dslcpe install; \
	fi
else
libupnp:
	@echo "skipping $@ (not configured)"
endif

#
#ses, a soft link directory
#
ifneq ($(strip $(BUILD_SES)),)
ses:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ $@ ; \
	fi; \
	if [ -e $(@)/Makefile ]; then \
	$(MAKE) -C $@ $(BUILD_SES) install;\
	fi
else
ses:
	@echo "skipping $@ (not configured)"
endif


#
#ssd, a soft link directory
#
ifneq ($(strip $(EXT_SSD)),)
ssd:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	   if [ -e $@/Makefile.dslcpe ]; then \
	   $(MAKE) -C $@ -f Makefile.dslcpe install ;\
	   fi; \
	fi
else
ssd:
	@echo "skipping $@ (not configured)"
endif

ifneq ($(strip $(EXT_TOAD)),)
toad:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	   if [ -e $@/Makefile.dslcpe ]; then \
	   $(MAKE) -C $@ -f Makefile.dslcpe install ;\
	   fi; \
	fi
else
toad:
	@echo "skipping $@ (not configured)"
endif

#
# eventd
#
ifeq ($(strip $(CONFIG_BCMEVENTD)),y)
eventd:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	   if [ -e $@/Makefile.dslcpe ]; then \
			$(MAKE) -C $@ -f Makefile.dslcpe install ;\
	   fi \
	fi
else
eventd:
	@echo "skipping $@ (not configured)"
endif

#
# aspmd
#
ifeq ($(strip $(CONFIG_BCMASPMD)),y)
aspmd:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	if [ -e $@/Makefile.dslcpe ]; then \
		$(MAKE) -C $@ -f Makefile.dslcpe install ;\
	fi \
	fi
else
aspmd:
	@echo "skipping $@ (not configured)"
endif

#
# bshared
#
ifeq ($(strip $(CONFIG_BSHARED)),y)
bshared: bcmwlshared 
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	if [ -e $@/Makefile.dslcpe ]; then \
		$(MAKE) -C $@ -f Makefile.dslcpe install ;\
	fi \
	fi
else
bshared:
	@echo "skipping $@ (not configured)"
endif


#
# appeventd
#
ifeq ($(strip $(CONFIG_BCM_APPEVENTD)),y)
appeventd:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	if [ -e $@/Makefile.dslcpe ]; then \
		$(MAKE) -C $@ -f Makefile.dslcpe install ;\
	fi \
	fi
else
appeventd:
	@echo "skipping $@ (not configured)"
endif

#
# utils
#
utils:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	if [ -e $@/Makefile.dslcpe ]; then \
	   $(MAKE) -C $@ -f Makefile.dslcpe install ;\
	fi \
	fi


#
#
#usbdev apps for Wlan USB driver
#
ifneq (,$(findstring usb, $(strip $(WLBUS))))
#ifeq ($(strip $(BRCM_DRIVER_WIRELESS_USBAP)),y)
usbdev:
	@echo "copy bcmdl utilities to " $(INSTALL_DIR)/sbin
	rm -f $@
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ $@ ; \
	fi

	cd usbdev; tar xzkf libusb-0.1.12/libusb-0.1.12.tgz 2>/dev/null; cd ..
	$(MAKE) -C usbdev/libusb-0.1.12 clean
	$(MAKE) TARGETENV=linuxmips -C usbdev/libusb-0.1.12
	install -m 755 usbdev/libusb-0.1.12/.libs/libusb-0.1.so.4 $(INSTALL_DIR)/lib

#	make -C usbdev/usbdl clean
#	make -C usbdev/usbdl
	install -m 755 usbdev/usbdl/bcmdl $(INSTALL_DIR)/bin
	cp -f $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/dongle/rte/wl/builds/4322-bmac/rom-ag/rtecdc.trx $(INSTALL_DIR)/rom/rom/etc/wlan
else
usbdev:
	@echo "skipping $@ (not configured)"
endif

#
#wapi, a soft link directory
#
ifeq ($(BRCM_WAPI),y)
wapi:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	fi; \
	if [ -e $@/Makefile.dslcpe ]; then \
	$(MAKE) -C $@ $(BUILD_WAPI) -f Makefile.dslcpe install ;\
	fi
else
wapi:
	@echo "skipping $@ (not configured)"
endif

#
#ias, a soft link directory
#
ifeq ($(BRCM_WAPI),y)
ias:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/wapi ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/wapi ias ; \
	fi; \
	if [ -e ias/Makefile.dslcpe ]; then \
	$(MAKE) -C $@ $(BUILD_WAPI) -f Makefile.dslcpe install ;\
	fi
else
ias:
	@echo "skipping $@ (not configured)"
endif

ifneq ($(strip $(HNDAPPS_IN_COMP)),)
WLROUTER_WWW := $(HNDAPPS_DIR)/www/broadcom
ifneq ($(strip $(BUILD_BSTREAM_IQOS)),)
IQOS_ICONS   := $(HNDAPPS_DIR)/../apps/iqos/www/icons
endif
else
WLROUTER_WWW := $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)$(WL_SRC_BASE)/router/www/broadcom
endif

mogrified_www := $(wildcard $(WLROUTER_WWW)/.mogrify)
ifneq ($(strip $(mogrified_www)),)
WLROUTER_WWW := $(WLROUTER_WWW)/.mogrify
endif


WLROUTER_PAGES := $(wildcard $(WLROUTER_WWW)/*.asp)
WLROUTER_PAGES += $(wildcard $(WLROUTER_WWW)/*.js)
WLROUTER_PAGES += $(wildcard $(WLROUTER_WWW)/*.css)
WLROUTER_PAGES += $(wildcard $(WLROUTER_WWW)/*.gif)
WLROUTER_PAGES += $(wildcard $(WLROUTER_WWW)/*.jpeg)
WLROUTER_PAGES += $(wildcard $(WLROUTER_WWW)/*.jpg)
WLROUTER_PAGES += $(wildcard $(WLROUTER_WWW)/*.png)
ifneq ($(strip $(BUILD_BSTREAM_IQOS)),)
WLROUTER_PAGES += $(wildcard $(IQOS_ICONS)/*.png)
endif

www_install:
ifeq ($(strip $(BRCM_IKOS)),)
	@mkdir -p  $(INSTALL_DIR)/webs/wlrouter
	@rm -rf $(INSTALL_DIR)/www
	@install -m 644 $(WLROUTER_PAGES) $(INSTALL_DIR)/webs/wlrouter
	@install -m 644 $(CURR_DIR)/$(WLMNGR2_APP)/style.css $(INSTALL_DIR)/webs/wlrouter
	@install -m 644 $(CURR_DIR)/$(WLMNGR2_APP)/visstyle.css $(INSTALL_DIR)/webs/wlrouter
else
	@echo "No www for ikos"
endif

