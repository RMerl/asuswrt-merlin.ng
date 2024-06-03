# Makefile for the bcm963xx board drivers
#

obj-$(CONFIG_BCM_VFBIO_DRV)  := vfbio.o
obj-$(CONFIG_BCM_VFBIO_OOPS) := vfbio_oops.o

vfbio-y += vfbio_core.o vfbio_proc.o
vfbio-y += vfbio_lvm.o vfbio_lvm_ioctl.o

EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/block/

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean



##  erouter Makefile ##
#ccflags-y += -Werror -Wfatal-errors
#ccflags-y += -Idrivers/bcm_media_gw/include

#vfbio-y += vfbio_core.o vfbio_proc.o

#obj-$(CONFIG_BCM_VFBIO) := vfbio.o
#obj-$(CONFIG_BCM_VFBIO_OOPS) := vfbio_oops.o
