#
# Makefile for the SYNCE_HOLDOVER driver
#

obj-$(CONFIG_BCM_SYNCE_HOLDOVER) += synce_holdover.o
synce_holdover-objs += bcm_synce_holdover.o

EXTRA_CFLAGS += $(INC_RDP_FLAGS) -Werror -Wall -I$(src) 
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/phy
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/pon_drv/impl$(CONFIG_BCM_PON_DRV_IMPL)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko *.mod.c modules.*
