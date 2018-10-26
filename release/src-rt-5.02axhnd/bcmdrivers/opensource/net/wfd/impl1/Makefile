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

# File: bcmdrivers/opensource/net/wifi_mw/impl1/Makefile
#
# Makefile for the wifi MW
#
MODULE_NAME=wfd

ifneq ($(PREBUILT_EXTRAMOD),1)
$(MODULE_NAME)-objs += wfd_dev.o
ifneq ($(strip $(CONFIG_BCM_WFD_RATE_LIMITER)),)
$(MODULE_NAME)-objs += rate_limiter.o
endif
else
$(MODULE_NAME)-objs := $(MODULE_NAME)$(PRBM_EXT).o
endif

obj-$(CONFIG_BCM_WIFI_FORWARDING_DRV) += $(MODULE_NAME).o

EXTRA_CFLAGS += -I$(obj)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/fap/impl$(CONFIG_BCM_FAP_IMPL)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/fap/impl$(CONFIG_BCM_FAP_IMPL)/4ke
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/xtmrt/impl$(CONFIG_BCM_XTMRT_IMPL) 
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH) 

ifneq ($(strip $(CONFIG_BCM_WLAN)),)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/shared/impl1
EXTRA_CFLAGS += -DDSLCPE -DBCMDRIVER -DPKTC
EXTRA_INCLUDES  += -I$(INC_BRCMSHARED_PUB_PATH)/bcm963xx -I$(INC_BRCMDRIVER_PUB_PATH)/bcm963xx -I$(BRCMDRIVERS_DIR)/opensource/net/enet/share
endif

EXTRA_CFLAGS += $(INC_RDP_FLAGS) $(EXTRA_INCLUDES) -Werror -Wall -g

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko


