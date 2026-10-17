#
# Makefile for the Broadcom WL shared Layer
#

-include $(WLAN_SHARED_DIR)/wifi_cfg_common.mk

wlshared-objs := wlshared_linux.o wl_br_d3lut.o wlan_rsvmem.o
ifneq ($(BUILD_HND_EAP_AP1),y)
wlshared-objs += dhd_br_d3lut.o
endif


EXTRA_CFLAGS += -I$(WLAN_SHARED_DIR)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/include/linux
EXTRA_CFLAGS += -I$(KERNEL_DIR)/net/bridge
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/wl/config
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx

KBUILD_CFLAGS += -DBCMDRIVER

ifneq ($(strip $(CONFIG_BLOG)),)
    EXTRA_CFLAGS    += -DBCM_BLOG
endif

ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
    EXTRA_CFLAGS    += -DBCM_PKTFWD
endif

obj-$(CONFIG_BCM_WLAN) := wlshared.o

clean-files += wlshared_linux.o wlshared.o wl_br_d3lut.o wlan_rsvmem.o

ifneq ($(BUILD_HND_EAP_AP1),y)
clean-files += dhd_br_d3lut.o
endif
