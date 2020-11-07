# File: bcmdrivers/char/bcm963xx/adsl/Makefile
#
# Makefile for the BCM63xx ADSL connection driver.
#
#***********************************************************************
#
#  Copyright (c) 2006-2012  Broadcom Corporation
#  All Rights Reserved
#
# <:label-BRCM:2012:proprietary:standard
# 
#  This program is the proprietary software of Broadcom and/or its
#  licensors, and may only be used, duplicated, modified or distributed pursuant
#  to the terms and conditions of a separate, written license agreement executed
#  between you and Broadcom (an "Authorized License").  Except as set forth in
#  an Authorized License, Broadcom grants no license (express or implied), right
#  to use, or waiver of any kind with respect to the Software, and Broadcom
#  expressly reserves all rights in and to the Software and all intellectual
#  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
#  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
#  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
# 
#  Except as expressly set forth in the Authorized License,
# 
#  1. This program, including its structure, sequence and organization,
#     constitutes the valuable trade secrets of Broadcom, and you shall use
#     all reasonable efforts to protect the confidentiality thereof, and to
#     use this information only in connection with your use of Broadcom
#     integrated circuit products.
# 
#  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
#     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
#     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
#     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
#     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
#     PERFORMANCE OF THE SOFTWARE.
# 
#  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
#     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
#     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
#     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
#     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
#     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
#     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
#     LIMITED REMEDY.
# :>
#
#***********************************************************************/

override obj := $(abspath $(obj))
obj-$(CONFIG_BCM_ADSL) += adsldd.o

softdsl-objs += softdsl/SoftDslFrame.o softdsl/HdlcFramer.o softdsl/HdlcByte.o softdsl/G997.o 
softdsl-objs += softdsl/Flatten.o softdsl/DslFramer.o softdsl/AdslMibObj.o softdsl/AdslMib.o softdsl/G992p3OvhMsg.o

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_63158_63178_63146_),_$(strip $(BRCM_CHIP))_)
softdsl-objs += softdsl/BlockUtil.o
else
softdsl-objs += softdsl/BlockShortMove.o softdsl/BlockLongMove.o softdsl/BlockByteMove.o softdsl/BlockByteFill.o softdsl/ASSERT_SIZE.o
endif

adsldd-objs += adsl.o AdslCore.o AdslCoreFrame.o BcmAdslCore.o
adsldd-objs += $(softdsl-objs)


ifeq "$(BRCM_PHY_CO)" "y"
adsldd-objs += hmimsg/Core.o hmimsg/LineMgr.o hmimsg/bcm_hmiLineMsg.o hmimsg/bcm_DrvXface.o hmimsg/bcm_layout.o
EXTRA_CFLAGS += -DBCM_PACKING_ATTRIBUTE="__attribute__ ((packed))"
EXTRA_CFLAGS += -DSUPPORT_HMI
endif

ifeq "$(CONFIG_OS)" "OSNONE"
ADSL_PHY_MODE = nofile
EXTRA_CFLAGS += -D_NOOS
adsldd-objs += adsldrv_noos.o BcmAdslDiagNone.o BcmOs_None.o
else
adsldd-objs += adsldrv.o BcmAdslDiagCommon.o BcmAdslDiagLinux.o BcmOs.o AdslFile.o
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_6368_63268_63381_63138_63148_63158_63178_63146_),_$(strip $(BRCM_CHIP))_)
ifneq "$(BRCM_PHY_CO)" "y"
adsldd-objs += BcmXdslVectLinux.o
endif
endif
endif

#EXTRA_CFLAGS += -DSUPPORT_PHY_BIN_FROM_TMP
#EXTRA_CFLAGS += -g3

# Set only when building test images
#BRCM_xDSL_GDB_SUPPORT=y

ifeq "$(BRCM_xDSL_GDB_SUPPORT)" "y"

ifeq "$(findstring 63146,$(BRCM_CHIP))" ""
adsldd-objs += BcmAdslGdbStub.o
else
adsldd-objs += BcmAdslArmGdbStub.o
endif

EXTRA_CFLAGS += -DSUPPORT_XDSLDRV_GDB
endif

ifeq "$(BRCM_IKOS)" "y"
CONFIG_PARSER_ENABLE = y
ADSL_PHY_MODE = nofile
#EXTRA_CFLAGS += -DXDSL_DRV_STATUS_POLLING
EXTRA_CFLAGS += -DXDSLDRV_ENABLE_MIBPRINT
endif

ifeq ($(strip $(BRCM_DRIVER_XTM)),)
EXTRA_CFLAGS += -DNO_XTM_MODULE
endif

ifeq "$(CONFIG_PARSER_ENABLE)" "y"
adsldd-objs += softdsl/StatusParser.o softdsl/DiagsCommon.o
EXTRA_CFLAGS += -DXDSLDRV_ENABLE_PARSER
endif

ifneq "$(BRCM_VERSION)" "3"
ifneq "$(CONFIG_PARSER_ENABLE)" "y"
EXTRA_CFLAGS += -Werror -Wfatal-errors # NEW_FORBID_WARNINGS
endif
endif

ifeq "$(BRCM_PHY_BONDING)" "y"
EXTRA_CFLAGS += -DSUPPORT_DSL_BONDING
endif

ifeq "$(BRCM_PHY_BONDING5B)" "y"
EXTRA_CFLAGS += -DSUPPORT_DSL_BONDING5B
endif

ifeq "$(BRCM_PHY_BONDING_C0)" "y"
EXTRA_CFLAGS += -DSUPPORT_DSL_BONDING_C0
endif

ifeq "$(BRCM_MULTI_PHY)" "y"
EXTRA_CFLAGS += -DSUPPORT_MULTI_PHY
endif

ifeq "$(BUILD_DSL_SELT_TEST)" "y"
EXTRA_CFLAGS += -DSUPPORT_SELT
endif

ifeq "$(ADSL_PHY_MODE)" "file"
EXTRA_CFLAGS += -DADSL_PHY_FILE
endif

ifeq "$(BRCM_CHIP)" "6338"
BRCM_CHIP=6348
endif
ifeq "$(BRCM_CHIP)" "6358"
BRCM_CHIP=6348
endif

ifeq "$(BRCM_CHIP)" "6328"
BRCM_CHIP=6362
endif

ifeq ($(strip $(BRCM_NTR_SUPPORT)),1)
EXTRA_CFLAGS += -DNTR_SUPPORT
endif

ifeq "$(ADSL)" "ANNEX_C"
phy_annex = C
EXTRA_CFLAGS += -DADSL_ANNEXC
HOST_EXTRACFLAGS += -DADSL_ANNEXC
endif
ifeq "$(ADSL)" "ANNEX_B"
phy_annex = B
EXTRA_CFLAGS += -DADSL_ANNEXB
HOST_EXTRACFLAGS += -DADSL_ANNEXB
endif
ifeq "$(ADSL)" "SADSL"
phy_annex = SA
EXTRA_CFLAGS += -DADSL_SADSL
HOST_EXTRACFLAGS += -DADSL_SADSL
endif
ifeq "$(ADSL)" "ANNEX_A"
phy_annex =
endif
ifeq "$(BRCM_ANNEXAB_COMBO)" "y"
phy_annex0 = B
EXTRA_CFLAGS += -DSUPPORT_ANNEXAB_COMBO
endif

ifeq "$(BRCM_CHIP)" "6318"
EXTRA_CFLAGS += -DSUPPORT_STATUS_BACKUP
endif

ifeq "$(BRCM_CHIP)" "6362"
EXTRA_CFLAGS += -DSUPPORT_STATUS_BACKUP
endif

ifeq "$(BRCM_CHIP)" "6368"
EXTRA_CFLAGS += -DSUPPORT_STATUS_BACKUP -DCONFIG_VDSL_SUPPORTED -DUSE_6306_CHIP
ifeq "$(BRCM_EXT_PHY_BONDING)" "y"
EXTRA_CFLAGS += -DSUPPORT_EXT_DSL_BONDING_SLAVE -DSUPPORT_2CHIP_BONDING
endif
ifeq "$(BUILD_PHY_ADSL)" "y"
phy_mode = ao
endif
endif

ifeq "$(BRCM_CHIP)" "63268"
EXTRA_CFLAGS += -DSUPPORT_STATUS_BACKUP -DLMEM_ACCESS_WORKAROUND -DCONFIG_VDSL_SUPPORTED -DUSE_6306_CHIP -DNTR_SUPPORT
ifeq "$(BRCM_EXT_PHY_BONDING)" "y"
EXTRA_CFLAGS += -DSUPPORT_EXT_DSL_BONDING_MASTER -DSUPPORT_2CHIP_BONDING -DSUPPORT_DSL_BONDING
endif
ifeq "$(BUILD_PHY_ADSL)" "y"
phy_mode = ao
endif
endif

ifneq "$(findstring 63158,$(BRCM_CHIP))$(findstring 63178,$(BRCM_CHIP))$(findstring 63146,$(BRCM_CHIP))" ""
EXTRA_CFLAGS += -DUSE_RESERVE_SHARE_MEM
endif

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_63381_63138_63148_63158_63178_63146_),_$(strip $(BRCM_CHIP))_)
EXTRA_CFLAGS += -DSUPPORT_STATUS_BACKUP -DCONFIG_VDSL_SUPPORTED -DUSE_PMC_API -DUSE_PINMUX_DSLCTL_API -DNTR_SUPPORT
endif

ifneq "$(findstring 63138,$(BRCM_CHIP))$(findstring 63148,$(BRCM_CHIP))$(findstring 63158,$(BRCM_CHIP))$(findstring 63178,$(BRCM_CHIP))" ""
EXTRA_CFLAGS += -DADSLDRV_LITTLE_ENDIAN
endif

ifeq "$(BRCM_PHY_GFAST)" "y"
HOST_EXTRACFLAGS += -DCONFIG_BCM_DSL_GFAST
ifeq "$(BRCM_PHY_GFASTCOMBO)" "y"
HOST_EXTRACFLAGS += -DCONFIG_BCM_DSL_GFASTCOMBO
EXTRA_CFLAGS += -DCONFIG_BCM_DSL_GFASTCOMBO
adsldd-objs  += BcmGfastCfg.o
else
ifneq "$(BRCM_ANNEXAB_COMBO)" "y"
phy_annex0 = F
endif
endif
endif

ifeq "$(BRCM_PHY_BONDING)$(findstring 63138,$(BRCM_CHIP))$(findstring 63148,$(BRCM_CHIP))$(findstring 63158,$(BRCM_CHIP))$(findstring 63146,$(BRCM_CHIP))" "y"
phy_bnd = bnd
endif
ifeq "$(BRCM_PHY_BONDING5B)" "y"
phy_bnd = bnd5
endif
ifeq "$(BRCM_PHY_BONDING_C0)" "y"
phy_bnd0 = bnd
endif

adslphy := adslcore$(BRCM_CHIP)$(phy_annex)$(phy_mode)$(phy_bnd)
ifeq "$(BRCM_PHY_BONDING_C0)" "y"
adslphy0 := adslcore$(BRCM_CHIP)$(phy_annex)$(phy_mode)$(phy_bnd0)
else
adslphy0 := adslcore$(BRCM_CHIP)$(phy_annex)$(phy_annex0)
endif
adslphy1 := adslcore$(BRCM_CHIP)$(phy_annex)$(phy_annex1)

ifeq "$(ADSL_SELF_TEST)" "y"
adslphy := adslcore$(BRCM_CHIP)Test
EXTRA_CFLAGS += -DADSL_SELF_TEST
endif

ifneq ($(strip $(BUILD_TR69_XBRCM)),)
EXTRA_CFLAGS += -DADSL_MIBOBJ_PLN
endif

#EXTRA_CFLAGS += -DUSE_LOCAL_DS_POWER
EXTRA_CFLAGS += -DUSE_CXSY_OVH_MSG_WORKAROUND
EXTRA_CFLAGS += -DUSE_CXSY_OVH_MSG_COUNTER_WORKAROUND
EXTRA_CFLAGS += -DUSE_CXSY_OVH_MSG_DISABLE_POLLING_CMD

ifneq ($(strip $(BUILD_SNMP_EOC)),)
EXTRA_CFLAGS += -DBUILD_SNMP_EOC
endif
ifneq ($(strip $(BUILD_SNMP_AUTO)),)
EXTRA_CFLAGS += -DBUILD_SNMP_AUTO
endif
ifeq "$(BUILD_SNMP_TRANSPORT_DEBUG)" "y"
EXTRA_CFLAGS += -DBUILD_SNMP_TRANSPORT_DEBUG
endif

EXTRA_CFLAGS += -DLINUX_FW_VERSION=$(BRCM_VERSION)$(BRCM_RELEASE) -DXDSL_DRV

EXTRA_CFLAGS += -DLINUX_FW_EXTRAVERSION=$(BRCM_VERSION)$(BRCM_RELEASE)$(shell echo $(BRCM_EXTRAVERSION) | head -c 2)

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)

EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc

EXTRA_CFLAGS +=-fms-extensions

ifeq "$(BRCM_PHY_LOOPBACK)" "y"
adslphy := adslcore$(BRCM_CHIP)LB
EXTRA_CFLAGS += -DPHY_LOOPBACK
endif

ifeq "$(BRCM_PHY_CO)" "y"
adslphy := adslcore$(BRCM_CHIP)CO
EXTRA_CFLAGS += -DPHY_CO
endif

ifeq "$(BRCM_CHIP)" "63158"
ifeq "$(BRCM_FULL_CHIP_NAME)" "63158A0"
adslphy := adslcore$(BRCM_CHIP)_A0
endif
endif

EXTRA_CFLAGS += ${BRCM_DRIVER_ATM_BONDING_DEFINES}

ifeq "$(BRCM_PHY_TEQ_FAST_DATA_MODE)" "y"
EXTRA_CFLAGS += -DSUPPORT_TEQ_FAKE_LINKUP
endif

ifeq "$(ADSL_PHY_MODE)" "file"

HOST_EXTRACFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)

ifeq "$(ADSL_SELF_TEST)" "y"
PHYBLD_EXE := $(obj)/$(adslphy)/AdslTestPhyBld
PHYBLD_C := $(obj)/AdslTestPhyBld.c
PHY_BIN  := $(obj)/adsl_test_phy.bin
else
PHYBLD_EXE := $(obj)/$(adslphy)/AdslPhyBld
PHYBLD_C := $(obj)/AdslPhyBld.c
PHY_BIN  := $(obj)/adsl_phy.bin

PHYBLD0_EXE := $(obj)/$(adslphy0)/AdslPhyBld
PHY_BIN0  := $(obj)/adsl_phy0.bin

PHYBLD1_EXE := $(obj)/$(adslphy1)/AdslPhyBld
PHY_BIN1  := $(obj)/adsl_phy1.bin
endif

clean-files := adsl_phy.bin adsl_test_phy.bin

clean-files += $(adslphy)/AdslPhyBld
ifeq "$(BRCM_PHY_BONDING_C0)" "y"
clean-files += adsl_phy0.bin $(adslphy0)/AdslPhyBld
endif
ifneq "$(findstring 63268,$(BRCM_CHIP))$(findstring 6368,$(BRCM_CHIP))" ""
ifeq "$(BRCM_MULTI_PHY)" "y"
ifeq "$(BRCM_PHY_BONDING_C0)" "y"
clean-files += adsl_phy1.bin $(adslphy1)/AdslPhyBld
else
clean-files += adsl_phy0.bin $(adslphy0)/AdslPhyBld
endif
endif
endif
ifeq "$(BRCM_ANNEXAB_COMBO)" "y"
clean-files += adsl_phy0.bin $(adslphy0)/AdslPhyBld
endif
ifeq "$(BRCM_PHY_GFAST)" "y"
clean-files += adsl_phy0.bin $(adslphy0)/AdslPhyBld
endif

$(PHY_BIN): $(PHYBLD_EXE) 
	$(PHYBLD_EXE) $(PHY_BIN)
	if test ! -f $(PHY_BIN) ; then exit 1 ; fi 

$(PHY_BIN0): $(PHYBLD0_EXE) 
	$(PHYBLD0_EXE) $(PHY_BIN0)
	if test ! -f $(PHY_BIN0) ; then exit 1 ; fi 

$(PHY_BIN1): $(PHYBLD1_EXE) 
	$(PHYBLD1_EXE) $(PHY_BIN1)
	if test ! -f $(PHY_BIN1) ; then exit 1 ; fi 

$(PHYBLD_EXE): $(obj)/$(adslphy)/*.[ch] $(PHYBLD_C)
ifeq "$(ADSL_SELF_TEST)" "y"
	cd $(obj)/$(adslphy); \
	$(HOSTCC) $(HOST_EXTRACFLAGS) -o $@ $(obj)/AdslTestPhyBld.c $(obj)/$(adslphy)/adsl_selftest_lmem.c $(obj)/$(adslphy)/adsl_selftest_sdram.c; \
	cd $(obj)
else
	cd $(obj)/$(adslphy); \
	$(HOSTCC) $(HOST_EXTRACFLAGS) -o $@ $(obj)/AdslPhyBld.c $(obj)/$(adslphy)/adsl_lmem.c $(obj)/$(adslphy)/adsl_sdram.c; \
	cd $(obj)
endif

$(PHYBLD0_EXE): $(obj)/$(adslphy0)/*.[ch] $(PHYBLD_C)
	cd $(obj)/$(adslphy0); \
	$(HOSTCC) $(HOST_EXTRACFLAGS) -o $@ $(obj)/AdslPhyBld.c $(obj)/$(adslphy0)/adsl_lmem.c $(obj)/$(adslphy0)/adsl_sdram.c; \
	cd $(obj)

$(PHYBLD1_EXE): $(obj)/$(adslphy1)/*.[ch] $(PHYBLD_C)
	cd $(obj)/$(adslphy1); \
	$(HOSTCC) $(HOST_EXTRACFLAGS) -o $@ $(obj)/AdslPhyBld.c $(obj)/$(adslphy1)/adsl_lmem.c $(obj)/$(adslphy1)/adsl_sdram.c; \
	cd $(obj)

PHY_BIN_FN = $(PHY_BIN)
ifeq "$(BRCM_PHY_BONDING_C0)" "y"
PHY_BIN_FN += $(PHY_BIN0)
endif
ifneq "$(findstring 63268,$(BRCM_CHIP))$(findstring 6368,$(BRCM_CHIP))" ""
ifeq "$(BRCM_MULTI_PHY)" "y"
ifeq "$(BRCM_PHY_BONDING_C0)" "y"
PHY_BIN_FN += $(PHY_BIN1)
else
PHY_BIN_FN += $(PHY_BIN0)
endif
endif
endif
ifeq "$(BRCM_ANNEXAB_COMBO)" "y"
PHY_BIN_FN += $(PHY_BIN0)
endif
ifeq "$(BRCM_PHY_GFAST)" "y"
ifneq "$(BRCM_PHY_GFASTCOMBO)" "y"
PHY_BIN_FN += $(PHY_BIN0)
endif
endif

$(obj)/adsldrv.o: $(PHY_BIN_FN)
targets += $(obj)/adsldrv.o

else

ifeq "$(ADSL_SELF_TEST)" "y"
adsldd-objs += $(adslphy)/adsl_selftest_lmem.o $(adslphy)/adsl_selftest_sdram.o
else
adsldd-objs += $(adslphy)/adsl_lmem.o $(adslphy)/adsl_sdram.o
endif

endif

$(info clean-files: $(clean-files))


-include $(TOPDIR)/Rules.make

clean:
	rm -rf $(clean-files)
