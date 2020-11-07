#
# Makefile for the bcm963xx PCIe host controller drivers
#  for ARM and MIPS based platforms with 4.1 kernel onwards
#

PCIE_HCD_PATH=$(BRCMDRIVERS_DIR)/opensource/bus/pci/host/impl2
EXTRA_CFLAGS += -I$(PCIE_HCD_PATH)/common
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)

#
# ARM platforms
#
ifeq ($(filter y,$(CONFIG_ARM) $(CONFIG_ARM64)),)
else
EXTRA_CFLAGS += -I$(PCIE_HCD_PATH)/arm
obj-$(CONFIG_BCM_PCI) += arm/pcie-bcm947xx.o
ifeq ("$(CONFIG_ARM64)" ,"y")
#temporary until fill DT is implemented for arm64 platforms
obj-$(CONFIG_BCM_PCI) += ../../../../../../kernel/$(notdir $(KERNEL_DIR))/drivers/pci/setup-irq.o
endif
endif

obj-$(CONFIG_BCM_PCI) += common/pcie_common.o

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

