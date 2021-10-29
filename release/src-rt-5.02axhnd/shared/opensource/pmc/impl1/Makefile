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
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/include/bcm963xx
obj-y	 := pmc_drv.o
ifeq ($(strip $(BRCM_IKOS)),)
obj-$(CONFIG_BCM96838) += clk_rst.o pmc_pcie.o pmc_sysfs.o pmc_wan.o
obj-$(CONFIG_BCM963138) += clk_rst.o pmc_switch.o pmc_sar.o pmc_pcie.o pmc_sysfs.o pmc_usb.o pmc_dsl.o pmc_sata.o pmc_apm.o pmc_rdp.o pmc_neon.o
obj-$(CONFIG_BCM963148) += clk_rst.o pmc_switch.o pmc_sar.o pmc_pcie.o pmc_sysfs.o pmc_usb.o pmc_dsl.o pmc_sata.o pmc_apm.o pmc_rdp.o
obj-$(CONFIG_BCM963381) += pmc_switch.o pmc_sar.o pmc_pcie.o pmc_usb.o pmc_dsl.o pmc_sysfs.o pmc_pcm.o
obj-$(CONFIG_BCM96848) += pmc_pcie.o pmc_sysfs.o clk_rst.o pmc_pcm.o pmc_usb.o pmc_wan.o
obj-$(CONFIG_BCM94908) += clk_rst.o pmc_pcie.o pmc_switch.o pmc_sysfs.o pmc_fpm.o pmc_rdp.o pmc_usb.o pmc_sata.o pmc_pcm.o pmc_spu.o
obj-$(CONFIG_BCM96858) += clk_rst.o pmc_sysfs.o pmc_xrdp.o pmc_lport.o pmc_wan.o pmc_pcie.o pmc_pcm.o pmc_usb.o pmc_sata.o pmc_spu.o
obj-$(CONFIG_BCM963158) += clk_rst.o pmc_xrdp.o pmc_pcie.o pmc_usb.o pmc_sata.o pmc_pcm.o pmc_dsl.o pmc_sysfs.o pmc_switch.o pmc_wan.o pmc_spu.o pmc_dgasp.o
obj-$(CONFIG_BCM96846) += clk_rst.o pmc_sysfs.o pmc_usb.o pmc_xrdp.o pmc_pcie.o pmc_pcm.o pmc_wan.o
obj-$(CONFIG_BCM96856) += clk_rst.o pmc_sysfs.o pmc_pcie.o pmc_xrdp.o  pmc_usb.o pmc_pcm.o pmc_wan.o
endif

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s
