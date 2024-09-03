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
# Makefile for the Virtual Forwarded driver
#

bcmvfrwd-objs += vfrwd.o

obj-$(CONFIG_BCM_VFRWD) += bcmvfrwd.o

EXTRA_CFLAGS += -I$(obj)

EXTRA_CFLAGS += -Werror -Wall

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko

