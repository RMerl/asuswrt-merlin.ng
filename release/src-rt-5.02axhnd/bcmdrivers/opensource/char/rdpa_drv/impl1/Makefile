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
# Makefile for the RDPA CMD
#
ifneq ($(PREBUILT_EXTRAMOD),1)
rdpa_cmd-objs := rdpa_cmd_drv.o rdpa_cmd_tm.o rdpa_cmd_misc.o
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_4908_),)
rdpa_cmd-objs += rdpa_cmd_ds_wan_udp_filter.o rdpa_cmd_cpu.o rdpa_cmd_ic.o rdpa_cmd_filter.o
else ifeq ("$(CONFIG_BCM963158)" ,"y")
rdpa_cmd-objs += rdpa_cmd_ds_wan_udp_filter.o rdpa_cmd_sys.o
else
rdpa_cmd-objs += rdpa_cmd_iptv.o rdpa_cmd_ic.o rdpa_cmd_sys.o rdpa_cmd_port.o rdpa_cmd_br.o rdpa_cmd_llid.o rdpa_cmd_filter.o rdpa_cmd_dscp_to_pbit.o rdpa_cmd_pbit_to_q.o rdpa_cmd_cpu.o 
endif
else
rdpa_cmd-objs := rdpa_cmd$(PRBM_EXT).o
endif
obj-$(CONFIG_BCM_RDPA_DRV) += rdpa_cmd.o

SRCS := $(subst rdpa_drv.mod.c,,$(notdir $(shell ls $(src)/*.c)))
OBJS := $(SRCS:.c=.o)
rdpa_drv-objs += $(OBJS)

ifneq ($(strip $(BUILD_BRCM_FTTDP)),)
EXTRA_CFLAGS += -DBRCM_FTTDP
endif


EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/time_sync/impl$(CONFIG_BCM_TIME_SYNC_IMPL)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/rdpa_drv/impl$(CONFIG_BCM_RDPA_DRV_IMPL) -I$(INC_RDPA_MW_PATH)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/rdpa_gpl/impl1/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/rdpa_gpl/impl1/include/autogen
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/bcm963xx
EXTRA_CFLAGS += $(EXTRA_INCLUDES) $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -Werror -Wall 

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko *.mod.c modules.*

