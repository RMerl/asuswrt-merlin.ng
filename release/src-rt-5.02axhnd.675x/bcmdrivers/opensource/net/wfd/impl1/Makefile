#
# <:copyright-BRCM:2012:DUAL/GPL:standard
# 
#    Copyright (c) 2012 Broadcom 
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

# File: bcmdrivers/opensource/net/wifi_mw/impl1/Makefile
#
# Makefile for the wifi MW
#
MODULE_NAME=wfd

$(MODULE_NAME)-objs += wfd_dev.o
ifneq ($(strip $(CONFIG_BCM_WFD_RATE_LIMITER)),)
$(MODULE_NAME)-objs += rate_limiter.o
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
ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
	EXTRA_CFLAGS += -DBCM_PKTFWD
    # Enable credit based Host Flow Control
    ifneq ($(strip $(CONFIG_BCM_PKTFWD_FLCTL)),)
        EXTRA_CFLAGS += -DBCM_PKTFWD_FLCTL
    endif
endif
EXTRA_INCLUDES  += -I$(INC_BRCMSHARED_PUB_PATH)/bcm963xx -I$(INC_BRCMDRIVER_PUB_PATH)/bcm963xx -I$(BRCMDRIVERS_DIR)/opensource/net/enet/share
endif

EXTRA_CFLAGS += $(INC_RDP_FLAGS) $(EXTRA_INCLUDES) -Werror -Wall -g

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko


