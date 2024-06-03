# Helper makefile for building Broadcom dongle host driver (DHD) for
# router platforms
# This file maps dhd feature flags DHDFLAGS(import) and DHDFILES_SRC(export).
#
# Copyright (C) 2023, Broadcom. All Rights Reserved.
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
# $Id: dhd_router.mk 831871 2023-10-27 05:30:23Z $

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
DHDFLAGS += -DDHD_USE_PERIM
endif # CMWIFI

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
# === END: BCACPE and CMWIFI router build flags ===

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
DHDFLAGS        += -DDHD_SBF
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

DHDFLAGS        += -DMLO_IPC
DHDFLAGS        += -DBCMMLO_GR
DHDFLAGS        += -DMLO_BCMC

ifeq ($(BUILD_BCM_WLMLO),y)
    DHDFLAGS += -DWLMLO_STA
endif

# RX Completion host info of a PDU in the headroom
DHDFLAGS        += -DBCM_RXCPLE_RCH

# Console Log Streaming support in DHD
DHDFLAGS        += -DHNDUCLS

#DHDFLAGS        += -DBCM_HMO_EVENT

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
DHDFLAGS	+= -DWL_CFG80211_STRICT
else ifeq ($(RDKB_ONE_WIFI),y)
DHDFLAGS	+= -DWL_CFG80211_STRICT -DRDKB_ONE_WIFI
endif
DHDFLAGS        += -DROUTER_CFG
DHDFLAGS        += -DWL_CFG80211
DHDFLAGS        += -DWL_HAPD_WDS
DHDFLAGS        += -DUSE_CFG80211
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

#ifdef  PCIEG3_EP
# pcie gen3 support
DHDFILES_SRC    += $(shell test -e $(SRCBASE)/include/pcieg3_ep.h && echo src/shared/pcieg3_ep.c)
DHDFLAGS        += $(shell test -e $(SRCBASE)/include/pcieg3_ep.h && echo -DPCIEG3_EP)
#endif # PCIEG3_EP

ifeq ($(WL_MONITOR),1)
DHDFLAGS        += -DWL_MONITOR
DHDFILES_SRC    += src/shared/bcmwifi/src/bcmwifi_monitor.c
endif

ifeq ($(BCM_EVENT_LOG),1)
DHDFLAGS        += -DSHOW_LOGTRACE
DHDFLAGS        += -DEVENT_LOG_ACCESS

ifeq ($(BCM_PHY_PERI_LOG),1)
DHDFLAGS        += -DPHY_PERI_LOG
endif # BCM_PHY_PERI_LOG

DHDFILES_SRC    += src/dhd/sys/dhd_debug.c
DHDFILES_SRC    += src/shared/eldbg_ring.c
DHDFILES_SRC    += src/shared/linux_exportfs.c
endif # BCM_EVENT_LOG

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
DHDFILES_SRC    += src/wl/sys/wl_dhd_cfg80211.c
DHDFILES_SRC    += src/wl/sys/wl_cfgp2p.c
DHDFILES_SRC    += src/wl/sys/wl_cfgvendor_common.c
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
	DHDFLAGS += -DDHD_EAP

	ifeq ($(strip $(DHDBHM)), 1)
		# For EAP, overide default BHM memory size
		DHDFLAGS += -DDHDPCIE_BHM_MEM_SIZE_OVERRIDE
		DHDFLAGS += -DDHDPCIE_BHM_MEM_SIZE_AX=0x400000		#  4MB
		DHDFLAGS += -DDHDPCIE_BHM_MEM_SIZE_AX_PP=0x1600000	# 22MB
		DHDFLAGS += -DDHDPCIE_BHM_MEM_SIZE_BE=0x1C00000		# 28MB
		DHDFLAGS += -DDHDPCIE_BHM_MEM_SIZE_BE_PUQ=0x1F00000	# 31MB
		DHDFLAGS += -DDHDPCIE_BHM_MEM_SIZE_BE_PUQ_MAP=0x2D00000	# 45MB
	endif # DHDBHM
endif
#endif WL_EAP_AP

BCM_PCAP = $(shell if [ -f "$(SRCBASE)/../../$(SRCBASE_DHD)/shared/bcmwifi/include/bcmwifi_monitor.h" ]; then echo 1; else echo 0; fi)
#ifndef CMWIFI
ifeq ($(CMWIFI),)
ifeq ($(BCM_PCAP),1)
# enable the BCM_PCAP feature
	DHDFLAGS += -DBCM_PCAP -DWL_MONITOR
	DHDFILES_SRC += src/shared/bcmwifi/src/bcmwifi_monitor.c
endif
endif
#endif /* CMWIFI */

#ifdef CMWIFI
ifneq ($(CMWIFI),)
$(info ******************* CMWIFI DHD enabled BRCM_CHIP=$(BRCM_CHIP) TARGETARCH=$(TARGETARCH) *******************)
$(info *** LINUXDIR=$(LINUXDIR))
$(info *** CMWIFI_EROUTER=$(CMWIFI_EROUTER) CMWIFI_EROUTER_RFAP=$(CMWIFI_EROUTER_RFAP) CMWIFI_RDKB=$(CMWIFI_RDKB) RDKB=$(RDKB))
$(info *** CMWIFI_33940=$(CMWIFI_33940))
$(info *** CONFIG_BCM_DHD_GFAP=$(CONFIG_BCM_DHD_GFAP))
	DHDFLAGS += -DCMWIFI=1
	DHDFLAGS += -DMAX_WLAN_ADAPTER=$(MAX_WIFI_CARDS)
	ifeq ($(CMWIFI_RDKB),y)
		DHDFLAGS += -DCMWIFI_RDKB=1
	endif
	ifeq ($(RDKB),y)
		DHDFLAGS += -DRDKB=1
	endif
	ifneq ($(filter arm%,$(TARGETARCH)),)
		DHDFLAGS += -D__LINUX_ARM_ARCH__=7 -march=armv7-a
		ifeq ($(BRCM_CHIP),3390)
			DHDFLAGS += -DCM3390
		endif
	endif

	ifeq ($(call KERNEL_GE,3.8.0),TRUE)
		DHDIFLAGS += -I$(LINUXDIR)/include/uapi
		DHDIFLAGS += -I$(LINUXDIR)/include/generated/uapi
		DHDIFLAGS += -I$(LINUXDIR_INC_BASE)/include/generated/
	endif
	DHDIFLAGS += -I$(LINUXDIR_INC_BASE)/include/asm/mach-brcmstb
	DHDIFLAGS += -I$(LINUXDIR_INC_BASE)/include/asm/mach-generic

	DHDFILES_SRC += src/shared/linux_osl.c
	DHDFILES_SRC += src/shared/linux_pkt.c
	DHDFILES_SRC += ../../cmwifi/mods/shared/cmwifi_srom.c
	ifneq ($(CMWIFI_EROUTER),)
		DHDFLAGS += -DCMWIFI_EROUTER
		DHDIFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/include
		DHDIFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/dqm
		DHDIFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/dqnet
		DHDIFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/fpm
		DHDIFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/mdqm
		ifneq ($(strip $(CONFIG_BCM_DHD_GFAP)),)
			DHDFLAGS += -DRDPA_DHD_HELPER_FEATURE_HBQD_SUPPORT
		endif
		DHDFLAGS += -DPKTC
		ifeq ($(WLEROUTER_AS_MODULE),)
			DHDFILES_SRC += ../../cmwifi/mods/wlerouter/src/dhd_erouter_pktc.c
		else
			DHDFLAGS += -DWLEROUTER_AS_MODULE
		endif
		ifneq ($(CMWIFI_EROUTER_RFAP),)
			DHDFLAGS += -DCMWIFI_EROUTER_RFAP
			DHDIFLAGS += -I$(EXTMODDIR)/runner/drivers/dhd/sys
			DHDIFLAGS += -I$(EXTMODDIR)/runner/drivers/rdpa_gpl/include
			DHDIFLAGS += -I$(EXTMODDIR)/runner/drivers/bdmf/framework
			DHDIFLAGS += -I$(EXTMODDIR)/runner/drivers/bdmf/system
			DHDIFLAGS += -I$(EXTMODDIR)/runner/drivers/bdmf/system/linux
			DHDIFLAGS += -I$(EXTMODDIR)/runner/drivers/rdp_subsystem
			DHDIFLAGS += -I$(EXTMODDIR)/runner/drivers/rdp_subsystem/BCM3390
			DHDIFLAGS += -I$(EXTMODDIR)/runner/projects/CM3390/drivers/rdd
			ifeq ($(WLEROUTER_AS_MODULE),)
				ifeq ($(CM_BUILDROOT),)
					DHDFILES_SRC += ../../cmwifi/mods/wlerouter/src/dhd_erouter_rfap_legacy.c
				else
					DHDFILES_SRC += ../../cmwifi/mods/wlerouter/src/dhd_erouter_rfap.c
				endif
			endif
		endif
		ifneq ($(strip $(CONFIG_BCM_DHD_RUNNER)),)
			DHDFLAGS += -DBCM_DHD_TX_ON_RUNNER
			DHDFLAGS += -DBCM_DHD_RX_ON_RUNNER
			ifneq ($(CMWIFI_WMF_IPV6), )
				DHDFLAGS += -DCMWIFI_WMF_IPV6
			endif
			DHDFLAGS += -DDHD_D2H_SOFT_DOORBELL_SUPPORT
			ifeq ($(strip $(CONFIG_BCM_DHD_GFAP)),)
				DHDIFLAGS += -I$(EXTMODDIR)/runner/drivers/bdmf/system/linux/cm
				DHDIFLAGS += -I$(EXTMODDIR)/runner/drivers/rdpa_gpl/include/autogen
				DHDIFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/runner/drivers/dhd/sys/
			endif
			ifneq ($(strip $(CONFIG_BCM_DHD_GFAP)),)
				DHDFILES_SRC    += $(WLAN_DHD_PATH)/r2g_adapter.c
				DHDIFLAGS += -I$(EXTMODDIR)/driver/dhd/sys/
				DHDIFLAGS += -I$(EXTMODDIR)/include/339x/
				DHDIFLAGS += -I$(EXTMODDIR)/api/339x/
			endif
			DHDFILES_SRC += src/dhd/sys/dhd_cmwifi.c
		endif
		DHDIFLAGS += -I$(LINUXDIR)/include/linux
		DHDIFLAGS += -Wno-vla
	endif #CMWIFI_EROUTER
	ifdef CMWIFI_33940
		DHDIFLAGS += -DCMWIFI_33940 -DBCMDMA64OSL
	endif #CMWIFI_33940
endif # CMWIFI
#endif
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
