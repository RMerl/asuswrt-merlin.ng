# Helper makefile for building Broadcom dongle host driver (DHD) for
# router platforms
# This file maps dhd feature flags DHDFLAGS(import) and DHDFILES_SRC(export).
#
# Copyright (C) 2025, Broadcom. All Rights Reserved.
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
# $Id: dhd_router.mk 831869 2023-10-27 04:43:52Z $

# Components path inclusion
include $(dir $(lastword $(MAKEFILE_LIST)))/../../makefiles/WLAN_Common.mk

LINUXVERSION=$(VERSION).$(PATCHLEVEL).$(SUBLEVEL).$(EXTRAVERSION)
ifeq ($(strip $(USE_WLAN_SHARED)), 1)
ifneq ($(strip $(WLAN_SHARED_DIR)),)
-include   $(WLAN_SHARED_DIR)/wifi_cfg_common.mk
endif
endif

# Let the mogrifier to handle it in router platform

#ifdef WLTEST
ifeq ($(WLTEST),1)
    DHDFLAGS += -DWLTEST -DIOCTL_RESP_TIMEOUT=20000
endif
#endif

#ifdef BCMDBG_MEM
ifeq ($(BCMDBG_MEM),1)
    DHDFLAGS += -DBCMDBG_MEM
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
DHDFLAGS        += -Werror
DHDFLAGS        += -DLINUX
DHDFLAGS        += -DBCMDRIVER
DHDFLAGS        += -DBCMDONGLEHOST
DHDFLAGS	+= -DHEALTH_CHECK

# === BCACPE and CMWIFI router build flags ===
ifneq ($(strip $(BCA_HNDROUTER)),)
DHDFLAGS        += -DDHDAP

ifneq ($(strip $(CONFIG_BCM_WLAN_64BITPHYSADDR)),)
    DHDFLAGS        += -DBCMDMA64OSL
endif

ifneq ($(strip $(CONFIG_BCM_EAPFWD)),)
    DHDFLAGS        += -DBCM_EAPFWD
    # DHD in SKB only mode, FKBs are not supported
    DHDFLAGS        += -DDHD_SKB
endif

ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
    DHDFLAGS    += -DPKTC -DBCM_PKTFWD -DBCM_PKTFWD_DWDS
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
DHDIFLAGS	+= -I$(BRCMDRIVERS_DIR)/opensource/char/bcm_knvram/impl1/include
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

# Perimeter lock disabled for 3390 CMWIFI Platform
ifneq ("${BRCM_CHIP}","3390")
DHDFLAGS += -DDHD_USE_PERIM
endif # BRCM_CHIP 3390

#
# DHD Runner Offload
#   Internally Enables DHD_D2H_SOFT_DOORBELL_SUPPORT
#   Internally Disables CONFIG_BCM_WFD_CHAIN_SUPPORT, DHD_RX_CHAINING
#
ifneq ($(strip $(CONFIG_BCM_DHD_RUNNER)),)
DHDFLAGS += -DBCM_DHD_RUNNER

ifeq ($(strip $(CONFIG_BCM_DHD_GFAP)),)
ifeq ($(strip $(CMWIFI) $(CONFIG_BCM_DHD_ARCHER)),)
# Runner interface include paths and flags
DHDIFLAGS += -I$(INC_RDPA_MW_PATH)
endif
endif
endif
# Generic offload code for Runner/Archer/CrossBow
ifneq ($(strip $(CONFIG_BCM_DHD_OFFLOAD)),)
EXTRA_CFLAGS += -DBCM_DHD_OFFLOAD
endif
endif

ifeq ($(strip $(WL_CSIMON_CUST_SENDUP)), 1)
DHDFLAGS += -DWL_CSIMON_CUST_SENDUP
endif
# === END: BCACPE and CMWIFI router build flags ===

# compatibility flags
DHDFLAGS	+= -DOSL_CACHE_NEWAPI_AVAIL

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

# Station Buffer Fairness
DHDFLAGS		+= -DDHD_SBF
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
ifneq ($(CONFIG_DPSTA),)
DHDFLAGS        += -DDHD_DPSTA
endif
DHDFLAGS        += -DDHD_WET
DHDFLAGS        += -DDHD_MCAST_REGEN
DHDFLAGS        += -DSHOW_EVENTS
DHDFLAGS        += -DBCM_CEVENT

DHDFLAGS        += -DBCM_QOSMGMT

ifeq ($(BCM_QOSMGMT_REL), 2)
DHDFLAGS += -DBCM_QOSMGMT_R2
else
DHDFLAGS += -DBCM_QOSMGMT_R1
endif

ifeq ($(CONFIG_STBAP),y)
DHDFLAGS += -DBCM_REQUEST_FW
endif

ifneq ($(CONFIG_LBR_AGGR),)
DHDFLAGS        += -DDHD_LBR_AGGR_BCM_ROUTER
endif

#M2M host memory allocation
ifneq ($(CONFIG_STBAP),y)
DHDFLAGS	+= -DBCM_HOST_MEM_SCB -DDMA_HOST_BUFFER_LEN=0x80000
endif

ifeq ($(BCM_BUZZZ),1)
DHDFLAGS        += -DBCM_BUZZZ
endif

# Idle flowring Eviction
DHDFLAGS        += -DDHD_IFE

ifneq ($(CONFIG_DMA_CMA),)
# Boot Host Memory
DHDBHM		:= 1
DHDFLAGS        += -DDHD_BHM
endif

# WDS flowring expansion build flag
DHDFLAGS	+= -DWDS_FREXP

ifeq ($(BUILD_BCM_WLMLO),y)
    DHDFLAGS        += -DMLO_IPC
    DHDFLAGS        += -DBCMMLO_GR
    DHDFLAGS        += -DMLO_BCMC
    DHDFLAGS        += -DWLMLO_STA
endif

# RX Completion host info of a PDU in the headroom
DHDFLAGS        += -DBCM_RXCPLE_RCH

# Console Log Streaming support in DHD
DHDFLAGS        += -DHNDUCLS

DHDFLAGS        += -DBCM_HMO_EVENT

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
else ifeq ($(BUILD_PRPL_FEEDS),y)
DHDFLAGS	+= -DWL_CFG80211_STRICT -DPRPL_FEEDS
else ifeq ($(RDKB_ONE_WIFI),y)
DHDFLAGS	+= -DWL_CFG80211_STRICT -DRDKB_ONE_WIFI -DWL_CFG80211_ACL
endif
DHDFLAGS        += -DROUTER_CFG
DHDFLAGS        += -DWL_CFG80211
DHDFLAGS        += -DWL_HAPD_WDS
DHDFLAGS        += -DWL_DRV_AVOID_SCANCACHE
DHDFLAGS        += -DWLP2P
DHDFLAGS        += -DSUPPORT_SOFTAP_WPAWPA2_MIXED
DHDFLAGS        += -DMFP
DHDFLAGS	+= -DWLFBT
DHDFLAGS	+= -DWL11U
DHDFLAGS	+= -DWL_DPP
DHDFLAGS	+= -DWL_SPP_AMSDU
DHDFLAGS        += -DWL_OCV
DHDFLAGS        += -DWL_BCN_PROT
DHDFLAGS        += -DWL_MRSNO
ifeq ($(call wlan_version_ge,$(LINUXVERSION),3.14.0),TRUE)
  DHDFLAGS	+= -DWL_VENDOR_EXT_SUPPORT
endif

ifneq ($(CONFIG_STBAP),y)
# For router, SAE is always enabled
DHDFLAGS        += -DWL_SAE
endif
endif

# Enable IOCV logging to mem
DHDFLAGS        += -DBCM_IOCV_MEM_LOG

# DHD Include Paths
DHDIFLAGS       += -I$(SRCBASE)/include
DHDIFLAGS       += -I$(SRCBASE)/../components/shared
DHDIFLAGS       += -I$(SRCBASE)/../components/wlioctl/include
DHDIFLAGS       += -I$(SRCBASE)/../components/proto/include
DHDIFLAGS       += -I$(SRCBASE)/../components/math/include
DHDIFLAGS       += -I$(SRCBASE)/common/include
DHDIFLAGS       += -I$(SRCBASE)/shared
DHDIFLAGS       += -I$(SRCBASE)/shared/bcmwifi/include
DHDIFLAGS       += -I$(SRCBASE)/dhd/sys
DHDIFLAGS       += -I$(SRCBASE)/dongle/include
DHDIFLAGS       += -I$(SRCBASE)/wl/sys
DHDIFLAGS       += -I$(SRCBASE_ROUTER)/bcmdrv/include
ifeq ($(CONFIG_BCM_HOSTAPD),y)
DHDIFLAGS       += -I$(SRCBASE)/../components/clm-api/include
DHDIFLAGS       += -I$(SRCBASE)/wl/ppr/include
DHDIFLAGS       += -I$(SRCBASE)//wl/iocv/include
endif
DHDIFLAGS       += -I$(SRCBASE_ROUTER)/dpsta

# DHD Source files - For pciefd-msgbuf target
DHDFILES_SRC    := src/shared/aiutils.c
DHDFILES_SRC    += src/shared/bcmevent.c
DHDFILES_SRC    += src/shared/bcm_l2_filter.c
DHDFILES_SRC    += src/shared/bcmotp.c
DHDFILES_SRC    += src/shared/bcmsrom.c
DHDFILES_SRC    += src/shared/bcmutils.c
DHDFILES_SRC    += src/shared/bcmstdlib_s.c
DHDFILES_SRC    += src/shared/siutils.c
DHDFILES_SRC    += src/shared/bcm_psta.c
DHDFILES_SRC    += src/dhd/sys/dhd_common.c
DHDFILES_SRC    += src/dhd/sys/dhd_custom_gpio.c
DHDFILES_SRC    += src/dhd/sys/dhd_ip.c
DHDFILES_SRC    += src/dhd/sys/dhd_linux.c
#DHDFILES_SRC    += src/dhd/sys/dhd_linux_platdev.c
DHDFILES_SRC    += src/dhd/sys/dhd_linux_sched.c
DHDFILES_SRC    += src/dhd/sys/dhd_linux_wq.c
DHDFILES_SRC    += src/dhd/sys/dhd_ife.c
DHDFILES_SRC    += src/dhd/sys/dhd_sbf.c
DHDFILES_SRC    += src/dhd/sys/dhd_mlo_ipc.c
DHDFILES_SRC    += src/dhd/sys/dhd_qosmgmt.c
DHDFILES_SRC    += src/wl/sys/wl_core.c
DHDFILES_SRC    += src/wl/sys/wldev_common.c
DHDFILES_SRC    += src/dhd/sys/dhd_linux_mon.c

ifeq ($(BAS_WLHAL),y)
  include $(abspath $(SRCBASE)/../../main/components/bas_hal/wlhal/src/bas_wlhal_dhd_router.mk)
endif

#ifdef  PCIEG3_EP
# pcie gen3 support
DHDFILES_SRC    += $(shell test -e $(SRCBASE)/include/pcieg3_ep.h && echo src/shared/pcieg3_ep.c)
DHDFLAGS        += $(shell test -e $(SRCBASE)/include/pcieg3_ep.h && echo -DPCIEG3_EP)
#endif # PCIEG3_EP

ifeq ($(WL_MONITOR),1)
DHDFLAGS        += -DWL_MONITOR
DHDFILES_SRC    += src/shared/bcmwifi/src/bcmwifi_monitor.c
endif

ifneq ($(WLTEST),1)
ifeq ($(BCM_EVENT_LOG),1)
DHDFLAGS        += -DSHOW_LOGTRACE
DHDFLAGS        += -DEVENT_LOG_ACCESS
DHDFLAGS        += -DEVENT_LOG_SET_BCA_NEW_DEFINITION
DHDFLAGS        += -DEVENT_LOG_FD_STR_VSNPRINTF

ifeq ($(BCM_PHY_PERI_LOG),1)
DHDFLAGS        += -DPHY_PERI_LOG
endif # BCM_PHY_PERI_LOG

DHDFILES_SRC    += src/dhd/sys/dhd_debug.c
DHDFILES_SRC    += src/shared/eldbg_ring.c
DHDFILES_SRC    += src/shared/linux_exportfs.c
DHDFILES_SRC    += src/shared/linux_dbg.c
endif # BCM_EVENT_LOG
endif # !WLTEST

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

DHDFILES_SRC    += src/shared/pcie_core.c

ifeq ($(CONFIG_BCM_HOSTAPD),y)
DHDFILES_SRC    += src/dhd/sys/dhd_cfg80211.c
DHDFILES_SRC    += src/wl/sys/wl_dhd_cfg80211.c
DHDFILES_SRC    += src/wl/sys/wl_cfgp2p.c
DHDFILES_SRC    += src/wl/sys/wl_cfgvendor_common.c
endif

#ifeq ($(FTM_11AZ), 1) /// currently PASN is used only with FTM 11az
DHDFLAGS                += -DWL_PASN
#endif

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
ifneq ($(strip $(CONFIG_BCM_DHD_OFFLOAD)),)
DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_offload.c
else
DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_runner.c
endif
endif

DHDFILES_SRC    += src/shared/linux_osl.c
endif

ifneq ($(strip $(CONFIG_WIFI_RETAIN_ALLOC)),)
	DHDFLAGS += -DCONFIG_WIFI_RETAIN_ALLOC
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

#ifdef WL_EAP_AP
ifneq ($(CONFIG_BCM_HND_EAP),)
	DHDFLAGS += -DDHD_EAP -DDHD_IFE_RECLAIM

	ifeq ($(strip $(DHDBHM)), 1)
		# For EAP, overide default BHM memory size
		DHDFLAGS += -DDHDPCIE_BHM_MEM_SIZE_OVERRIDE
		DHDFLAGS += -DDHDPCIE_BHM_MEM_SIZE_AX=0x400000		#  4MB
		DHDFLAGS += -DDHDPCIE_BHM_MEM_SIZE_AX_PP=0x1700000	# 23MB
		DHDFLAGS += -DDHDPCIE_BHM_MEM_SIZE_BE_PUQ=0x2100000	# 33MB
		DHDFLAGS += -DDHDPCIE_BHM_MEM_SIZE_BE_PUQ_MAP=0x2f00000	# 47MB
	endif # DHDBHM

	# Enable 'phy_refresh' iovar capturing
	# This flag should miror the setting in wl.mk, as Phy Refresh is an option.  See also
	# 'phyrefresh' option in the dongle makefile.top
	DHDFLAGS += -DDHD_PHY_REFRESH
endif
#endif WL_EAP_AP

BCM_PCAP = $(shell if [ -f "$(SRCBASE)/shared/bcmwifi/include/bcmwifi_monitor.h" ]; then echo 1; else echo 0; fi)
ifeq ($(CMWIFI),)
ifeq ($(BCM_PCAP),1)
# enable the BCM_PCAP feature
	DHDFLAGS += -DBCM_PCAP -DWL_MONITOR -DWL11BE
	DHDFILES_SRC += src/shared/bcmwifi/src/bcmwifi_monitor.c
endif
endif

WL_AIRIQ = $(shell if [ -f "$(SRCBASE)/../components/wlioctl/include/wlioctl_airiq.h" ]; then echo 1; else echo 0; fi)
ifeq ($(WL_AIRIQ),1)
# enable the WL_AIRIQ feature
	DHDFLAGS += -DWL_AIR_IQ
endif

BCM_FIS = $(shell if [ -f "$(SRCBASE)/include/sreng_regs.h" ]; then echo 1; else echo 0; fi)
ifeq ($(CMWIFI),)
ifeq ($(BCM_FIS),1)
ifneq ($(CONFIG_STBAP),y)
ifneq ($(NO_BCMINTERNAL),1)
# FIS (Force Immediate Save) Feature
DHDFLAGS += -DDHD_FIS_DUMP
DHDFILES_SRC += src/dhd/sys/dhd_fis.c
endif
endif
endif
endif

ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
    DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_pktfwd.c

ifneq ($(strip $(BUILD_SW_GSO_WLAN)),)
    DHDFLAGS        += -DBCM_CPE_DHD_GSO
    DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_sw_gso.c
endif

endif

ifneq ($(strip $(CONFIG_BCM_ARCHER_WLAN)),)
    DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_awl.c
endif

DHD_OBJS := $(sort $(patsubst %.c,%.o,$(addprefix $(SRCBASE_OFFSET)/,$(patsubst src/%,%,$(DHDFILES_SRC)))))

EXTRA_CFLAGS += $(DHDFLAGS) $(DHDIFLAGS)

ifeq ($(strip $(USE_WLAN_SHARED)), 1)
ifneq ($(strip $(WLAN_SHARED_DIR)),)
ifneq ($(strip $(BCA_HNDROUTER)),)
EXTRA_CFLAGS += -DBCM_COUNTER_EXTSTATS
ifneq ($(strip $(CONFIG_BCM_KF_EXTSTATS)),)
EXTRA_CFLAGS += -DBCM_CPEROUTER_EXTSTATS
endif
endif
endif
endif

# This comment block must stay at the bottom of the file.
# Local Variables:
# mode: makefile-gmake
# fill-column: 80
# End:
#
# vim: filetype=make sw=2 tw=80 cc=+1 noet
