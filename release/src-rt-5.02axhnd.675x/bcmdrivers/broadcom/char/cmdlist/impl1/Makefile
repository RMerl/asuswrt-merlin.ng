
# File: bcmdrivers/broadcom/char/cmdlist/impl1/Makefile
#
# Makefile for the Command List driver
#
#***********************************************************************
#
#  Copyright (c) 2006-2017  Broadcom Corporation
#  All Rights Reserved
#
# <:label-BRCM:2017:proprietary:standard
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

cmdlist-objs :=  cmdlist_driver.o cmdlist_ucast.o cmdlist_l2_ucast.o cmdlist_mcast.o

ifneq ($(strip $(CONFIG_BCM_ARCHER)),)
RUNNER_BUILD = n
else
ifneq ($(strip $(CONFIG_BCM_RDPA)),)
RUNNER_BUILD = y
endif
endif

ifeq ("$(RUNNER_BUILD)", "y")
# Runner
cmdlist-objs += gpe_api.o
ifneq ($(strip $(CONFIG_BCM_CMDLIST_SIM)),)
cmdlist-objs += gpe_sim.o
cmdlist-objs += runner_sim.o
endif
EXTRA_CFLAGS += -DCONFIG_CMDLIST_GPE
else
# System Port
cmdlist-objs += spe_api.o
ifneq ($(strip $(CONFIG_BCM_CMDLIST_SIM)),)
cmdlist-objs += spe_sim.o
cmdlist-objs += sysport_sim.o
cmdlist-objs += sysport_parser.o
cmdlist-objs += sysport_rsb.o
cmdlist-objs += sysport_crc.o
cmdlist-objs += sysport_classifier.o
ARCHER_PATH = $(BRCMDRIVERS_DIR)/broadcom/char/archer/impl1
CMDLIST_PATH = $(BRCMDRIVERS_DIR)/broadcom/char/cmdlist/impl1
$(shell rm -f $(CMDLIST_PATH)/sysport_parser_inline.h)
$(shell ln -s -f $(ARCHER_PATH)/sysport_parser_inline.h $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_parser.h)
$(shell ln -s -f $(ARCHER_PATH)/sysport_parser.h $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_rsb.c)
$(shell ln -s -f $(ARCHER_PATH)/sysport_rsb.c $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_rsb.h)
$(shell ln -s -f $(ARCHER_PATH)/sysport_rsb.h $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_crc.c)
$(shell ln -s -f $(ARCHER_PATH)/sysport_crc.c $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_classifier.c)
$(shell ln -s -f $(ARCHER_PATH)/sysport_classifier.c $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_classifier.h)
$(shell ln -s -f $(ARCHER_PATH)/sysport_classifier.h $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_classifier_inline.h)
$(shell ln -s -f $(ARCHER_PATH)/sysport_classifier_inline.h $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_spe.c)
$(shell ln -s -f $(ARCHER_PATH)/sysport_spe.c $(CMDLIST_PATH))
$(shell rm -f $(CMDLIST_PATH)/sysport_spe.h)
$(shell ln -s -f $(ARCHER_PATH)/sysport_spe.h $(CMDLIST_PATH))
endif
EXTRA_CFLAGS += -DCONFIG_CMDLIST_SPE
endif

obj-$(CONFIG_BCM_CMDLIST) += cmdlist.o

EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/archer/impl1/
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH)

EXTRA_CFLAGS += -Werror -Wall -Wfatal-errors 

ifneq ($(strip $(CONFIG_BCM_DHD_RUNNER)),)
EXTRA_CFLAGS += -DCONFIG_DHD_RUNNER
endif

clean:
	rm -f core *.o *.a *.s

# Makefile trick: breaking the following into two rules allows
# the "%.o : %.c" rule (defined earlier by kbuild), to take precidence

%.o : %.o_tmp
	$(Q)mv $< $@

%.o_tmp : %.$(BCMARCH).o_saved
	@echo "Using $<"
	$(Q)cp $< $@
