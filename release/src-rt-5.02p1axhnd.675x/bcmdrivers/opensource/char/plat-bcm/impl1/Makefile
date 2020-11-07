# File: bcmdrivers/opensource/char/plat-bcm
#
# Makefile for the BCM63xx/68xx  platform drivers
obj-$(CONFIG_ARM64) += bcm_legacy_io_map.o
obj-$(CONFIG_ARM64) += bcm_arm_irq.o
obj-$(CONFIG_ARM64) += bcm_arm64_setup.o
obj-$(CONFIG_ARM64) += bcm_memc_irq.o
obj-$(CONFIG_BCM_THERMAL) += bcm_thermal.o
obj-$(CONFIG_BCM_ARM_CPUIDLE) += bcm_arm_cpuidle.o
obj-$(CONFIG_BCM_UBUS4_DCM) += bcm_ubus4_dcm.o
obj-$(CONFIG_BCM_CFE_XARGS) += blxargs.o
obj-${CONFIG_BCM_CPUOFF} += bcm_cpuoff.o
obj-$(CONFIG_ARM) += bcm_arm_setup.o
obj-$(CONFIG_BCM_CMA_RSVMEM) += bcm_rsvmem.o

ifneq ($(findstring _$(strip ${BRCM_CHIP})_,_47189_6846_47622_63178_6878_),)
obj-$(CONFIG_ARM) += bcm_legacy_io_map.o 
endif
obj-$(CONFIG_ARM) += bcm_arm_irq.o
obj-$(CONFIG_ARM) += bcm_memc_irq.o
obj-$(CONFIG_MIPS) += bcm_mips_setup.o

obj-y += setup.o bcm_dt.o

ifeq ($(strip $(BRCM_IKOS)),)
obj-y += bcm_extirq.o
endif

ifneq ($(strip $(CONFIG_BCM_WLAN)),)
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/../char/board/$(BRCM_BOARD)/impl1
EXTRA_CFLAGS += -DWIRELESS
endif

ifeq ($(filter-out 47622 63178 6878,${BRCM_CHIP}),)
obj-$(CONFIG_ARM) += bcm_arm_smp.o bcm-ca9mp-headsmp.o bcm_arm_hotplug.o 
endif

EXTRA_CFLAGS +=  -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/pmc
EXTRA_CFLAGS += -Werror -Wfatal-errors

ifeq ($(filter-out 6858 63158 6846 6856 6878,${BRCM_CHIP}),)
obj-$(CONFIG_ARM)$(CONFIG_ARM64) += bcm_ru_map.o
EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource/drv/lport/ag -DBDMF_SESSION_H -D_BDMF_INTERFACE_H_ -D_BDMF_SYSTEM_H_ -DBDMF_MON_H -DNO_BDMF_HANDLE 
#EXTRA_CFLAGS += -I$(INC_RDP_FLAGS) -DBDMF_SESSION_H -D_BDMF_INTERFACE_H_ -D_BDMF_SYSTEM_H_ -DBDMF_MON_H -DNO_BDMF_HANDLE
endif
ifneq ($(strip $(BUILD_6802_MOCA)),)
EXTRA_CFLAGS +=  -I$(BRCMDRIVERS_DIR)/opensource/char/board/$(BRCM_BOARD)/impl$(CONFIG_BCM_BOARD_IMPL)
endif
-include $(TOPDIR)/Rules.make

# mips platforms are still not modified to use these drivers
ifeq ($(filter-out 63138 63148 4908 6858 63158 6846 6878 47622 47189 6856 63178 47622,${BRCM_CHIP}),)
obj-$(CONFIG_USB_EHCI_HCD) += bcm_usb.o
obj-$(CONFIG_SATA_AHCI_PLATFORM) += bcm_sata.o
endif
obj-$(CONFIG_BCM_I2C_CHIP) += bcm_i2c.o

clean:
	rm -f core *.o *.a *.s

