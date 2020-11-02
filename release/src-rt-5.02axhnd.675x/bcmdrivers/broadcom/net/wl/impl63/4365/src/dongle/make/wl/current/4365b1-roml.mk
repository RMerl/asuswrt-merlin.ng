#
# Makefile for hndrte based 4365b1 ROM Offload image building
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# Copyright 2020 Broadcom
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
# $Id: 4365b1-roml.mk$
#

####################################################################################################
# This makefile is used when building a ROM offload image, so an image that runs in RAM and calls
# routines that reside in ROM. It is not used when building a ROM. Default settings are defined in
# the 'wlconfig', settings in there may be redefined in this file when a 'default' ROM offload image
# should support more or less features than the ROM.
####################################################################################################

# chip specification
CHIP		:= 4365
ROMREV		:= b1
REV		:= b1
REVID		:= 3
# default targets
TARGETS		:= \
	pcie-ag-err-assert-splitrx \
	pcie-ag-mfgtest-seqcmds-splitrx-phydbg \
	pcie-ag-mfgtest-seqcmds-splitrx-phydbg-err-assert \
	pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-splitrx \
	pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-splitrx-err-assert \
	pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-ndoe-pktfilter-pf2-keepalive-splitrx-toe-ccx-mfp-anqpo-p2po-wl11k-wl11u-wnm-relmcast-txbf-fbt-tdls-sr-pktctx-amsdutx-proxd \
	pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-ndoe-pktfilter-pf2-keepalive-splitrx-toe-ccx-mfp-anqpo-p2po-wl11k-wl11u-wnm-relmcast-txbf-fbt-tdls-sr-pktctx-amsdutx-proxd-err-assert \
	pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-wnm-txbf-pktctx-amsdutx-ampduretry-proptxstatus \
	pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-wnm-txbf-pktctx-amsdutx-ampduretry-proptxstatus-err-assert \
	pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-msgbuf-ndis-vista-dhdoid-pktfilter-mfp-keepalive

TEXT_START	:= 0x200000
DATA_START	:= 0x200100

# 1.2MB ROM
ROM_LOW		?= 0x00000000
ROM_HIGH	?= 0x00133333

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= ca7
TARGET_HBUS	:= pcie
THUMB		:= 1
HBUS_PROTO	:= msgbuf

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_4365b1_dev
WLTUNEFILE	:= wltunable_rte_4365b1.h

# ROM image info
ROMOFFLOAD	:= 1
ROMLDIR		:= $(TOPDIR)/../../../../components/chips/images/roml/$(CHIP)$(ROMREV)
ROMLLIB		:= roml.exe

# Use TCAM to patch ROM functions
TCAM		:= 1
JMPTBL_TCAM	:= 1
GLOBALIZE	:= 1
TCAM_PCNT	:= 2
TCAM_SIZE	:= 512

# features (sync with romlsym/4365b0.mk)
MEMBASE     := $(TEXT_START)
MEMSIZE		:= 2097152
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ASSERT_TRAP	:= 1
DBG_ERROR	:= 1
DBG_TEMPSENSE	:= 1
WLRXOV		:= 0
PROP_TXSTATUS	:= 1

# Memory reduction features:
# - HNDLBUFCOMPACT: Compacts head/end pointers in lbuf to single word
#   To disable set HNDLBUFCOMPACT = 0
# - BCMPKTIDMAP: Suppresses pkt pointers to Ids in lbuf<next,link>, pktpool, etc
#   Must specify max number of packets (various pools + heap)

HNDLBUFCOMPACT	:= 1
BCMPKTIDMAP     := 1
BCMFRAGPOOL	:= 1
BCMRXFRAGPOOL	:= 1
BCMLFRAG	:= 1
BCMSPLITRX	:= 1

POOL_LEN_MAX    := 576
POOL_LEN        := 32

WL_POST_FIFO1   := 2
MFGTESTPOOL_LEN := 10
FRAG_POOL_LEN	:= 576
RXFRAG_POOL_LEN	:= 320

# Rx throughput improves with 32 byte aligned DMAs.
# So, make sure MAXPKTRXFRAGSZ - LBUFFRAGSZ is a multiple of 32
# When HNDLBUFCOMPACT=1,  LBUFFFRAGSZ is 92
# When HNDLBUFCOMPACT=0,  LBUFFFRAGSZ is 96
ifeq ($(HNDLBUFCOMPACT), 1)
EXTRA_DFLAGS	+= -DMAXPKTRXFRAGSZ=252
else
EXTRA_DFLAGS	+= -DMAXPKTRXFRAGSZ=256
endif

PKT_MAXIMUM_ID  := 1024

H2D_DMAQ_LEN	:= 256
D2H_DMAQ_LEN	:= 256

PCIE_NTXD	:= 256
PCIE_NRXD	:= 256

WL_POST_CLASSIFIED_FIFO := 4
WL_SPLITRX_MODE := 2
WL_CLASSIFY_FIFO := 2
ifeq ($(WL_SPLITRX_MODE),2)
EXTRA_DFLAGS    += -DFORCE_RX_FIFO1
endif

ifeq ($(findstring ate,$(TARGET)),ate)
        TARGET_HBUS     := sdio
endif

ifeq ($(findstring mfgtest,$(TARGET)),mfgtest)
	#allow MFG image to write OTP
	BCMNVRAMR	:= 0
	BCMNVRAMW	:= 1
endif

ifeq ($(findstring fdap,$(TARGET)),fdap)
	#router image, enable router specific features
	CLM_TYPE			:= 4365a0_access

	AP				:= 1
	SUFFIX_ENAB			:= 1
	INITDATA_EMBED			:= 1
	WLC_DISABLE_DFS_RADAR_SUPPORT	:= 0
	# Max MBSS virtual slave devices
	MBSS_MAXSLAVES			:= 8
	# Max Tx Flowrings
	PCIE_TXFLOWS			:= 264
	WLBSSLOAD			:= 1
	WLOSEN				:= 1
	WLPROBRESP_MAC_FILTER		:= 1
	# In router, we would "never" be able to support flowring per tid.
	EXTRA_DFLAGS		+= -DFLOW_PRIO_MAP_AC

	EXTRA_DFLAGS	+= -DPKTC_FDAP

	# Memory optimizations by avoiding unnecessary abandons
	# by disabling non-router related post-ROM code
	#EXTRA_DFLAGS    += -DNO_ROAMOFFL_SUPPORT
	EXTRA_DFLAGS    += -DNO_WLNDOE_RA_SUPPORT

	EXTRA_DFLAGS	+= -DPROP_TXSTATUS_SUPPR_WINDOW
	# Enable Probe response intransit filter, to limit to 1 probe response per station DA.
	EXTRA_DFLAGS   += -DWLPROBRESP_INTRANSIT_FILTER

else
	# CLM info
	CLM_TYPE	:= 4365a0

	NAR_MAX_TRANSIT_PACKETS		:= 128
endif

ifeq ($(findstring wlcx,$(TARGET)),wlcx)
	WLCX_ATLAS	:= 1
endif

# Reduce stack size to increase free heap
HNDRTE_STACK_SIZE	:= 4608
EXTRA_DFLAGS		+= -DHNDRTE_STACK_SIZE=$(HNDRTE_STACK_SIZE)

EXTRA_DFLAGS	+= -DBCMPKTPOOL_ENABLED

# Add flops support
FLOPS_SUPPORT	:= 1

#Enable GPIO
EXTRA_DFLAGS	+= -DWLGPIOHLR

TOOLSVER	:= 2013.11
NOFNRENAME	:= 1

# Hard code some PHY characteristics to reduce RAM code size
# RADIO
EXTRA_DFLAGS	+= -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS	+= -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS	+= -DBCMRADIOID=$(BCMRADIOID)
# Only support EPA
EXTRA_DFLAGS	+= -DWLPHY_EPA_ONLY -DEPA_SUPPORT=1
# Don't support PAPD
EXTRA_DFLAGS	+= -DEPAPD_SUPPORT=0 -DWLC_DISABLE_PAPD_SUPPORT -DPAPD_SUPPORT=0

EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=64
EXTRA_DFLAGS	+= -DPKTC_DONGLE
EXTRA_DFLAGS	+= -DPCIEDEV_USE_EXT_BUF_FOR_IOCTL
PCIEDEV_MAX_IOCTLRSP_BUF_SIZE := 8192

# Ideal: (MAX_HOST_RXBUFS > (RXFRAG_POOL_LEN + POOL_LEN)); At least (MAX_HOST_RXBUFS > (WL_POST + WL_POST_FIFO1)) for pciedev_fillup_rxcplid callback from pktpool_get
# Also increase H2DRING_RXPOST_MAX_ITEM to match WL_POST
EXTRA_DFLAGS	+= -DMAX_HOST_RXBUFS=512

#wowl gpio pin 14, Polarity at logic low is 1
WOWL_GPIOPIN	:= 0xe
WOWL_GPIO_POLARITY := 0x1
EXTRA_DFLAGS    += -DWOWL_GPIO=$(WOWL_GPIOPIN) -DWOWL_GPIO_POLARITY=$(WOWL_GPIO_POLARITY)

# max fetch count at once
EXTRA_DFLAGS    += -DPCIEDEV_MAX_PACKETFETCH_COUNT=64
EXTRA_DFLAGS	+= -DPCIEDEV_MAX_LOCALBUF_PKT_COUNT=400
EXTRA_DFLAGS	+= -DMAX_TX_STATUS_BUF_LEN=368
EXTRA_DFLAGS	+= -DMAX_RXCPL_BUF_LEN=320
# PD_NBUF_H2D_RXPOST * items(32) > MAX_HOST_RXBUFS for pciedev_fillup_haddr=>pciedev_get_host_addr callback from pktpool_get
EXTRA_DFLAGS    += -DPD_NBUF_H2D_RXPOST=16
EXTRA_DFLAGS    += -DMAX_TX_STATUS_QUEUE=256
EXTRA_DFLAGS    += -DMAX_TX_STATUS_COMBINED=128

# Size of local queue to store completions
EXTRA_DFLAGS    += -DPCIEDEV_CNTRL_CMPLT_Q_SIZE=16

# RxOffsets for the PCIE mem2mem DMA
EXTRA_DFLAGS    += -DH2D_PD_RX_OFFSET=0
EXTRA_DFLAGS    += -DD2H_PD_RX_OFFSET=0

# Set deadman timeout to 5 seconds,
EXTRA_DFLAGS    += -DDEADMAN_TIMEOUT=5000000

# Dual Pkt AMSDU optimization
EXTRA_DFLAGS += -DDUALPKTAMSDU

#Support for SROM format
EXTRA_DFLAGS	+= -DBCMPCIEDEV_SROM_FORMAT

#Support for STBC
EXTRA_DFLAGS	+= -DWL11N_STBC_RX_ENABLED

# Disabled CCA_STATS post tapeout, need to fix up structures
EXTRA_DFLAGS	+= -DCCA_STATS_IN_ROM
EXTRA_DFLAGS	+= -DISID_STATS_IN_ROM

# ROM compatibility with legacy version of wlc_chanim_stats_t.
EXTRA_DFLAGS	+= -DWLC_CHANIM_STATS_ROM_COMPAT2

# Flags to relocate struct fields and enum values that were excluded in ROMs,
# but are required in ROM offload builds.
EXTRA_DFLAGS	+= -DPROP_TXSTATUS_ROM_COMPAT -DWLMCHANPRECLOSE_ROM_COMPAT

#Flag to relocate struct fields that were mismatched in ROM symbol stat table
EXTRA_DFLAGS    += -DWLBSSLOAD_ROM_COMPACT

# ROM compatibility with legacy SGI features
EXTRA_DFLAGS	+= -DSGI_PROBE_ROM_COMPAT
EXTRA_DFLAGS	+= -DWL_RELMCAST_DISABLED -DP2PO_DISABLED -DANQPO_DISABLED

ifeq ($(findstring vista,$(TARGET)),vista)
# 4365B1 Rom compatibility. ROM was built without EXT_STA
EXTRA_DFLAGS	+= -DEXT_STA_RELOC
EXTRA_DFLAGS    += -DAP_TKIP_SW_WSEC
endif

# Support for sliding window within flowrings
# This allows an option to set a large flowring size, but operate in a sliding
# window model where dongle only consumes packets upto the window size.
EXTRA_DFLAGS    += -DFLOWRING_SLIDING_WINDOW -DFLOWRING_SLIDING_WINDOW_SIZE=512

# Support for using smaller bitsets on each flowring - instead of the full flowring depth
EXTRA_DFLAGS    += -DFLOWRING_USE_SHORT_BITSETS

# Disable WET/WET TUNNEL. These features cannot be supported in dongle.
EXTRA_DFLAGS    += -DWET_DISABLED -DWET_TUNNEL_DISABLED

# PHYCAL_CACHING : Limit max number of chanctx in dongle to avoid oom.
EXTRA_DFLAGS += -DDONGLE_MAX_CAL_CACHE=5

# ROM compatibility with legacy version of wlc_l2_filter.
EXTRA_DFLAGS += -DWLC_L2_FILTER_ROM_COMPAT

# Disable/enable AMSDU for AC_VI
EXTRA_DFLAGS    += -DDISABLE_AMSDUTX_FOR_VI

# Instead of disabling frameburst completly in dynamic frame burst logic, we enable RTS/CTS in frameburst.
EXTRA_DFLAGS    += -DFRAMEBURST_RTSCTS_PER_AMPDU

# To tune frameburst override thresholds
EXTRA_DFLAGS    += -DTUNE_FBOVERRIDE
