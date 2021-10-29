#
# <:copyright-BRCM:2013:DUAL/GPL:standard
# 
#    Copyright (c) 2013 Broadcom 
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
# Makefile for the RDPA MW
#
obj-$(CONFIG_BCM_RDPA_MW) := rdpa_mw.o

SRCS := $(subst rdpa_mw.mod.c,,$(notdir $(shell ls $(src)/*.c)))
OBJS := $(SRCS:.c=.o)
rdpa_mw-objs += $(OBJS)

EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/time_sync/impl$(CONFIG_BCM_TIME_SYNC_IMPL)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/rdpa_gpl/impl1/include
EXTRA_CFLAGS += $(EXTRA_INCLUDES) $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/net/bridge
EXTRA_CFLAGS += -Werror -Wall

ifneq ($(strip $(BUILD_BRCM_CMS)),)
	EXTRA_CFLAGS += -DBRCM_CMS_BUILD
endif

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko *.mod.c modules.*

