#
# <:copyright-BRCM:2015:DUAL/GPL:standard
# 
#    Copyright (c) 2015 Broadcom 
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
# File: bcmdrivers/opensource/char/fpm
#
# Makefile for the Runner FPM driver based on BCM3390's FPM driver
#

rdp_fpm-objs := fpm_core.o fpm_proc.o
ifneq ($(CONFIG_BCM_FPM_DT),)
rdp_fpm-objs += fpm_dt.o
endif
ifneq ($(CONFIG_BCM_FPM_TOKEN_HIST),)
rdp_fpm-objs += fpm_hist.o
endif
obj-$(CONFIG_BCM_FPM) += rdp_fpm.o

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc
EXTRA_CFLAGS += -Werror -Wfatal-errors

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko *.mod.c modules.*
