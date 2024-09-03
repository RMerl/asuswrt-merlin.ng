#/*
#<:copyright-BRCM:2022:DUAL/GPL:standard 
#
#   Copyright (c) 2022 Broadcom 
#   All Rights Reserved
#
#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License, version 2, as published by
#the Free Software Foundation (the "GPL").
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#
#A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
#writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#Boston, MA 02111-1307, USA.
#
#:> 
#*/



# remove optimization for debug 
#EXTRA_CFLAGS += -O0 -g
EXTRA_CFLAGS += -Werror
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/misc/pmc/impl$(CONFIG_BCM_PMC_IMPL)/$(BRCM_CHIP)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/include/bcm963xx

obj-y	 := pmc_drv.o pmc_drv_linux.o
ifeq ($(strip $(BRCM_IKOS)),)
obj-y += clk_rst.o pmc_sysfs.o pmc_pcie.o pmc_usb.o pmc_rpc.o avs_rpc_svc.o

obj-$(CONFIG_BCM968880) += pmc_xrdp.o pmc_xport.o pmc_wan.o pmc_pcm.o  pmc_spu.o pmc_cpu.o  pmc_sdhci.o  pmc_spu.o power_rpc_svc.o clk_rpc_svc.o ba_rpc_svc.o pmc_ethtop.o pmc_rccore.o pmc_shutdown.o
obj-$(CONFIG_BCM96837)  += pmc_xrdp.o pmc_xport.o pmc_wan.o pmc_pcm.o  pmc_spu.o pmc_cpu.o  pmc_sdhci.o  pmc_spu.o power_rpc_svc.o clk_rpc_svc.o ba_rpc_svc.o pmc_ethtop.o pmc_rccore.o pmc_shutdown.o
ifneq ($(strip $(CONFIG_BCM_PMC_AUX)),)
obj-$(CONFIG_BCM968880) +=  pmc_aux.o
endif
endif
clean:
	rm -f core *.o *.a *.s
