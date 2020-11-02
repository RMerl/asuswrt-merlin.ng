#
# Makefile for the bcm963xx PCIe host controller drivers
#  for ARM and MIPS based platforms with 4.1 kernel onwards
#

PCIE_HCD_PATH=$(BRCMDRIVERS_DIR)/opensource/bus/pci/host/impl1
EXTRA_CFLAGS += -I$(PCIE_HCD_PATH)/common
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc

#
# ARM platforms
#
ifeq ($(filter y,$(CONFIG_ARM) $(CONFIG_ARM64)),)
else
EXTRA_CFLAGS += -I$(PCIE_HCD_PATH)/arm
MODULE_OBJS += arm/pcie-bcm963xx.o
MODULE_OBJS += arm/pcie_hcd.o
ifeq ("$(CONFIG_BCM_PCIE_VCD)","y")
MODULE_OBJS += arm/pcie-vcore.o
endif
endif

#
# MIPS platforms
#
ifeq ("$(CONFIG_MIPS)" ,"y")
ifdef BCM_KF # defined(CONFIG_BCM_KF_PCI_FIXUP)
ifdef CONFIG_MIPS_BCM963XX
EXTRA_CFLAGS += -I$(PCIE_HCD_PATH)/mips
MODULE_OBJS += mips/pci-bcm963xx.o mips/fixup-bcm963xx.o mips/ops-bcm963xx.o
endif
endif # BCM_KF
endif

MODULE_OBJS += common/pcie_common.o

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
	rm -f arm/*.o arm/.*.cmd
	rm -f common/*.o common/.*.cmd
	rm -f mips/*.o mips/.*.cmd
