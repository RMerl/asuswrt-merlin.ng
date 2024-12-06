#/*
#<:copyright-BRCM:2022:DUAL/GPL:standard 
#
#   Copyright (c) 2022 Broadcom 
#   All Rights Reserved
#
#Unless you and Broadcom execute a separate written software license
#agreement governing use of this software, this software is licensed
#to you under the terms of the GNU General Public License version 2
#(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
#with the following added to such license:
#
#   As a special exception, the copyright holders of this software give
#   you permission to link this software with independent modules, and
#   to copy and distribute the resulting executable under terms of your
#   choice, provided that you also meet, for each linked independent
#   module, the terms and conditions of the license of that module.
#   An independent module is a module which is not derived from this
#   software.  The special exception does not apply to any modifications
#   of the software.
#
#Not withstanding the above, under no circumstances may you combine
#this software in any way with any other Broadcom software provided
#under a license other than the GPL, without Broadcom's express prior
#written consent.
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
