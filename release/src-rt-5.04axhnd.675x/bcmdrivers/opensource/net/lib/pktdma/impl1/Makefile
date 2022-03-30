# File: bcmdrivers/opensource/char/pktdma
#
# Makefile for the BCM63xx pktdma driver

ifneq "$(BRCM_CHIP)" "63138"
obj-$(CONFIG_BCM_PKTDMA) += bcmPktDmaEthIudma.o

obj-$(CONFIG_BCM_PKTDMA) += bcmPktDmaBds.o

ifneq ($(strip $(BRCM_DRIVER_XTM)),)
#ifneq ($(strip $(BCM_XTMCFG))$(strip $(BCM_XTMCFG_MODULE)),)
obj-$(CONFIG_BCM_PKTDMA) += bcmPktDmaXtmIudma.o
#endif
endif

endif
obj-$(CONFIG_BCM_PKTDMA) += bcmPktDmaBds.o

ifneq ($(strip $(BRCM_DRIVER_XTM)),)
obj-$(CONFIG_BCM_PKTDMA) += bcmPktDmaXtmIudma.o
endif

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
#  EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
ifeq ($(CONFIG_BCM_XTMRT_IMPL),1)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/xtmrt/impl3
else
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/xtmrt/impl$(CONFIG_BCM_XTMRT_IMPL)
endif
#  EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

EXTRA_CFLAGS += -Werror -Wall -Wfatal-errors


-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

