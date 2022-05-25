#/*
#<:copyright-BRCM:2013:DUAL/GPL:standard 
#
#   Copyright (c) 2013 Broadcom 
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
obj-$(CONFIG_BCM963158) += pmc_xrdp.o pmc_sata.o pmc_pcm.o pmc_dsl.o pmc_switch.o pmc_wan.o pmc_spu.o pmc_dgasp.o pmc_cpu.o pmc_sdhci.o pmc_ssb_access.o
obj-$(CONFIG_BCM96846) += pmc_xrdp.o pmc_pcm.o pmc_wan.o
obj-$(CONFIG_BCM96856) += pmc_xrdp.o pmc_pcm.o pmc_wan.o pmc_rgmii.o pmc_sdhci.o pmc_sgmii.o
obj-$(CONFIG_BCM963178) += pmc_switch.o pmc_dsl.o pmc_dgasp.o pmc_pcm.o pmc_wlan.o
obj-$(CONFIG_BCM947622) += pmc_sysport.o pmc_wlan.o pmc_pcm.o pmc_spu.o pmc_sdhci.o
obj-$(CONFIG_BCM96878) += pmc_xrdp.o pmc_pcm.o pmc_wlan.o pmc_wan.o pmc_avs.o
obj-$(CONFIG_BCM96855) += pmc_avs.o  pmc_sgmii.o pmc_xrdp.o pmc_avs.o pmc_wlan.o pmc_sdhci.o pmc_pcm.o
obj-$(CONFIG_BCM96756) += pmc_switch.o pmc_wlan.o pmc_spu.o pmc_sdhci.o pmc_pcm.o
obj-$(CONFIG_BCM963146) += pmc_dsl.o pmc_pcm.o pmc_xrdp.o pmc_dgasp.o pmc_rgmii.o pmc_sdhci.o pmc_mpm.o
obj-$(CONFIG_BCM94912) += pmc_xrdp.o pmc_spu.o pmc_pcm.o pmc_rgmii.o pmc_sdhci.o pmc_xport.o pmc_mpm.o
obj-$(CONFIG_BCM96813) += pmc_xrdp.o pmc_spu.o pmc_pcm.o pmc_sdhci.o pmc_xport.o pmc_mpm.o
endif

clean:
	rm -f core *.o *.a *.s
