#
# <:copyright-BRCM:2012:DUAL/GPL:standard
# 
#    Copyright (c) 2012 Broadcom 
#    All Rights Reserved
# 
# Unless you and Broadcom execute a separate written software license
# agreement governing use of this software, this software is licensed
# to you under the terms of the GNU General Public License version 2
# (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
# with the following added to such license:
# 
#    As a special exception, the copyright holders of this software give
#    you permission to link this software with independent modules, and
#    to copy and distribute the resulting executable under terms of your
#    choice, provided that you also meet, for each linked independent
#    module, the terms and conditions of the license of that module.
#    An independent module is a module which is not derived from this
#    software.  The special exception does not apply to any modifications
#    of the software.
# 
# Not withstanding the above, under no circumstances may you combine
# this software in any way with any other Broadcom software provided
# under a license other than the GPL, without Broadcom's express prior
# written consent.
# 
# :> 
#
# Makefile for the Bridge MW
#

bcmbrfp-objs += br_fp.o 

obj-$(CONFIG_BCM_RDPA_BRIDGE) += bcmbrfp.o

EXTRA_CFLAGS += -I$(obj)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH)
EXTRA_CFLAGS += $(INC_RDP_FLAGS)

EXTRA_CFLAGS += -I$(KERNEL_DIR)/net/bridge

EXTRA_CFLAGS += -Werror -Wall 

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko

