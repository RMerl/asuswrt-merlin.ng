# File: bcmdrivers/opensource/char/plat-bcm
#
# Makefile for the BCM63xx/68xx  platform drivers
obj-$(CONFIG_ARM64) += bcm_legacy_io_map.o
obj-$(CONFIG_ARM64) += bcm_arm_irq.o
obj-$(CONFIG_ARM64) += bcm_arm64_setup.o
ifeq ($(PREBUILT_EXTRAMOD),1)
bcm_thermal-objs := bcm_thermal_preb.o
endif
obj-$(CONFIG_BCM_THERMAL) += bcm_thermal.o
obj-$(CONFIG_BCM_ARM_CPUIDLE) += bcm_arm_cpuidle.o
obj-$(CONFIG_BCM_CFE_XARGS) += blxargs.o
obj-$(CONFIG_ARM) += bcm_arm_setup.o
ifeq ($(strip $(BRCM_CHIP)),47189)
obj-$(CONFIG_ARM) += bcm_legacy_io_map.o
endif
obj-$(CONFIG_MIPS) += bcm_mips_setup.o

obj-y += setup.o bcm_dt.o

EXTRA_CFLAGS +=  -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/pmc
EXTRA_CFLAGS += -Werror -Wfatal-errors

ifeq ($(strip $(BRCM_CHIP)),6858)
obj-$(CONFIG_ARM64) += bcm_ru_map.o
EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource/drv/lport/ag -DBDMF_SESSION_H -D_BDMF_INTERFACE_H_ -D_BDMF_SYSTEM_H_ -DBDMF_MON_H -DNO_BDMF_HANDLE 
#EXTRA_CFLAGS += -I$(INC_RDP_FLAGS) -DBDMF_SESSION_H -D_BDMF_INTERFACE_H_ -D_BDMF_SYSTEM_H_ -DBDMF_MON_H -DNO_BDMF_HANDLE
endif
-include $(TOPDIR)/Rules.make

# mips platforms are still not modified to use these drivers
ifeq ($(filter-out 63138 63148 4908 6858,${BRCM_CHIP}),)
ifeq ($(PREBUILT_EXTRAMOD),1)
bcm_usb-objs := bcm_usb_preb.o
endif
obj-$(CONFIG_USB_XHCI_PLATFORM) += bcm_usb.o
obj-$(CONFIG_SATA_AHCI_PLATFORM) += bcm_sata.o
endif

clean:
	rm -f core *.o *.a *.s

