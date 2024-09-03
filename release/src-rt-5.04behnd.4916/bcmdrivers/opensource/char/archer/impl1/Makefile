#
# <:copyright-BRCM:2021:DUAL/GPL:standard
# 
#    Copyright (c) 2021 Broadcom 
#    All Rights Reserved
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2, as published by
# the Free Software Foundation (the "GPL").
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# 
# A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
# writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
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
