#
# Makefile for the Generic PCIe End Point driver for Broadcom BCA platforms
#

PCIE_GEPD_PATH=$(BRCMDRIVERS_DIR)/opensource/bus/pci/endpoint/gepd/impl1

MODULE_OBJS += pcie-gepd.o


ifeq ("$(CONFIG_BCM_PCIE_GEPD)" ,"y")
obj-$(CONFIG_BCM_PCIE_GEPD) +=  $(MODULE_OBJS)
else
MODULE_NAME=pcie_gepd
$(MODULE_NAME)-objs += $(MODULE_OBJS)
obj-$(CONFIG_BCM_PCIE_GEPD) += $(MODULE_NAME).o
endif

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.o .*.cmd *.ko $(MODULE_NAME)* modules.*
