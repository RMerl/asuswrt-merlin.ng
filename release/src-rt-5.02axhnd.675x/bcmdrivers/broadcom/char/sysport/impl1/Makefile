#
# Makefile for sysport enet test driver
#
# Note this driver is referencing header files from ENET impl7 implementation

NAME = bcm_enet_sysport

obj-$(CONFIG_BCM_ENET_SYSPORT) = $(NAME).o

SYSPORT_DIR = $(BRCMDRIVERS_DIR)/broadcom/char/sysport/bcm9$(BRCM_CHIP)

ifneq ($(CONFIG_BCM_ARCHER),)
$(shell rm -f $(SYSPORT_DIR)/sysport_driver_linux.[ch])
$(shell ln -s -f $(BRCMDRIVERS_DIR)/broadcom/char/archer/bcm9$(BRCM_CHIP)/sysport_driver_linux.[ch] $(SYSPORT_DIR))
$(NAME)-objs += sysport_driver_linux.o
else
$(NAME)-objs += enet_sysport_drv.o
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_),)
$(shell rm -f $(SYSPORT_DIR)/bcmgmac_4908.c)
$(shell rm -f $(SYSPORT_DIR)/bcmgmac_4908.h)
$(shell ln -s -f $(BRCMDRIVERS_DIR)/opensource/net/enet/impl5/bcmgmac_4908.c $(SYSPORT_DIR)/bcmgmac_4908.c)
$(shell ln -s -f $(BRCMDRIVERS_DIR)/opensource/include/bcm963xx/bcmgmac_4908.h $(SYSPORT_DIR)/bcmgmac_4908.h)
$(NAME)-objs += bcmgmac_4908.o
endif
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/include/bcm963xx
EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource/include/bcm963xx
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/customerApi/impl1
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx

-include $(TOPDIR)/Rules.make

EXTRA_CFLAGS += -Werror -Wall

clean:
	rm -f core *.o *.a *.s *.ko .*.cmd .*.o.d Module.symvers modules.builtin modules.order *.mod.c
	rm -rf .tmp_versions

.PHONY: clean
