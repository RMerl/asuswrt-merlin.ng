#
# Makefile for the enetx driver
#

NAME = bcm_enet

ifneq ($(strip $(CONFIG_BCM_ENET)),)
obj-$(CONFIG_BCM_ENET) = $(NAME).o
endif

$(NAME)-objs += enet.o port.o bp_parsing.o
$(NAME)-objs += stub.o

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_47189_),)
$(NAME)-objs += 47189.o
$(NAME)-objs += 47189_dma.o
EXTRA_CFLAGS += $(INC_ENET_DMA_FLAGS)
EXTRA_CFLAGS += -DENET_DMA
EXTRA_CFLAGS += -DDT
endif

EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/drv

ifneq ($(strip $(CONFIG_BCM_RDPA)),)
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -DRUNNER
$(NAME)-objs += runner.o

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_),)
ifneq ($(strip $(CONFIG_BCM_PTP_1588)),)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/time_sync/impl$(CONFIG_BCM_TIME_SYNC_IMPL)
$(NAME)-objs += ptp_1588_oren.o
endif
endif

ifneq ($(strip $(CONFIG_BCM_WLAN)),)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/shared/impl1
EXTRA_CFLAGS += -DDSLCPE -DBCMDRIVER -DPKTC
endif

# 6858_BU use rdp ring workaround
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_),)
RDP_RING = 1
endif

ifneq ($(strip $(RDP_RING)),)
$(NAME)-objs += rdp_ring.o
else
$(NAME)-objs += enet_ring.o
endif

EXTRA_CFLAGS += -DENET_INT_COALESCING_ENABLE -DENET_INTERRUPT_COALESCING_TIMEOUT_US=500 -DENET_INTERRUPT_COALESCING_MAX_PKT_CNT=32

ifneq ($(strip $(BUILD_BRCM_FTTDP)),)
$(NAME)-objs += g9991.o
EXTRA_CFLAGS += -DBRCM_FTTDP
endif

ifneq (,$(filter $(BRCM_CHIP),6838 6848))
$(NAME)-objs += runner_wifi.o
EXTRA_CFLAGS += -DENET_RUNNER_WIFI
endif
endif

ifneq ($(strip $(CONFIG_BCM_GPON)),)
EXTRA_CFLAGS += -DGPON
endif
ifneq ($(strip $(CONFIG_BCM_EPON)),)
EXTRA_CFLAGS += -DEPON
endif

ifneq (,$(filter $(BRCM_CHIP),6838 6848 6858))
EXTRA_CFLAGS += -DSEPARATE_MAC_FOR_WAN_INTERFACES
endif

ifneq (,$(filter $(BRCM_CHIP),47189))
EXTRA_CFLAGS += -DSEPARATE_MAC_FOR_LAN_INTERFACES
endif

EXTRA_CFLAGS += -DIOCTL_COMPAT
$(NAME)-objs += bcmenet_ioctl_compat.o

# data demuxing example: (TODO: (de)mux working, vlan_id add/strip not implemented)
# EXTRA_CFLAGS += -DVLANTAG -DNEXT_LEVEL_MUX_REQUIRED -DNEXT_LEVEL_DEMUX_REQUIRED
# $(NAME)-objs += vlan_tag.o

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s *.ko .*.cmd .*.o.d Module.symvers modules.builtin modules.order *.mod.c
	rm -rf .tmp_versions

.PHONY: clean

