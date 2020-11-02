# Makefile for hndrte based 43xx standalone programs,
#	to generate romtable.S for 43xx ROM builds
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
# $Id: 43xx.mk 254324 2011-04-20 06:12:25Z $:

# chip specification
CHIP		:= 4334
REV		:= a0
REVID		:= 0

# Note on rebuilding romtable.S
#	make 43xx
#	cp builds/43xx/sdio/romtable_full.S ../roml/43xx/romtable.S
TARGETS		:= sdio

# common target attributes
TARGET_HBUS	:= sdio usb
TARGET_ARCH	:= arm
TARGET_CPU	:= cm3
THUMB		:= 1
HBUS_PROTO 	:= cdc
BAND		:= ag

# wlconfig & wltunable
WLCONFFILE	:= wlconfig_rte_43xx_dev
WLTUNEFILE	:= wltunable_rte_4334a0.h

# ROMCTL needed for location of romctl.txt
ROMCTL		:= $(TOPDIR)/../roml/4334sim-43xx/romctl.txt

# features (sync with 43xx.mk, 4334a0-romlsim-43xx.mk)
SMALL		:= 0
MEMSIZE		:= 524288	# Hardcoding it saves ~112 bytes from startarm.S
FLASH		:= 0
MFGTEST		:= 1
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ERROR	:= 1
BCMPKTPOOL	:= 1
DMATXRC		:= 1
WLRXOV		:= 1
PROP_TXSTATUS	:= 1

POOL_LEN_MAX	:= 60

# p2p dongle code support
VDEV		:= 1

# extra flags
EXTRA_DFLAGS	+= -DSHARE_RIJNDAEL_SBOX	# Save 1400 bytes; wep & tkhash slightly slower
EXTRA_DFLAGS	+= -DWLAMSDU_TX -DAMPDU_RX_BA_DEF_WSIZE=16
ifneq ($(ROMLIB),1)
ROMBUILD	:= 1
EXTRA_DFLAGS	+= -DBCMROMSYMGEN_BUILD
endif
