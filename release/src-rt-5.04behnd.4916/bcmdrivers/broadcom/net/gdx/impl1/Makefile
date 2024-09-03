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
#
# Makefile for the GDX
#
MODULE_NAME=gdx

$(MODULE_NAME)-objs += gdx_main.o gdx_private.o

ifneq ($(strip $(CONFIG_BCM_GDX_HW)),)
$(MODULE_NAME)-objs += gdx_hw.o
endif

obj-$(CONFIG_BCM_GDX) += gdx.o

EXTRA_CFLAGS += $(INC_RDP_FLAGS) $(EXTRA_INCLUDES) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -Werror -Wall -g

-include $(TOPDIR)/Rules.make

$(obj)/gdx_hw.o:
	cp $(obj)/prebuilt/gdx_hw.o $(obj)/gdx_hw.o

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko

# Makefile trick: breaking the following into two rules allows
# the "%.o : %.c" rule (defined earlier by kbuild), to take precedence

%.o : %.o_tmp
	$(Q)mv $< $@

%.o_tmp : %.$(BRCM_CHIP).o_saved

	@echo "Using $<"
	$(Q)cp $< $@

