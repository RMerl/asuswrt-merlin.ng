# File: bcmdrivers/opensource/char/plat-bcm
#
# Makefile for the BCM63xx/68xx  platform drivers
obj-$(CONFIG_ARM64) += bcm_arm_irq.o
obj-$(CONFIG_ARM64) += bcm_arm64_setup.o
obj-$(CONFIG_BCM_CFE_XARGS) += blxargs.o
obj-$(CONFIG_ARM) += bcm_arm_setup.o
obj-$(CONFIG_BCM_CMA_RSVMEM) += bcm_rsvmem.o

obj-$(CONFIG_ARM) += bcm_arm_irq.o
obj-$(CONFIG_BCM_GLB_COHERENCY) += bcm_coherency.o

obj-y += setup.o bcm_dt.o

ifneq ($(strip $(CONFIG_BRCM_QEMU)),)
    obj-y += qemu/qemu_stubs.o
endif

ifneq ($(strip $(CONFIG_BCM_WLAN)),)
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/../char/board/$(BRCM_BOARD)/impl1
EXTRA_CFLAGS += -DWIRELESS
endif

obj-$(CONFIG_ARM) += bcm_arm_smp.o bcm-ca9mp-headsmp.o bcm_arm_hotplug.o 

EXTRA_CFLAGS +=  -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -Werror -Wfatal-errors

ifeq ($(filter-out 6858 63158 6846 6856 6878 63146 4912 6855 6888 6813 ,${BRCM_CHIP}),)
EXTRA_CFLAGS += -DBDMF_SESSION_H -D_BDMF_INTERFACE_H_ -D_BDMF_SYSTEM_H_ -DBDMF_MON_H -DNO_BDMF_HANDLE 
#EXTRA_CFLAGS += -I$(INC_RDP_FLAGS) -DBDMF_SESSION_H -D_BDMF_INTERFACE_H_ -D_BDMF_SYSTEM_H_ -DBDMF_MON_H -DNO_BDMF_HANDLE
endif

ifeq ($(filter-out 63146 4912 6813,${BRCM_CHIP}),)
EXTRA_CFLAGS += -DCONFIG_RDPA_NEW_RSVMEM
endif

-include $(TOPDIR)/Rules.make

obj-$(CONFIG_BCM_M2M_DMA) += bcm_m2mdma.o

clean:
	rm -f core *.o *.a *.s

