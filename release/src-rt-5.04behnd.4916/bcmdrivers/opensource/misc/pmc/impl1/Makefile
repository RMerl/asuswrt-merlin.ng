#/*
#<:copyright-BRCM:2013:DUAL/GPL:standard 
#
#   Copyright (c) 2013 Broadcom 
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
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/include/bcm963xx

ifeq ($(strip $(BUILD_DSL)),)
EXTRA_CFLAGS += -DNO_DSL
endif

obj-y	 := pmc_drv.o pmc_drv_linux.o pmc_drv_dt.o
ifeq ($(strip $(BRCM_IKOS)),)
obj-y += clk_rst.o pmc_sysfs.o pmc_pcie.o pmc_usb.o

obj-$(CONFIG_BCM963138) += pmc_switch.o pmc_sar.o pmc_dsl.o pmc_sata.o pmc_apm.o pmc_rdp.o pmc_neon.o pmc_dect.o pmc_acp.o pmc_sdhci.o pmc_ssb_access.o
obj-$(CONFIG_BCM963148) += pmc_switch.o pmc_sar.o pmc_dsl.o pmc_sata.o pmc_apm.o pmc_rdp.o pmc_dect.o pmc_ssb_access.o
obj-$(CONFIG_BCM94908) += pmc_switch.o pmc_fpm.o pmc_rdp.o pmc_sata.o pmc_pcm.o pmc_spu.o pmc_sdhci.o pmc_ssb_access.o
obj-$(CONFIG_BCM96858) += pmc_xrdp.o pmc_lport.o pmc_wan.o pmc_pcm.o pmc_sata.o pmc_spu.o pmc_cpu.o pmc_rgmii.o pmc_sdhci.o 
obj-$(CONFIG_BCM963158) += pmc_xrdp.o pmc_sata.o pmc_dsl.o pmc_switch.o pmc_wan.o pmc_spu.o pmc_dgasp.o pmc_cpu.o pmc_sdhci.o pmc_ssb_access.o
obj-$(CONFIG_BCM96846) += pmc_xrdp.o pmc_wan.o
obj-$(CONFIG_BCM96856) += pmc_xrdp.o pmc_wan.o pmc_rgmii.o pmc_sdhci.o pmc_sgmii.o
obj-$(CONFIG_BCM963178) += pmc_switch.o pmc_dsl.o pmc_dgasp.o pmc_wlan.o
obj-$(CONFIG_BCM947622) += pmc_sysport.o pmc_wlan.o pmc_spu.o pmc_sdhci.o
obj-$(CONFIG_BCM96878) += pmc_xrdp.o pmc_wlan.o pmc_wan.o pmc_avs.o
obj-$(CONFIG_BCM96855) += pmc_avs.o pmc_sgmii.o pmc_xrdp.o pmc_wlan.o pmc_sdhci.o pmc_wan.o
obj-$(CONFIG_BCM96756) += pmc_switch.o pmc_wlan.o pmc_spu.o pmc_sdhci.o
obj-$(CONFIG_BCM96765) += pmc_sdhci.o pmc_spu.o pmc_switch.o pmc_wlan.o
obj-$(CONFIG_BCM96766) += pmc_sdhci.o pmc_spu.o pmc_switch.o pmc_wlan.o
obj-$(CONFIG_BCM96764) += pmc_sdhci.o pmc_spu.o pmc_switch.o pmc_wlan.o
obj-$(CONFIG_BCM963146) += pmc_dsl.o pmc_xrdp.o pmc_dgasp.o pmc_rgmii.o pmc_sdhci.o pmc_mpm.o
obj-$(CONFIG_BCM94912) += pmc_xrdp.o pmc_spu.o pmc_rgmii.o pmc_sdhci.o pmc_xport.o pmc_mpm.o
obj-$(CONFIG_BCM96813) += pmc_xrdp.o pmc_spu.o pmc_sdhci.o pmc_xport.o pmc_mpm.o pmc_wan.o
obj-$(CONFIG_BCM96888) += pmc_spu.o pmc_xport.o pmc_wan.o pmc_sdhci.o
obj-$(CONFIG_BCM968880) += pmc_spu.o pmc_xport.o pmc_wan.o pmc_sdhci.o
obj-$(CONFIG_BCM96837) += pmc_xport.o pmc_wan.o pmc_sdhci.o
else
obj-$(CONFIG_BCM96888) += pmc_spu.o
obj-$(CONFIG_BCM96765) += pmc_spu.o
obj-$(CONFIG_BCM96766) += pmc_spu.o
endif

clean:
	rm -f core *.o *.a *.s
