#
# <:copyright-BRCM:2021:DUAL/GPL:standard
# 
#    Copyright (c) 2021 Broadcom 
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

NAME = bcm_archer_gpl

obj-$(CONFIG_BCM_ARCHER) = $(NAME).o

$(NAME)-objs :=  archer_gpl.o

ifneq ($(strip $(CONFIG_BCM_CROSSBOW_FULL_OFFLOAD)),)
$(NAME)-objs +=  crossbow_gpl.o
endif

EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/archer/impl1
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)

EXTRA_CFLAGS += -Werror -Wall -Wfatal-errors
EXTRA_CFLAGS += -O3

clean:
	rm -f core *.o *.a *.s *.ko .*.cmd .*.o.d Module.symvers modules.builtin modules.order *.mod.c
