#
# Makefile for the PCIe host controller drivers
#  for ARM/ARM64 based platforms with 4.1 kernel onwards
#

PCIE_HCD_PATH=$(BRCMDRIVERS_DIR)/opensource/bus/pci/host/impl1
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -I$(PCIE_HCD_PATH)


obj-pcie-hcd-$(CONFIG_BCM_PCIE_HCD)        += pcie_hcd.o
obj-pcie-hcd-$(CONFIG_BCM_963XX_PCIE_HC)   += pcie-bcm963xx.o
obj-pcie-hcd-$(CONFIG_BCM_947XX_PCIE_HC)   += pcie-bcm947xx.o
obj-pcie-hcd-$(CONFIG_BCM_PCIE_VCD)        += pcie-vcore.o

MODULE_OBJS += $(obj-pcie-hcd-y) $(obj-pcie-hcd-m)

ifeq ("$(CONFIG_BCM_PCIE_HCD)" ,"y")
obj-$(CONFIG_BCM_PCIE_HCD) +=  $(MODULE_OBJS)
else
MODULE_NAME=bcm_pcie_hcd
$(MODULE_NAME)-objs += $(MODULE_OBJS)
obj-$(CONFIG_BCM_PCIE_HCD) += $(MODULE_NAME).o
endif

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.o .*.cmd *.ko $(MODULE_NAME)* modules.*
