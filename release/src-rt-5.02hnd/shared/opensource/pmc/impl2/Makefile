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
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc -I$(INC_BRCMSHARED_PUB_PATH)/rdp
obj-y	 := pmc_drv.o       \
			pmc_pcie.o		\
			pmc_sysfs.o		\
            clk_rst.o       


-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s
