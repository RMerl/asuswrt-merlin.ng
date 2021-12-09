#
# <:copyright-BRCM:2013:DUAL/GPL:standard
# 
#    Copyright (c) 2013 Broadcom 
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
# Makefile for the RDPA CMD
#
rdpa_cmd-objs := rdpa_cmd_sys.o rdpa_cmd_drv.o rdpa_cmd_tm.o rdpa_cmd_misc.o rdpa_cmd_ic.o rdpa_cmd_filter.o rdpa_cmd_cpu.o
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_4908_),)
rdpa_cmd-objs += rdpa_cmd_ds_wan_udp_filter.o
else ifeq ("$(CONFIG_BCM963158)" ,"y")
rdpa_cmd-objs += rdpa_cmd_llid.o 
else
rdpa_cmd-objs += rdpa_cmd_iptv.o rdpa_cmd_port.o rdpa_cmd_br.o rdpa_cmd_llid.o rdpa_cmd_dscp_to_pbit.o rdpa_cmd_pbit_to_q.o
endif

obj-$(CONFIG_BCM_RDPA_DRV) += rdpa_cmd.o

SRCS := $(subst rdpa_drv.mod.c,,$(notdir $(shell ls $(src)/*.c)))
OBJS := $(SRCS:.c=.o)
rdpa_drv-objs += $(OBJS)

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

