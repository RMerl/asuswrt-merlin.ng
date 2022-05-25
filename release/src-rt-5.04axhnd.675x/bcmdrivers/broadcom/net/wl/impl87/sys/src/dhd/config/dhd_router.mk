# Helper makefile for building Broadcom dongle host driver (DHD) for
# router platforms
# This file maps dhd feature flags DHDFLAGS(import) and DHDFILES_SRC(export).
#
# Copyright (C) 2022, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
# <<Broadcom-WL-IPTag/Open:>>
#
# $Id: dhd.mk 387757 2013-02-27 01:09:03Z $

# Components path inclusion
include $(dir $(lastword $(MAKEFILE_LIST)))/../../makefiles/WLAN_Common.mk

LINUXVERSION=$(VERSION).$(PATCHLEVEL).$(SUBLEVEL).$(EXTRAVERSION)

# Let the mogrifier to handle it in router platform

#ifdef WLTEST
ifeq ($(WLTEST),1)
    DHDFLAGS += -DWLTEST -DIOCTL_RESP_TIMEOUT=20000
endif
#endif

#ifdef BCMDBG
ifeq ($(BCMDBG),1)
    DHDFLAGS += -DBCMDBG
endif
#endif

#ifdef BCMDBG_ASSERT
ifeq ($(BCMDBG_ASSERT),1)
    DHDFLAGS += -DBCMDBG_ASSERT
endif
#endif

ifeq ($(BCMQT),1)
    DHDFLAGS += -DBCMSLTGT -DBCMQT=1
endif

# Common Flags
DHDFLAGS        += -Werror -Wno-incompatible-pointer-types
DHDFLAGS        += -DLINUX
DHDFLAGS        += -DBCMDRIVER
DHDFLAGS        += -DBCMDONGLEHOST

ifneq ($(strip $(BCA_HNDROUTER)),)
DHDFLAGS        += -DDHDAP

ifneq ($(strip $(CONFIG_BCM_WLAN_64BITPHYSADDR)),)
    DHDFLAGS        += -DBCMDMA64OSL
endif

ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
    DHDFLAGS    += -DPKTC -DBCM_PKTFWD -DBCM_PKTFWD_DWDS
endif

ifneq ($(strip $(CONFIG_BCM_CPEPKTC)),)
    DHDFLAGS    += -DPKTC -DPKTC_TBL -DBCM_CPE_PKTC
endif
# NBUFF (fkb/skb)
ifneq ($(strip $(CONFIG_BCM_KF_NBUFF)),)
DHDFLAGS	+= -DBCM_NBUFF
DHDFLAGS	+= -DBCM_NBUFF_PKT
DHDIFLAGS       += $(INC_RDP_FLAGS)
endif

# BLOG/FCACHE
ifneq ($(strip $(CONFIG_BLOG)),)
DHDFLAGS	+= -DBCM_BLOG
endif
DHDIFLAGS	+= -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
DHDIFLAGS	+= -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
DHDIFLAGS	+= -I$(BRCMDRIVERS_DIR)/broadcom/char/wlcsm_ext/impl1/include
DHDIFLAGS	+= -I$(BRCMDRIVERS_DIR)/opensource/include/bcm963xx
# WFD (WiFi Forwarding Driver)
ifneq ($(strip $(CONFIG_BCM_WIFI_FORWARDING_DRV)),)
DHDWFD := 1
endif

# Archer WLAN (implements WiFi Forwarding Driver)
ifneq ($(strip $(CONFIG_BCM_ARCHER_WLAN)),)
DHDWFD := 1
DHDFLAGS += -DBCM_AWL
endif

ifeq ($(strip $(DHDWFD)), 1)
DHDFLAGS += -DBCM_WFD
DHDIFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)

# Enable Fcache based WFD for 47189
ifeq ($(BRCM_CHIP),47189)
ifneq ($(strip $(BCA_CPEROUTER)),)
DHDFLAGS += -DCONFIG_BCM_FC_BASED_WFD
endif
endif

endif

ifneq ($(strip $(CONFIG_BCM_DHD_ARCHER)),)
# Archer DHD Offload follows the same implementation as Runner
CONFIG_BCM_DHD_RUNNER=y
endif

ifeq ($(CMWIFI),)
DHDFLAGS += -DBCM_GMAC3
endif # CMWIFI
DHDFLAGS += -DBCM_NO_WOFA
#
# DHD Runner Offload
#   Internally Enables DHD_D2H_SOFT_DOORBELL_SUPPORT
#   Internally Disables CONFIG_BCM_WFD_CHAIN_SUPPORT, DHD_RX_CHAINING
#
ifneq ($(strip $(CONFIG_BCM_DHD_RUNNER)),)
DHDFLAGS += -DBCM_DHD_RUNNER

ifeq ($(strip $(CMWIFI) $(CONFIG_BCM_DHD_ARCHER)),)
# Runner interface include paths and flags
DHDIFLAGS += -I$(INC_RDPA_MW_PATH)
endif
endif

endif

# PCIe specific flags
DHDFLAGS        += -DPCIE_FULL_DONGLE
DHDFLAGS        += -DBCMPCIE
DHDFLAGS        += -DBCMPCIE_IPC_ACWI
#Enable to use PCIe MSI interrupts if enabled in BSP
ifeq ($(CONFIG_PCI_MSI),y)
DHDFLAGS        += -DBCM_WLAN_PCIE_MSI
endif
DHDFLAGS        += -DCUSTOM_DPC_PRIO_SETTING=-1
ifneq ($(CONFIG_STBAP),y)
DHDFLAGS        += -DBCM_ROUTER_DHD
endif
DHDFLAGS        += -DDHD_DEBUG
ifneq ($(BCM_EXTFDIMAGE_PATH),)
DHDFLAGS        += -DEXTFDIMGPATH=\"$(BCM_EXTFDIMAGE_PATH)\"
else
ifneq ($(CONFIG_STBAP),y)
DHDFLAGS        +=$(shell cat "$(SRCBASE)/shared/rtecdc_router.h" |grep DLIMAGE | sed -e 's/\#define /\-D/g')
DHDFLAGS        += -DBCMEMBEDIMAGE=\"rtecdc_router.h\"
endif
endif
DHDFLAGS        += -DDHD_UNICAST_DHCP
DHDFLAGS        += -DDHD_L2_FILTER
DHDFLAGS        += -DQOS_MAP_SET
DHDFLAGS        += -DDHD_PSTA
ifneq ($(RTCONFIG_DPSTA),)
DHDFLAGS        += -DDHD_DPSTA
endif
DHDFLAGS        += -DDHD_WET
DHDFLAGS        += -DDHD_MCAST_REGEN
DHDFLAGS        += -DSHOW_EVENTS

ifeq ($(CONFIG_STBAP),y)
DHDFLAGS += -DBCM_REQUEST_FW
endif

ifneq ($(CONFIG_LBR_AGGR),)
DHDFLAGS        += -DDHD_LBR_AGGR_BCM_ROUTER
endif

#M2M host memory allocation
ifneq ($(CONFIG_STBAP),y)
DHDFLAGS		+= -DBCM_HOST_MEM_SCB -DDMA_HOST_BUFFER_LEN=0x80000
endif

ifeq ($(BCM_BUZZZ),1)
DHDFLAGS        += -DBCM_BUZZZ
endif

# Idle flowring Eviction
DHDFLAGS        += -DDHD_IFE

# Dongle DMAs indices to from TCM.
# If BCM_INDX_DMA is defined, then dongle MUST support DMAing of indices.
# When not defined, DHD learns dongle's DMAing capability and adopts the
# advertized RD/WR index size.
# DHDFLAGS        += -DBCM_INDX_DMA

ifneq ($(CONFIG_BCM_HWA),)
DHDFLAGS        += -DBCMHWA
endif

# WMF Specific Flags
DHDFLAGS        += -DDHD_WMF
DHDFLAGS        += -DDHD_IGMP_UCQUERY
DHDFLAGS        += -DDHD_UCAST_UPNP

# trunk uses bcmcrypto component
DHDFLAGS        += -DBCMCRYPTO_COMPONENT
ifeq ($(CONFIG_BCM_HOSTAPD),y)
ifeq ($(CONFIG_STBAP),y)
DHDFLAGS        += -DWL_VENDOR_EXT_SUPPORT
endif # CONFIG_STBAP
ifeq ($(BUILD_OPENWRT_NATIVE),y)
DHDFLAGS	+= -DWL_CFG80211_STRICT
endif
DHDFLAGS        += -DROUTER_CFG
DHDFLAGS        += -DWL_CFG80211
DHDFLAGS        += -DWL_HAPD_WDS
DHDFLAGS        += -DUSE_CFG80211
DHDFLAGS        += -DWL_DRV_AVOID_SCANCACHE
DHDFLAGS        += -DWL_RATELINKMEM
DHDFLAGS        += -DWLP2P
DHDFLAGS        += -DSUPPORT_SOFTAP_WPAWPA2_MIXED
DHDFLAGS        += -DMFP
DHDFLAGS	+= -DWLFBT
DHDFLAGS	+= -DWL11U
DHDFLAGS	+= -DWL_DPP
DHDFLAGS	+= -DWL_SPP_AMSDU
ifeq ($(call wlan_version_ge,$(LINUXVERSION),3.14.0),TRUE)
  DHDFLAGS	+= -DWL_VENDOR_EXT_SUPPORT
endif

ifneq ($(CONFIG_STBAP),y)
#For router, SAE is always enabled
DHDFLAGS        += -DWL_SAE
endif
endif

# DHD Include Paths
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/include
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/../components/shared
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/../components/wlioctl/include
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/../components/proto/include
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/../components/math/include
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/common/include
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/shared
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/shared/bcmwifi/include
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/dhd/sys
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/dongle/include
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/wl/sys
DHDIFLAGS	+= -I$(SRCBASE)/../../main/src/router-sysdep/bcmdrv/include
ifeq ($(CONFIG_BCM_HOSTAPD),y)
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/../components/clm-api/include
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/wl/ppr/include
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/wl/iocv/include
endif
DHDIFLAGS       += -I$(SRCBASE)/../../main/src/router-sysdep/dpsta

# DHD Source files - For pciefd-msgbuf target
DHDFILES_SRC    := src/shared/aiutils.c
DHDFILES_SRC    += src/shared/bcmevent.c
DHDFILES_SRC    += src/shared/bcm_l2_filter.c
DHDFILES_SRC    += src/shared/bcmotp.c
DHDFILES_SRC    += src/shared/bcmsrom.c
DHDFILES_SRC    += src/shared/bcmutils.c
DHDFILES_SRC    += src/shared/bcmxtlv.c
DHDFILES_SRC    += src/shared/hnd_pktq.c
DHDFILES_SRC    += src/shared/hnd_pktpool.c
DHDFILES_SRC    += src/shared/hndpmu.c
DHDFILES_SRC    += src/shared/sbutils.c
DHDFILES_SRC    += src/shared/siutils.c
DHDFILES_SRC    += src/shared/pcie_core.c
DHDFILES_SRC    += src/shared/bcm_psta.c
DHDFILES_SRC    += src/shared/bcmwifi/src/bcmwifi_channels.c
DHDFILES_SRC    += src/dhd/sys/dhd_common.c
DHDFILES_SRC    += src/dhd/sys/dhd_custom_gpio.c
DHDFILES_SRC    += src/dhd/sys/dhd_ip.c
DHDFILES_SRC    += src/dhd/sys/dhd_linux.c
DHDFILES_SRC    += src/dhd/sys/dhd_linux_platdev.c
DHDFILES_SRC    += src/dhd/sys/dhd_linux_sched.c
DHDFILES_SRC    += src/dhd/sys/dhd_linux_wq.c
DHDFILES_SRC    += src/dhd/sys/dhd_ife.c
DHDFILES_SRC    += src/wl/sys/wl_core.c
DHDFILES_SRC    += src/wl/sys/wldev_common.c

ifeq ($(WL_MONITOR),1)
DHDFLAGS        += -DWL_MONITOR
DHDFILES_SRC    += src/shared/bcmwifi/src/bcmwifi_monitor.c
DHDFILES_SRC    += src/shared/bcmwifi/src/bcmwifi_radiotap.c
DHDFILES_SRC    += src/shared/bcmwifi/src/bcmwifi_rates.c
DHDFILES_SRC    += src/shared/bcmwifi/src/bcmwifi_rspec.c
endif

ifneq ($(CONFIG_LBR_AGGR),)
DHDFILES_SRC    += src/dhd/sys/dhd_aggr.c
DHDFILES_SRC    += src/dhd/sys/dhd_lbr_aggr_linux.c
endif
DHDFILES_SRC    += src/dhd/sys/dhd_macdbg.c
DHDFILES_SRC    += src/dhd/sys/dhd_msgbuf.c
DHDFILES_SRC    += src/dhd/sys/dhd_flowring.c
DHDFILES_SRC    += src/dhd/sys/dhd_pcie.c
DHDFILES_SRC    += src/dhd/sys/dhd_pcie_linux.c
DHDFILES_SRC    += src/dhd/sys/dhd_wmf_linux.c
DHDFILES_SRC    += src/dhd/sys/dhd_l2_filter.c
DHDFILES_SRC    += src/dhd/sys/dhd_psta.c
DHDFILES_SRC    += src/dhd/sys/dhd_wet.c
ifeq ($(CONFIG_BCM_HOSTAPD),y)
DHDFILES_SRC    += src/dhd/sys/dhd_cfg80211.c
DHDFILES_SRC    += src/wl/sys/wl_cfg80211.c
DHDFILES_SRC    += src/wl/sys/wl_cfgp2p.c
DHDFILES_SRC    += src/wl/sys/wl_cfgvendor_common.c
DHDFILES_SRC    += src/wl/sys/wl_feature.c
DHDFILES_SRC    += src/wl/sys/wl_linux_mon.c
endif

ifeq ($(strip $(USE_WLAN_SHARED)), 1)
DHDIFLAGS     += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/shared/impl$(WLAN_SHARED_IMPL)
WLAN_DHD_PATH := ../../../shared/impl$(WLAN_SHARED_IMPL)
else
WLAN_DHD_PATH := src/dhd/sys
endif

ifneq ($(strip $(BCA_HNDROUTER)),)
ifneq ($(strip $(CONFIG_BCM_KF_NBUFF)),)
DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_nbuff.c
endif
ifneq ($(strip $(CONFIG_BLOG)),)
DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_blog.c
endif
ifeq ($(strip $(DHDWFD)), 1)
DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_wfd.c
endif
ifneq ($(strip $(CONFIG_BCM_DHD_RUNNER)),)
DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_runner.c
endif

# pktc rx
ifneq ($(strip $(CONFIG_BCM_CPEPKTC)),)
DHDFILES_SRC += $(WLAN_DHD_PATH)/dhd_pktc.c
endif
DHDFILES_SRC    += src/shared/linux_osl.c
DHDFILES_SRC    += src/shared/nvram_ext.c
endif
ifeq ($(CONFIG_STBAP),y)
DHDFILES_SRC    += src/shared/linux_osl.c
DHDFILES_SRC    += src/shared/linux_pkt.c
endif

# speed service
ifneq ($(strip $(BUILD_SPDSVC)),)
DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_spdsvc.c
endif

ifneq ($(CONFIG_STBAP),y)
endif

ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
    DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_pktfwd.c
endif

ifneq ($(strip $(CONFIG_BCM_ARCHER_WLAN)),)
    DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_awl.c
endif

# srom map
DHDFILES_SRC    += src/dhd/sys/dhd_srommap.c

DHD_OBJS := $(sort $(patsubst %.c,%.o,$(addprefix $(SRCBASE_OFFSET)/,$(patsubst src/%,%,$(DHDFILES_SRC)))))

EXTRA_CFLAGS += $(DHDFLAGS) $(DHDIFLAGS)

ifeq ($(strip $(USE_WLAN_SHARED)), 1)
ifneq ($(strip $(WLAN_SHARED_DIR)),)

-include   $(WLAN_SHARED_DIR)/wifi_cfg_common.mk
ifneq ($(strip $(BCA_HNDROUTER)),)
EXTRA_CFLAGS += -DBCM_COUNTER_EXTSTATS
ifneq ($(strip $(CONFIG_BCM_KF_EXTSTATS)),)
EXTRA_CFLAGS += -DBCM_CPEROUTER_EXTSTATS
endif
endif
endif
endif
