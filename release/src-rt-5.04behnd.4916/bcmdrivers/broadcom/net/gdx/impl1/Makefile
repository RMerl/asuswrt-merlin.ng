#
# <:copyright-BRCM:2012:DUAL/GPL:standard
# 
#    Copyright (c) 2012 Broadcom 
#    All Rights Reserved
# 
# Unless you and Broadcom execute a separate written software license
# agreement governing use of this software, this software is licensed
# to you under the terms of the GNU General Public License version 2
# (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
# with the following added to such license:
# 
#    As a special exception, the copyright holders of this software give
#    you permission to link this software with independent modules, and
#    to copy and distribute the resulting executable under terms of your
#    choice, provided that you also meet, for each linked independent
#    module, the terms and conditions of the license of that module.
#    An independent module is a module which is not derived from this
#    software.  The special exception does not apply to any modifications
#    of the software.
# 
# Not withstanding the above, under no circumstances may you combine
# this software in any way with any other Broadcom software provided
# under a license other than the GPL, without Broadcom's express prior
# written consent.
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

