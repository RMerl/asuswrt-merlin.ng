# File: bcmdrivers/Makefile
#
# Makefile for the Linux kernel modules.
#
export LN_NAME=bcm9$(BRCM_CHIP)
LN_DRIVER_DIRS :=
CLEAN_DRIVER_DIRS :=

-include $(KERNEL_DIR)/.config

export BCMARCH=$(subst ",,$(CONFIG_BCM_CPU_ARCH_NAME))
#"#

EXTRA_CFLAGS += ${DSL_XTM_SAR_TX_RX_IUDMA_631XX_DEFINES}

LN_TARGS := 

#---------------------------------
# LN_RULE
# params:
#   1: config value
#   2: dir
#   3: ln name
#   4: impl number
define LN_RULE

ifneq (,$(strip $(1)))

obj-$(strip $(1)) += $(strip $(2))/$(strip $(3))/

LN_TARGS += $(strip $(2))/$(strip $(3))

$(strip $(2))/$(strip $(3)) :
	@echo "linking $$@ => impl$(strip $(4))";
	$$(Q)ln -sfn impl$(strip $(4)) $$@;

endif
endef

#$(info   )
#$(info EXAMPLE RULE: -----------------------------)
#$(info $(call LN_RULE, $(CONFIG_BCM_BOARD), opensource/char/board/$(BRCM_BOARD), $(LN_NAME), $(CONFIG_BCM_BOARD_IMPL)))
#$(info -----------------------------)
#$(info   )

#---------------------------------
# LN_RULE_DEP: Copied from LN_RULE, but includes optional dependency to enforce build order
# params:
#   1: config value
#   2: dir
#   3: ln name
#   4: impl number
#   5: dependencies
define LN_RULE_DEP

ifneq (,$(strip $(1)))

obj-$(strip $(1)) += $(strip $(2))/$(strip $(3))/

LN_TARGS += $(strip $(2))/$(strip $(3))

$(strip $(2))/$(strip $(3)) : $(5)
	@echo "linking $$@ => impl$(strip $(4))";
	$$(Q)ln -sfn impl$(strip $(4)) $$@;

endif
endef

#---------------------------------
# LN_RULE_AG (Auto-Generated)
# Note: this is used for autodetected components.    Autodetected components
# should have an "autodetect" in thier parent directory along with a Kconfig.autodetect 
# file.   The autodetect file should have a line containing 
#
# "FEATURE:_featurename_" 

# (where _featurename_ is a unique name for the feature).   The Kconfig file 
# should include a choice clause to choose the impl, and may contain any other 
# feature-specific configuration options.

# params:
#   1: config name
#   2: dir
#   3: ln name

define LN_RULE_AG

ifneq (,$$($(strip $(1))))

_$(strip $(1))_NAME:=$(strip $(1))
_$(strip $(1))_VAL:=$${$(strip $(1))}
_$(strip $(1))_DIR:=$(strip $(2))
_$(strip $(1))_IMPL:=$${$(strip $(1))_IMPL}
_$(strip $(1))_LNNAME:=$(strip $(3))

export $$(_$(strip $(1))_NAME)_IMPL=$$(_$(strip $(1))_IMPL)

$$(info LN_RULE_AG (NAME=$$(_$(strip $(1))_NAME),VAL=$$(_$(strip $(1))_VAL),DIR=$$(_$(strip $(1))_DIR),IMPL=$$(_$(strip $(1))_IMPL)))
$$(info $$(_$(strip $(1))_NAME)_IMPL=$$(_$(strip $(1))_IMPL))

obj-$$(_$(strip $(1))_VAL) += $$(_$(strip $(1))_DIR)/$$(_$(strip $(1))_LNNAME)/

LN_TARGS += $$(_$(strip $(1))_DIR)/$$(_$(strip $(1))_LNNAME)

$$(_$(strip $(1))_DIR)/$$(_$(strip $(1))_LNNAME) :
	@echo "linking $$@ => impl$$(_$(strip $(1))_IMPL)";
	$$(Q)ln -sfn impl$$(_$(strip $(1))_IMPL) $$@;

endif
endef

#$(info   )
#$(info EXAMPLE LN_RULE_AG: ------------------------------)
#$(info $(call LN_RULE_AG, CONFIG_BCM_FAP, broadcom/char/fap, $(LN_NAME)))
#$(info ------------------------------)
#$(info   )


#---------------------------------

#
# To create a new driver, add a line below, which has the following format:
#
# $(eval $(call LN_RULE, CONFIG_OPTION, DIRECTORY, LINK_NAME, IMPL))
#
# where:
#  CONFIG_OPTION is the config option (such as "$(CONFIG_BCM_ADSL)"), which determines
#    whether or not the driver will be compiled
#  DIRECTORY is the directory (relative to bcmdrivers) where all the implX subdirectories 
#    reside
#  LINK_NAME should be $(LN_NAME)
#  IMPL is the config option which defines the implementation number the driver should use
#    (for example "$(CONFIG_BCM_ADSL_IMPL)"
#


$(eval $(call LN_RULE, $(CONFIG_BCM_BOARD), opensource/char/board/$(BRCM_BOARD), $(LN_NAME), $(CONFIG_BCM_BOARD_IMPL)))

#ifneq ($(CONFIG_BCM_NVRAM),)
$(eval $(call LN_RULE,$(CONFIG_BCM_NVRAM), broadcom/char/wlcsm_ext,$(LN_NAME), 1))
#endif
$(eval $(call LN_RULE, $(CONFIG_BCM_CHIPINFO), broadcom/char/chipinfo, $(LN_NAME), $(CONFIG_BCM_CHIPINFO_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_SERIAL), opensource/char/serial, $(LN_NAME), $(CONFIG_BCM_SERIAL_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_EXT_TIMER), opensource/char/timer, $(LN_NAME), $(CONFIG_BCM_EXT_TIMER_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_I2C_BUS), opensource/char/i2c/busses, $(LN_NAME), $(CONFIG_BCM_I2C_BUS_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_I2C_CHIP), opensource/char/i2c/chips, $(LN_NAME), $(CONFIG_BCM_I2C_CHIP_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_PLATFORM), opensource/char/plat-bcm, $(LN_NAME), $(CONFIG_BCM_PLATFORM_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_OPTICALDET), opensource/char/opticaldet, $(LN_NAME), $(CONFIG_BCM_OPTICALDET_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_WANTYPEDET), opensource/char/wantypedet, $(LN_NAME), $(CONFIG_BCM_WANTYPEDET_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_PKTFLOW), broadcom/char/pktflow, $(LN_NAME), $(CONFIG_BCM_PKTFLOW_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_PKTCMF), broadcom/char/pktcmf, $(LN_NAME), $(CONFIG_BCM_PKTCMF_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_FAP), broadcom/char/fap, $(LN_NAME), $(CONFIG_BCM_FAP_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_TM), broadcom/net/tm, $(LN_NAME), $(CONFIG_BCM_TM_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_SPDSVC), broadcom/char/spdsvc, $(LN_NAME), $(CONFIG_BCM_SPDSVC_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_SPDTEST), broadcom/char/tcpspdtest, $(LN_NAME), $(CONFIG_BCM_SPDTEST_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_PKTDMA), opensource/net/lib/pktdma, $(LN_NAME), $(CONFIG_BCM_PKTDMA_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_PKTRUNNER), broadcom/char/pktrunner, $(LN_NAME), $(CONFIG_BCM_PKTRUNNER_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_CMDLIST), broadcom/char/cmdlist, $(LN_NAME), $(CONFIG_BCM_CMDLIST_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_ARCHER), broadcom/char/archer, $(LN_NAME), $(CONFIG_BCM_ARCHER_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_ENET_SYSPORT), broadcom/char/sysport, $(LN_NAME), 1))
$(eval $(call LN_RULE, $(CONFIG_BCM_UBUSCAP), broadcom/char/ubuscap, $(LN_NAME), $(CONFIG_BCM_UBUSCAP_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_MEMCCAP), broadcom/char/memccap, $(LN_NAME), $(CONFIG_BCM_MEMCCAP_IMPL)))

ifneq ($(CONFIG_BCM_XTMRT),)
$(eval $(call LN_RULE, $(CONFIG_BCM_XTMRT), opensource/net/xtmrt,$(LN_NAME), $(CONFIG_BCM_XTMRT_IMPL)))
endif
ifneq ($(CONFIG_BCM_XTMCFG),)
$(eval $(call LN_RULE, $(CONFIG_BCM_XTMCFG), broadcom/char/xtmcfg,$(LN_NAME), $(CONFIG_BCM_XTMCFG_IMPL)))
endif

$(eval $(call LN_RULE, $(CONFIG_BCM_EXT_BONDING), broadcom/char/ext_bonding, $(LN_NAME), $(CONFIG_BCM_EXT_BONDING_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_ADSL), broadcom/char/adsl, $(LN_NAME), $(CONFIG_BCM_ADSL_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_PROCFS), broadcom/char/bcmprocfs, $(LN_NAME), $(CONFIG_BCM_PROCFS_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_BCMPROF), broadcom/char/profiler, $(LN_NAME), $(CONFIG_BCM_BCMPROF_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_DECT), broadcom/char/dect, $(LN_NAME), $(CONFIG_BCM_DECT_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_SLICSLAC), broadcom/char/slicslac, $(LN_NAME), $(CONFIG_BCM_SLICSLAC_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_DSPHAL), opensource/char/dsphal, $(LN_NAME), $(CONFIG_BCM_DSPHAL_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_TMS), broadcom/char/tms, $(LN_NAME), $(CONFIG_BCM_TMS_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_RDPA_BRIDGE), opensource/net/bridge, $(LN_NAME), $(CONFIG_BCM_RDPA_BRIDGE_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_ENET), opensource/net/enet, $(LN_NAME), $(CONFIG_BCM_ENET_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_GPON), broadcom/char/gpon, $(LN_NAME), $(CONFIG_BCM_GPON_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_LASER), broadcom/char/laser, $(LN_NAME), $(CONFIG_BCM_LASER_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_LICENSE), broadcom/char/license, $(LN_NAME), $(CONFIG_BCM_LICENSE_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_PMD), broadcom/char/pmd, $(LN_NAME), $(CONFIG_BCM_PMD_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_MoCA), opensource/char/moca, $(LN_NAME), $(CONFIG_BCM_MoCA_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_VLAN), broadcom/char/vlan, $(LN_NAME), $(CONFIG_BCM_VLAN_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_USB), broadcom/net/usb, $(LN_NAME), $(CONFIG_BCM_USB_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_ISDN), broadcom/char/isdn, $(LN_NAME), $(CONFIG_BCM_ISDN_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_SPU), opensource/char/spudd, $(LN_NAME), $(CONFIG_BCM_SPU_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_PWRMNGT), broadcom/char/pwrmngt, $(LN_NAME), $(CONFIG_BCM_PWRMNGT_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_INGQOS), broadcom/char/ingqos, $(LN_NAME), $(CONFIG_BCM_INGQOS_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_BPM), broadcom/char/bpm, $(LN_NAME), $(CONFIG_BCM_BPM_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_ARL), broadcom/char/arl, $(LN_NAME), $(CONFIG_BCM_ARL_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_EPON_STACK), broadcom/char/epon, $(LN_NAME), $(CONFIG_BCM_EPON_STACK_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_RDPA_MW), opensource/char/rdpa_mw, $(LN_NAME), $(CONFIG_BCM_RDPA_MW_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_RDPA_DRV), opensource/char/rdpa_drv, $(LN_NAME), $(CONFIG_BCM_RDPA_DRV_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_WIFI_FORWARDING_DRV), opensource/net/wfd, $(LN_NAME), $(CONFIG_BCM_WIFI_FORWARDING_DRV_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_NETXL), opensource/net/netxl, $(LN_NAME), $(CONFIG_BCM_NETXL_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_SIM_CARD), opensource/char/sim_card, $(LN_NAME), $(CONFIG_BCM_SIM_CARD_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_TIME_SYNC), broadcom/char/time_sync, $(LN_NAME), $(CONFIG_BCM_TIME_SYNC_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_OTP), broadcom/char/otp, $(LN_NAME), $(CONFIG_BCM_OTP_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_SOTP), broadcom/char/sotp, $(LN_NAME), $(CONFIG_BCM_SOTP_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_DPI), broadcom/char/dpiengine, $(LN_NAME), $(CONFIG_BCM_DPI_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_DPI_QOS), broadcom/char/dpiqos, $(LN_NAME), $(CONFIG_BCM_DPI_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_DPI), opensource/char/dpicore, $(LN_NAME), $(CONFIG_BCM_DPI_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_HS_UART), opensource/char/hs_uart, $(LN_NAME), $(CONFIG_BCM_HS_UART_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_I2S), opensource/char/i2s, $(LN_NAME), $(CONFIG_BCM_I2S_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_SATA_TEST), broadcom/char/sata_test, $(LN_NAME), $(CONFIG_BCM_SATA_TEST_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_BLUETOOTH_USB), opensource/char/btusb, $(LN_NAME), $(CONFIG_BCM_BLUETOOTH_USB_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_NFC_I2C), opensource/char/nfc, $(LN_NAME), $(CONFIG_BCM_NFC_I2C_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_LTE), broadcom/net/lte, $(LN_NAME), $(CONFIG_BCM_LTE_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_RDPA_GPL_EXT), opensource/char/rdpa_gpl_ext, $(LN_NAME), $(CONFIG_BCM_RDPA_GPL_EXT_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_FPM), opensource/char/fpm, $(LN_NAME), $(CONFIG_BCM_FPM_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_MAP), opensource/char/map, $(LN_NAME), $(CONFIG_BCM_MAP_IMPL)))


$(eval $(call LN_RULE, $(CONFIG_BCM_EAPFWD), broadcom/net/eapfwd, $(LN_NAME), $(CONFIG_BCM_EAPFWD_IMPL)))

#include drivers with autodetect:

-include $(BRCMDRIVERS_DIR)/Makefile.autogen

ifneq ($(CONFIG_BCM_WLAN),)

ifneq ($(WLIMPL),)
   CONFIG_BCM_WLAN_IMPL=$(patsubst %,%,$(WLIMPL))  
endif

$(eval $(call LN_RULE, $(CONFIG_BCM_WLAN), broadcom/net/wl,$(LN_NAME), $(CONFIG_BCM_WLAN_IMPL)))

# needed for rules below
WLLINK = broadcom/net/wl/$(LN_NAME)

# use wlan shared code
export WLAN_SHARED_IMPL = 1
export BCM_NBUFF_COMMON = 1
export WLAN_SHARED_DIR  := $(BRCMDRIVERS_DIR)/broadcom/net/wl/shared/impl$(WLAN_SHARED_IMPL)

ifeq ($(strip $(CONFIG_BCM_WLAN_IMPL)),$(filter $(strip $(CONFIG_BCM_WLAN_IMPL)),24 25 26 27 29 30 32))
export USE_WLAN_SHARED = 1
endif

ifneq ($(strip $(BCA_HNDROUTER)),)
export USE_WLAN_SHARED = 1

# the softlinks created are not that useful. However the following directories within $(WLLINK)
# depend on the LN_RULE above that creates $(WLLINK). Hence we use the LN_RULE_DEP, so that when 
# multiple build jobs execute the dependency will be satisfied.
$(eval $(call LN_RULE_DEP, $(CONFIG_BCM_WLAN), broadcom/net/wl/$(LN_NAME)/$(HNDDRIVER_BASE)/hnd,,, $(WLLINK)))
$(eval $(call LN_RULE_DEP, $(CONFIG_BCM_WLAN), broadcom/net/wl/$(LN_NAME)/$(HNDDRIVER_BASE)/hnd_emf,,, $(WLLINK)))
$(eval $(call LN_RULE_DEP, $(CONFIG_BCM_WLAN), broadcom/net/wl/$(LN_NAME)/$(HNDDRIVER_BASE)/hnd_igs,,, $(WLLINK)))

ifneq (,$(firstword $(wildcard ./broadcom/net/wl/$(LN_NAME)/$(HNDDRIVER_BASE)/hnd_dpsta \
   $(src)/broadcom/net/wl/$(LN_NAME)/$(HNDDRIVER_BASE)/hnd_dpsta)))
$(eval $(call LN_RULE, $(CONFIG_BCM_WLAN), broadcom/net/wl/$(LN_NAME)/$(HNDDRIVER_BASE)/hnd_dpsta,,))
endif

ifeq ($(strip $(BUILD_HND_NIC)),)
$(eval $(call LN_RULE_DEP, $(CONFIG_BCM_WLAN), broadcom/net/wl/$(LN_NAME)/$(HNDDRIVER_BASE)/hnd_dhd,,, $(WLLINK)))
endif # BUILD_HND_NIC
$(eval $(call LN_RULE_DEP, $(CONFIG_BCM_WLAN), broadcom/net/wl/$(LN_NAME)/$(HNDDRIVER_BASE)/hnd_wl,,, $(WLLINK)))
endif # BCA_HNDROUTER

endif # CONFIG_BCM_WLAN

$(eval $(call LN_RULE, $(CONFIG_BCM_PCIE_HCD), opensource/bus/pci/host, $(LN_NAME), $(CONFIG_BCM_PCIE_HCD_IMPL)))
$(eval $(call LN_RULE, $(CONFIG_BCM_PCIE_GEPD), opensource/bus/pci/endpoint/gepd, $(LN_NAME), $(CONFIG_BCM_PCIE_GEPD_IMPL)))

ifeq ($(CONFIG_BCM_PKTCMF_IMPL),2)
  CLEAN_DRIVER_DIRS += broadcom/char/pktcmf/impl$(CONFIG_BCM_PKTCMF_IMPL)
endif

ifneq ($(CONFIG_BCM_DECT),)
  CLEAN_DRIVER_DIRS += broadcom/char/dect/impl$(CONFIG_BCM_DECT_IMPL)
endif

.PHONY : applinks symlinks clean version_info

applinks:
ifneq ($(CONFIG_BCM_PKTCMF),)
ifeq ($(CONFIG_BCM_PKTCMF_IMPL),2)
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/char/pktcmf/impl$(CONFIG_BCM_PKTCMF_IMPL) applinks
endif
endif


ifneq ($(MAKECMDGOALS),version_info)
find_opt := -print
endif

symlinks: applinks $(LN_TARGS)
	@echo "done bcmdriver links"

_CLEAN_LN_TARGS := $(addprefix _clean_,$(wildcard $(LN_TARGS)))
$(_CLEAN_LN_TARGS) :
	@-rm -f $(patsubst _clean_%,%,$@)

cleanlinks: $(_CLEAN_LN_TARGS)

_CLEAN_CLEAN_DRIVER_DIRS := $(addprefix _clean_,$(wildcard $(CLEAN_DRIVER_DIRS)))
$(_CLEAN_CLEAN_DRIVER_DIRS) :
	-$(MAKE) -C $(patsubst _clean_%,%,$@) clean

clean : $(_CLEAN_CLEAN_DRIVER_DIRS)
ifneq ($(CONFIG_BCM_WLAN),)
ifeq ($(strip $(BCA_HNDROUTER)),)
	$(info =====> WLAN clean: CONFIG_BCM_WLAN=$(CONFIG_BCM_WLAN) IMPL=$(CONFIG_BCM_WLAN_IMPL) <======)
	$(MAKE) -C broadcom/net/wl/impl$(CONFIG_BCM_WLAN_IMPL) -f Makefile.clean clean
endif
endif


version_info: symlinks
ifneq ($(CONFIG_BCM_WLAN),)
ifneq ($(strip $(BCA_HNDROUTER)),)
	ln -sf $(BRCMDRIVERS_DIR)/broadcom/net/wl/Makefile $(BRCMDRIVERS_DIR)/broadcom/net/wl/$(LN_NAME)/.
endif
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/$(LN_NAME) version_info
endif

symlinks: bcm_headers_install

bcm_headers_install: $(LN_TARGS)
	@echo "Installing bcm_headers"
	mkdir -p $(INC_BCMDRIVER_PATH)/include/
	cp -f --no-preserve=mode -r $(BRCMDRIVERS_DIR)/broadcom/include/$(BRCM_BOARD)/* $(INC_BCMDRIVER_PATH)/include/
	cp -f --no-preserve=mode -r $(BRCMDRIVERS_DIR)/opensource/include/$(BRCM_BOARD)/* $(INC_BCMDRIVER_PATH)/include/
	for i in `egrep -l '^bcm_headers_install:' $(BRCMDRIVERS_DIR)/*/*/*/$(LN_NAME)/Makefile` ; \
	   do $(MAKE) -C `dirname $$i` -f $$i bcm_headers_install INC_BCMDRIVER_PATH=$(INC_BCMDRIVER_PATH) ; \
	   done 

.PHONY : cleanlinks $(_CLEAN_LN_TARGS) clean $(_CLEAN_CLEAN_DRIVER_DIRS) bcm_headers_install
