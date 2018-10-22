#
# To add an app, add it to APP_DIRS, and write the target rule in the
# last section of this Makefile

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

ifneq ($(strip $(BUILD_BRCM_CMS)),)
else 
BUILD_PURE181_PROFILES=y
endif

WL_ROUTER_CFG_FILE := $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)$(WL_SRC_BASE)/router/.config


#include from impl instead of softlink directory to make sure when run clean, it always get the righ definiations
-include $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)$(WL_SRC_BASE)/wl/config/wlconfig_lx_wl_dslcpe
-include $(WL_ROUTER_CFG_FILE)
#-include $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)/$(DHD_EXE)/dhd/config/dhdconfig_lx_dhd_dslcpe
-include $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)$(WL_SRC_BASE)/dslcpe_wlan_minor_version

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

WLLIB_APPS = nvram wlctl dhdctl
ifneq ($(strip $(BUILD_TR181_WLMNGR)),)
WLLIBS = wlcsm_dm
endif
#WLLIBS += nvram bcmwlcrypto bcmwlshared $(WLLIB_APPS)
WLLIBS += nvram bcmwlshared $(WLLIB_APPS)


ROUTER_SHARED_DIR = $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/shared
BCMCRYPTO_DIR = $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/bcmcrypto

WLANAPP_DIRS = util acsd nas epittcp lltd libupnp wps ses eapd wapi ias hspot_ap bsd ssd toad eventd aspmd

ifneq ($(strip $(BUILD_TR181_WLMNGR)),)
#WLANAPP_DIRS += 181wlmngr 
ifneq ($(strip $(BUILD_DM_DETECT)),)
WLANAPP_DIRS += wlmngr 
endif
else
#WLANAPP_DIRS += wlmngr 
endif


#
# We need to declare these directory names as phony targets,
# otherwise make will think the directories themselves are
# the targets and will never execute the cmds.
# See GNU Make, 4.6 
#

ifneq ($(strip $(WL_BASE)),)
.PHONY: setuplink $(WLLIBS) $(WLANAPP_DIRS)
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
clean:  wifi_apps_clean eapd_clean nas_clean
	-rm -rf src
	-rm -rf components
ifneq ($(strip $(BUILD_TR181_WLMNGR)),)
	$(MAKE) -C wlcsm_dm clean
endif
	$(foreach dir, $(WLANAPP_DIRS) $(WLLIB_APPS) $(ROUTER_SHARED_DIR) $(BCMCRYPTO_DIR), \
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


EXT_CPU_ARCH_NAME := $(subst \",,$(CONFIG_BCM_CPU_ARCH_NAME))
#"#
export EXT_CPU_ARCH_NAME

bcmwlcrypto:
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/libbcmcrypto -f Makefile.dslcpe  dynamic install

bcmwlshared: 
	if [ -f $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/shared/Makefile ]; then \
		$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/shared dynamic install; \
	elif [ -f $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/shared/Makefile.dslcpe ]; then \
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
#wireless nas command, a soft link directory		
#
ifneq ($(strip $(BUILD_NAS)),)
nas:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ $@ ; \
	elif [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	fi; \
	if [ -e $(@)/nas/Makefile.dslcpe ]; then \
	$(MAKE) -C $@/nas  -f Makefile.dslcpe install ;\
	fi
nas_clean:
	if [ -e nas/nas/Makefile.dslcpe ]; then \
	$(MAKE) -C nas/nas  -f Makefile.dslcpe clean;\
	fi
else
nas nas_clean:
	@echo "skipping $@ (not configured)"
endif

#
#eapd, a soft link directory
#
ifneq ($(strip $(BUILD_NAS)),)
eapd:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ $@ ; \
	elif [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	fi; \
	if [ -e $@/linux/Makefile.dslcpe ]; then \
	$(MAKE) -C $@/linux  -f Makefile.dslcpe install ;\
	fi
eapd_clean:
	if [ -e eapd/linux/Makefile.dslcpe ]; then \
	$(MAKE) -C eapd/linux  -f Makefile.dslcpe clean;\
	fi

else
eapd eapd_clean:
	@echo "skipping $@ (not configured)"
endif

#
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

wlmngr: 
	$(MAKE) -C $@ all install

181wlmngr:
	$(MAKE) -C $@ all install

util:
ifneq ($(PREBUILT_BCMBIN),1)
	$(MAKE) -C $@ all install
else
	-cp -f $(TOP)/wlan/util/$(PRBM_DIR)/nvramUpdate util/
	$(MAKE) -C $@ install
endif
#
#lltd, a soft link directory
#
ifneq ($(strip $(BUILD_LLTD)),)
lltd:
	rm -f $@; \
	if [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/$@ $@ ; \
	elif [ -e $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE)/router/$@ $@ ; \
	fi; \
	if [ -e $(@)/Makefile.dslcpe ]; then \
	$(MAKE) -C $@ $(BUILD_LLTD) -f Makefile.dslcpe install ;\
	fi
else
lltd:
	@echo "skipping $@ (not configured)"
endif

ifeq ($(CONFIG_MIPS),y)
export DSLCPE_CONFIG_MIPS=1
endif

ifeq ($(DSLCPE_WLCSM_EXT),1)
CFLAGS += -DDSLCPE_WLCSM_EXT
export DSLCPE_WLCSM_EXT=1
wlcsm_dm:
	$(MAKE) -C $@ install 
endif

setuplink: 
	if [ ! -d src ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_SRC_BASE) src ; \
	fi; \
	if [ ! -d components ]; then \
	   ln -s $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)$(WL_BASE)/components components ; \
	fi

wifi_apps: setuplink $(WLLIBS)
	@echo done

wifi_apps_clean:
	$(info "base dir is $(BASEDIR)")
	if [ -d src/router ]; then \
	$(MAKE) -i -C src/router -f Makefile.dslcpe clean; \
	fi

ifeq ($(BUILD_WLHSPOT),y)
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
	make -C usbdev/libusb-0.1.12 clean
	make TARGETENV=linuxmips -C usbdev/libusb-0.1.12
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

WLROUTER_WWW := $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/impl$(IMPL)$(WL_SRC_BASE)/router/www/broadcom
#WLROUTER_WWW = $(BASEDIR)/src/router/www/broadcom
WLROUTER_PAGES := $(wildcard $(WLROUTER_WWW)/*.asp)
WLROUTER_PAGES += $(wildcard $(WLROUTER_WWW)/*.js)
WLROUTER_PAGES += $(wildcard $(WLROUTER_WWW)/*.css)
WLROUTER_PAGES += $(wildcard $(WLROUTER_WWW)/*.gif)
WLROUTER_PAGES += $(wildcard $(WLROUTER_WWW)/*.jpeg)
WLROUTER_PAGES += $(wildcard $(WLROUTER_WWW)/*.jpg)

www_install:
	@mkdir -p  $(INSTALL_DIR)/webs/wlrouter
	@install -m 644 $(WLROUTER_PAGES) $(INSTALL_DIR)/webs/wlrouter
