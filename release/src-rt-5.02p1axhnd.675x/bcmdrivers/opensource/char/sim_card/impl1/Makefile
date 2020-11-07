# File: bcmdrivers/char/sim_card/bcm963xx/Makefile
#
# Makefile for the BCM68xx sim card driver.
#
sim_card-objs = chal_simio.o simio_drv.o simio_atr.o simio_board.o simio_api.o 

ifeq ($(SIM_CARD_PROC_SUPPORT),y)
sim_card-objs += simio_user.o
else
sim_card-objs += simio_user_kernel.o
endif

obj-$(CONFIG_BCM_SIM_CARD) = sim_card.o 

EXTRA_CFLAGS += -I$(obj)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

EXTRA_CFLAGS += -Werror

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko


