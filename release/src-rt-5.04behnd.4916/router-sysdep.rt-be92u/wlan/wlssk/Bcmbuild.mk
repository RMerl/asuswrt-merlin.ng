EXE    := wlssk
 
all install: conditional_build
 
 
ifeq ($(BCM_MODULAR_BUILD),)
  # Old way: infer location of make.common based on pwd.
  CURR_DIR := $(shell pwd)
  BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
  BUILD_DIR:=$(word 1, $(BUILD_DIR))
  include $(BUILD_DIR)/make.common
  include $(BUILD_DIR)/make.wlan
else
  # New Modular Build way: EXT_BUILD_DIR must be set.
  # Also point BUILD_DIR to EXT_BUILD_DIR
  BUILD_DIR := $(EXT_BUILD_DIR)
  include $(EXT_BUILD_DIR)/make.common
  include $(BUILD_DIR)/make.wlan
endif
 
 
ARCH                  := $(PROFILE_ARCH)
EXE_INSTALL_DIR       := $(BCM_FSBUILD_DIR)/private/bin
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
 
CFLAGS += -I$(HNDAPPS_DIR)/shared  -I$(HNDAPPS_DIR)/../wlioctl/include -I$(HNDAPPS_DIR)/../proto/include
CFLAGS += -I$(HNDAPPS_DIR)/../math/include 
 
ALLOWED_LIB_DIRS := /lib:/private/lib:/public/lib:/public/lib64:/public/lib32:/usr/lib
 
LIB_RPATH = $(INSTALL_DIR)$(subst :,:$(INSTALL_DIR),$(ALLOWED_LIB_DIRS))
LDFLAGS += $(BCM_RPATH_LINK_OPTION) -Wl,-rpath-link=$(LIB_RPATH) 
 
export ARCH CFLAGS BCM_LD_FLAGS BCM_RPATH_LINK_OPTION BCM_LIB_PATH CMS_LIB_PATH LDFLAGS LIBS_CC_RT
export BUILD_DIR EXEALT INSTALL_DIR EXE_INSTALL_DIR
export BUILD_OPENWRT_NATIVE BUILD_BRCM_OPENWRT BUILD_PRPL_FEEDS BUILD_SMARTMESH_FOR_PRPL
export BUILD_OPENSYNC_GW BUILD_OPENSYNC BUILD_BRCM_CMS BUILD_BRCM_BDK BUILD_RDKWIFI RDK_BUILD USR_CFLAGS
export BUILD_WLDATAELD BUILD_BAS2 BUILD_BCM_WLAN_DPDCTL BUILD_WLHSPOT BUILD_WSC BUILD_WSC_CONFIG_UPNP_DISABLE BUILD_BRCM_AIRIQ
export BUILD_BRCM_HOSTAPD BRCM_ENGR_ONLY BUILD_BRCM_UNFWLCFG BUILD_HND_EAP BUILD_CUSTOMER WLTEST
export WIRELESS_IMPL_PATH WIRELESS_DRIVER_PATH BUILD_BSTREAM_IQOS
 
 
ifneq ($(strip $(BUILD_BRCM_BDK)),)
# Remove all mdm_cbk_* and bcm_*_hal libs from CMS_CORE_LIBS.  The local
# Makefile will specify which mdm_cbk and hal libs it needs.
MDM_CORE_LIBS := $(patsubst -lmdm_cbk_%,,$(CMS_CORE_LIBS))
MDM_CORE_LIBS := $(patsubst -lbcm_%_hal,,$(MDM_CORE_LIBS))
 MDM_CORE_LIBS += -lmdm_cbk_wifi
else
 MDM_CORE_LIBS := $(CMS_CORE_LIBS)
endif
 
export CMS_COMMON_LIBS MDM_CORE_LIBS
 
 
# Final location of EXE for system image.  Only the BRCM build system needs to
# know about this.
FINAL_EXE_INSTALL_DIR := $(INSTALL_DIR)/bin
 
 
# Do we build tr69 component even if we don't build the Broadcom tr69c app?
# The Device.ManagementServer should always be available in the system.
ifeq ($(strip $(BUILD_OPENWRT_NATIVE))$(strip $(BUILD_PRPL_FEEDS)),)
ifneq ($(strip $(BUILD_BRCM_WLSSK)),)
COND_BUILD_WLSSK := 1
endif
else # native openwrt or build_prpl_feed
ifneq ($(strip $(BUILD_SMARTMESH_FOR_PRPL)),)
COND_BUILD_WLSSK := 1
endif
endif
 
ifneq ($(strip $(COND_BUILD_WLSSK)),)
 
conditional_build:
	mkdir -p objs
	$(MAKE) -C objs -f ../Makefile install
	mkdir -p $(FINAL_EXE_INSTALL_DIR)
	cp -p $(EXE_INSTALL_DIR)/$(EXE) $(FINAL_EXE_INSTALL_DIR)
	-ln -sf /usr/sbin/wl $(FINAL_EXE_INSTALL_DIR)/wlctl
	-ln -sf /usr/sbin/dhd $(FINAL_EXE_INSTALL_DIR)/dhdctl
 
else
 
conditional_build:
	@echo "Skipping $(EXE) (not configured)"
 
endif
 
 
clean:
	rm -f $(FINAL_EXE_INSTALL_DIR)/$(EXE)
	-mkdir -p objs
	-$(MAKE) -C objs -f ../Makefile clean
	rm -rf objs
 
 
shell:
	@echo "Entering makefile debug shell (type exit to exit) >>>"
	@bash -i
	@echo "exiting debug shell."
