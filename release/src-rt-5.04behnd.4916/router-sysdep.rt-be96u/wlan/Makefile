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
PREBUILT_BCMBIN = 0

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common
include $(BUILD_DIR)/make.wlan

# DSLCPE
export INSTALLDIR=$(INSTALL_DIR)
CFLAGS += $(SSP_TYP_COMPILER_OPTS)
LIBS += $(SSP_TYP_LIBS)  $(SSP_LIB_INC)
# end DSLCPE

export CMS_CORE_LIBS
export CMS_COMMON_LIBS
export BRCM_1905_TOPOLOGY_WEB_PAGE

WL_BASE=/main
WL_SRC_BASE=/main/src
DHD_WL_EXE=dhd/src/wl/exe
DHD_EXE=dhd/src

export SRCBASE=$(WIRELESS_DRIVER_PATH)
export TOP := $(WIRELESS_IMPL_PATH)/$(HNDAPPS_BASE)
export ALLSRCBASE :=$(WIRELESS_IMPL_PATH)
export WLSRCBASE :=$(WIRELESS_DRIVER_PATH)
export INSTALLDIR=$(INSTALL_DIR)
export CFLAGS += -DDSLCPE $(USR_CFLAGS)

export TOP=$(SRCBASE)/router
CFLAGS += -DWL_DEFAULT_NUM_SSID=$(BRCM_DEFAULT_NUM_MBSS)

# contains e.g. WlGetDriverCfg.sh
export APPS_UTILS_DIR := $(COMPDIR)/apps/utils

ifneq ($(strip $(BUILD_BRCM_CMS)),)
# Wifi 1905 in WBD2 has not test CMS yet,disable it for now 
# Comment out it when enable CMS for 1905
export BUILD_1905_NOCMS=y
else 
BUILD_PURE181_PROFILES=y
endif

WL_ROUTER_CFG_FILE := $(WIRELESS_IMPL_PATH)/$(HNDAPPS_BASE)/.config

ifneq ($(strip $(BCA_CPEROUTER)),)
-include  $(WL_ROUTER_CFG_FILE)
endif

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


WLLIB_APPS = wlctl dhdctl
WLLIBS += bcmwlshared bshared $(WLLIB_APPS)


ROUTER_SHARED_DIR = $(WIRELESS_DRIVER_PATH)/router/shared
BCMCRYPTO_DIR = $(WIRELESS_DRIVER_PATH)/bcmcrypto

WLANAPP_DIRS = acsd epittcp libupnp wps ses wapi ias hspot_ap bsd ssd toad eventd aspmd appeventd utils dhd_monitor

ifeq ($(strip $(BRCM_IKOS)),)

ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
# WLANAPP_DIRS += wlssk
endif
export BUILD_BRCM_UNFWLCFG

ifneq ($(strip $(BUILD_BRCM_CMS)),)
WLANAPP_DIRS += wlDiag
endif

ifneq ($(strip $(BUILD_WLDATAELD)),)
WLANAPP_DIRS += wldataeld
endif

endif #BRCM_IKOS

WLANAPP_DIRS += util scripts nvram

ifneq ($(strip $(BUILD_RDKWIFI)),)

ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
ifneq ($(strip $(BUILD_BRCM_BDK)),)
# Remove all mdm_cbk_* and bcm_*_hal libs from CMS_CORE_LIBS.  The local
# Makefile will specify which mdm_cbk and hal libs it needs.
MDM_CORE_LIBS := $(patsubst -lmdm_cbk_%,,$(CMS_CORE_LIBS))
MDM_CORE_LIBS := $(patsubst -lbcm_%_hal,,$(MDM_CORE_LIBS))
MDM_CORE_LIBS += -lmdm_cbk_wifi
else
MDM_CORE_LIBS := $(CMS_CORE_LIBS)
endif
UNFCFG_LIBS += -lcms_msg $(CMS_COMMON_LIBS) -lcms_dal -lcms_msg $(CMS_COMMON_LIBS) $(MDM_CORE_LIBS)
UNFCFG_LIBS += -lstaged -ljson-c -lpthread -lbcm_util -lsys_util
 ifneq ($(strip $(BUILD_CUSTOMER)),)
  UNFCFG_LIBS += -lwlmdm_customer
 else
  UNFCFG_LIBS += -lwlmdm -lpcre2-8
 endif
endif

WLANAPP_DIRS += cmwifi hapdconf

ifneq ($(strip $(RDK_BUILD)),)
WLANAPP_DIRS += wifi_rdk_initd
endif

cmwifi:
ifeq ($(strip $(RDK_BUILD)),)
	$(MAKE) -C $@ all install
else
	@echo "No need to build cmwifi"
endif


hapdconf:
ifneq ($(strip $(BUILD_BRCM_HOSTAPD)),)
	$(MAKE) -C ./components/router/hapdconf install BUILD_RDKWIFI=1  CONFIG_HOSTAPD=y EXTRA_LDFLAGS="$(UNFCFG_LIBS)"
else
	$(MAKE) -C ./components/router/hapdconf install BUILD_RDKWIFI=1 EXTRA_LDFLAGS="$(UNFCFG_LIBS)"
endif

wifi_rdk_initd:
	$(MAKE) -C ./wifi_rdk_initd -f Bcmbuild.mk 


endif
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


$(WLANAPP_DIRS): $(WLLIBS) wlcsmdbg_tools

wlcsmdbg_tools: 
ifneq ($(strip $(WLCSM_DEBUG_TRACE)),)
	$(MAKE) -C ./nvram wlcsmdbg_tools
endif

ifneq ($(PREBUILT_BCMBIN),1)
ifneq ($(strip $(WL_BASE)),)
#when it is impl22 or above, it will instal www for wirless router. 
all: setuplink $(WLANAPP_DIRS)

www_install: | setuplink $(WLANAPP_DIRS) 

$(WLANAPP_DIRS): | setuplink 
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
ifneq ($(strip $(BCA_CPEROUTER)),)
	-rm -rf ./components/router/config/*bca-cperouter*
endif
	-rm -rf src
	-rm -rf components
ifneq ($(strip $(BUILD_RDKWIFI)),)
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/wlcsm BUILD_RDKWIFI_LIB=1 clean
	-rm -rf ./cmwifi/wifi_hal
endif
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
	$(MAKE) -C $(WIRELESS_DRIVER_PATH)/router/libbcmcrypto -f Makefile.dslcpe  dynamic install

bcmwlshared: 
	if [ -f $(WIRELESS_DRIVER_PATH)/router/shared/Makefile.dslcpe ]; then \
		$(MAKE) -C $(WIRELESS_DRIVER_PATH)/router/shared -f Makefile.dslcpe dynamic install; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/$@ $@ ; \
	elif [ -e $(WIRELESS_IMPL_PATH)/$(DHD_WL_EXE) ]; then \
	   ln -s $(WIRELESS_IMPL_PATH)/$(DHD_WL_EXE) $@ ; \
	fi; \
	if [ -e $(@)/$(WLCTL_MAKEFILE) ]; then \
	$(MAKE) -f $(WLCTL_MAKEFILE) -C $@ $(BUILD_WLCTL) ;\
	fi
else
wlctl:
	echo "skipping $@ (not configured)"
endif

#
#wireless dhd command, a soft link directory
#
ifneq ($(strip $(BUILD_DHDCTL)),)
dhdctl:
	rm -f $@; \
	if [ -e $(WIRELESS_DRIVER_PATH)/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/$@ $@ ; \
	elif [ -e $(WIRELESS_IMPL_PATH)/$(DHD_EXE)/dhd/exe ]; then \
	   ln -s $(WIRELESS_IMPL_PATH)/$(DHD_EXE)/dhd/exe $@ ; \
	fi; \
	if [ -e $(@)/$(DHDCTL_MAKEFILE) ]; then \
	$(MAKE) -f $(DHDCTL_MAKEFILE) -C $@ $(BUILD_DHDCTL) ;\
	fi
else
dhdctl:
	echo "skipping $@ (not configured)"
endif

#	
#wireless dhd monitor, a soft link directory
#
export BUILD_DHD_MONITOR=y
ifneq ($(strip $(BUILD_DHD_MONITOR)),)
dhd_monitor:
	rm -f $@; \
	if [ -e $(WIRELESS_DRIVER_PATH)/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/$@ $@ ; \
	elif [ -e $(WIRELESS_DRIVER_PATH)/router/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/router/$@ $@ ; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/router/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/router/$@ $@ ; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/router/bsd ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/router/bsd bsd ; \
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

scripts: 
	$(MAKE) -C $@ install


wlssk:
	$(MAKE) -C $@ all install

wldataeld:
	$(MAKE) -C $@ all install

nvram:
ifneq ($(PREBUILT_BCMBIN),1)
	# $(MAKE) -C $@ all install
	$(MAKE) -C $@ dynamic install
else
	-cp -f $(TOP_PLATFORM)/wlan/nvram/$(PRBM_DIR)/libnvram.so nvram/
	$(MAKE) -C $@ install
endif

wlDiag:
	$(MAKE) -C $@ all install

ifeq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
CFLAGS += -DDSLCPE_WLCSM_EXT
export DSLCPE_WLCSM_EXT=1
endif

setuplink: 
	mkdir -p  $(INSTALL_DIR)/webs
	if [ ! -d src ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH) src ; \
	fi; \
	if [ ! -d components ]; then \
	   ln -s $(COMPON_WIRELESS_DRIVER_PATH) components ; \
	fi
ifneq ($(strip $(BUILD_RDKWIFI)),)
	if [ ! -d cmwifi/wifi_hal ]; then \
	   ln -sf $(COMPON_WIRELESS_DRIVER_PATH)/rdk/ccsp/hal/wifi/source/wifi ./cmwifi/wifi_hal; \
	   ln -sf $(WIRELESS_IMPL_PATH)/cmwifi/libs/wlcsm ./cmwifi/wlcsm; \
	fi
endif

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
	if [ -e $(WIRELESS_DRIVER_PATH)/router/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/router/$@ $@ ; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/$@ $@ ; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/$@ $@ ; \
	elif [ -e $(WIRELESS_DRIVER_PATH)/router/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/router/$@ $@ ; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/$@ $@ ; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/router/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/router/$@ $@ ; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/router/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/router/$@ $@ ; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/router/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/router/$@ $@ ; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/router/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/router/$@ $@ ; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/router/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/router/$@ $@ ; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/router/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/router/$@ $@ ; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/router/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/router/$@ $@ ; \
	if [ -e $@/Makefile.dslcpe ]; then \
	   $(MAKE) -C $@ -f Makefile.dslcpe install ;\
	fi \
	fi


#
#

util:
	install -d $(INSTALLDIR)/usr/sbin
	install -m 755 $(APPS_UTILS_DIR)/WlGetDriverCfg.sh $(INSTALLDIR)/usr/sbin
	install -m 755 $(APPS_UTILS_DIR)/WlGetDriverStats.sh $(INSTALLDIR)/usr/sbin
	install -m 755 $(APPS_UTILS_DIR)/init_tmctl.sh $(INSTALLDIR)/usr/sbin
#

#usbdev apps for Wlan USB driver
#
ifneq (,$(findstring usb, $(strip $(WLBUS))))
#ifeq ($(strip $(BRCM_DRIVER_WIRELESS_USBAP)),y)
usbdev:
	@echo "copy bcmdl utilities to " $(INSTALL_DIR)/sbin
	rm -f $@
	if [ -e $(WIRELESS_DRIVER_PATH)/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/$@ $@ ; \
	fi

	cd usbdev; tar xzkf libusb-0.1.12/libusb-0.1.12.tgz 2>/dev/null; cd ..
	$(MAKE) -C usbdev/libusb-0.1.12 clean
	$(MAKE) TARGETENV=linuxmips -C usbdev/libusb-0.1.12
	install -m 755 usbdev/libusb-0.1.12/.libs/libusb-0.1.so.4 $(INSTALL_DIR)/lib

#	make -C usbdev/usbdl clean
#	make -C usbdev/usbdl
	install -m 755 usbdev/usbdl/bcmdl $(INSTALL_DIR)/bin
	cp -f $(WIRELESS_DRIVER_PATH)/dongle/rte/wl/builds/4322-bmac/rom-ag/rtecdc.trx $(INSTALL_DIR)/rom/rom/etc/wlan
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
	if [ -e $(WIRELESS_DRIVER_PATH)/router/$@ ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/router/$@ $@ ; \
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
	if [ -e $(WIRELESS_DRIVER_PATH)/wapi ]; then \
	   ln -s $(WIRELESS_DRIVER_PATH)/wapi ias ; \
	fi; \
	if [ -e ias/Makefile.dslcpe ]; then \
	$(MAKE) -C $@ $(BUILD_WAPI) -f Makefile.dslcpe install ;\
	fi
else
ias:
	@echo "skipping $@ (not configured)"
endif

WLROUTER_WWW := $(HNDAPPS_DIR)/www/broadcom
ifneq ($(strip $(BUILD_BSTREAM_IQOS)),)
IQOS_ICONS   := $(HNDAPPS_DIR)/../apps/iqos/www/icons
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
	mkdir -p  $(INSTALL_DIR)/webs/wlrouter
	rm -rf $(INSTALL_DIR)/www
	install -m 644 $(WLROUTER_PAGES) $(INSTALL_DIR)/webs/wlrouter
	install -m 644 ./css/* $(INSTALL_DIR)/webs/wlrouter
else
	@echo "No www for ikos"
endif

