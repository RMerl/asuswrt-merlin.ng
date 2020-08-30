#
# Makefile for the enetx driver
#

NAME = bcm_enet

obj-$(CONFIG_BCM_ENET) = $(NAME).o

ifneq (,$(filter $(BRCM_CHIP),6858 6846 6856))
ifneq ($(wildcard $(BRCMDRIVERS_DIR)/opensource/net/enet/impl7/enet_ingress),)
obj-y += enet_ingress/
endif
endif

# Below block is used when SYSPORT is defined for 63158 & 4908 
ifneq ($(PREBUILT_EXTRAMOD),1)
ifeq ($(strip $(CONFIG_BCM_ENET_SYSPORT)),y)
$(NAME)-objs += enet_sysport_drv.o
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_),)
BCMGMAC_PATH = $(BRCMDRIVERS_DIR)/opensource/net/enet/bcm9$(BRCM_CHIP)
$(shell rm -f $(BCMGMAC_PATH)/bcmgmac_4908.c)
$(shell rm -f $(BCMGMAC_PATH)/bcmgmac_4908.h)
$(shell ln -s -f $(BRCMDRIVERS_DIR)/opensource/net/enet/impl5/bcmgmac_4908.c $(BCMGMAC_PATH)/bcmgmac_4908.c)
$(shell ln -s -f $(BRCMDRIVERS_DIR)/opensource/include/bcm963xx/bcmgmac_4908.h $(BCMGMAC_PATH)/bcmgmac_4908.h)
$(NAME)-objs += bcmgmac_4908.o
endif
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/include/bcm963xx
EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource/include/bcm963xx
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/customerApi/impl1
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx

# End Sysport block 
else

$(NAME)-objs += enet.o port.o bp_parsing.o mux_index.o port_types.o
$(NAME)-objs += stub.o

# proc interface
$(NAME)-objs += bcmenet_proc.o
EXTRA_CFLAGS += -DENET_PROC

# Debug capability rx cpu traffic; depends on proc interface
# EXTRA_CFLAGS += -DENET_DEBUG_RX_CPU_TRAFFIC

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_47189_),)
$(NAME)-objs += 47189.o
$(NAME)-objs += 47189_dma.o
EXTRA_CFLAGS += $(INC_ENET_DMA_FLAGS)
EXTRA_CFLAGS += -DENET_DMA
EXTRA_CFLAGS += -DDT
endif

EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/drv -I$(SHARED_DIR)/opensource -I$(KERNEL_DIR)/net/bridge

ifneq ($(strip $(CONFIG_BCM_RDPA)),)
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -DRUNNER
EXTRA_CFLAGS += -DONE_CHNL_TWO_QUEUE_PER_CHANNEL
$(NAME)-objs += runner.o
ifneq ($(strip $(CONFIG_BCM_RNR_CPU_RX_DYN_METER)),)
$(NAME)-objs += dynamic_meters.o
endif
ifneq ($(strip $(CONFIG_BCM_PTP_1588)),)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/time_sync/impl$(CONFIG_BCM_TIME_SYNC_IMPL)
EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource/drv/lport -I$(SHARED_DIR)/opensource/drv/lport/ag
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_),)
$(NAME)-objs += ptp_1588_oren.o
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6836_6846_6856_),)
$(NAME)-objs += ptp_1588_hwts.o
endif
endif

# CPU ring implementation
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6836_6846_6856_63158_),)
RDP_RING = 1
endif
ifneq ($(strip $(RDP_RING)),)
$(NAME)-objs += rdp_ring.o
else
$(NAME)-objs += enet_ring.o
endif

# 6848 6838 CPU meters
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6848_6838_),)
# EXTRA_CFLAGS += -DENET_CPU_LOW_PR_Q_METER_ENABLE
endif

EXTRA_CFLAGS += -DENET_INT_COALESCING_ENABLE -DENET_INTERRUPT_COALESCING_TIMEOUT_US=500 -DENET_INTERRUPT_COALESCING_MAX_PKT_CNT=32 

ifneq ($(strip $(BUILD_ETHTOOL)),)
EXTRA_CFLAGS += -DSUPPORT_ETHTOOL
$(NAME)-objs += enet_ethtool.o
$(NAME)-objs += ../shared/bcmenet_ethtool.o
endif

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
EXTRA_CFLAGS += -DCC_DROP_PRECEDENCE
endif
ifneq ($(strip $(CONFIG_BCM_EPON)),)
EXTRA_CFLAGS += -DEPON
endif

ifneq (,$(filter $(BRCM_CHIP),6838 6848 6858 6836 6846 6856 63158))
$(NAME)-objs += phy_drv_pon.o
EXTRA_CFLAGS += -DPHY_PON
endif

# Except 47189 and HNDROUTER, all BCACPE platforms
ifneq ($(strip $(BUILD_BRCM_HNDROUTER)),)
else
ifeq (,$(filter $(BRCM_CHIP),47189))
EXTRA_CFLAGS += -DSEPARATE_MAC_FOR_WAN_INTERFACES
endif
endif

ifneq (,$(filter $(BRCM_CHIP),47189))
EXTRA_CFLAGS += -DSEPARATE_MAC_FOR_LAN_INTERFACES
endif

ifneq (,$(filter $(BRCM_CHIP),63138 63148 4908 63158))
$(NAME)-objs += sf2_cfp.o
# DSL devices with runner
    EXTRA_CFLAGS += -DDSL_RUNNER_DEVICE 
ifneq (,$(filter $(BRCM_CHIP),63158))
    EXTRA_CFLAGS += -DHALT_ON_ANY_PORT_FAILURE -DPHY_SERDES_10G_CAPABLE
endif
# devices with starfighter2
    EXTRA_CFLAGS += -DSF2_DEVICE
    $(NAME)-objs += sf2.o
ifneq (,$(filter $(BRCM_CHIP),63138 63158 4908))
    EXTRA_CFLAGS += -DACB_ALGORITHM2
endif
ifneq (,$(filter $(BRCM_CHIP),63138))
    EXTRA_CFLAGS += -DCRB_5X3_QGPHY3_WORKAROUND
endif
endif

EXTRA_CFLAGS += -DIOCTL_COMPAT -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
$(NAME)-objs += bcmenet_ioctl_compat.o

# data demuxing example: (TODO: (de)mux working, vlan_id add/strip not implemented)
# EXTRA_CFLAGS += -DVLANTAG -DNEXT_LEVEL_MUX_REQUIRED -DNEXT_LEVEL_DEMUX_REQUIRED
# $(NAME)-objs += vlan_tag.o

endif

ifneq ($(strip $(CONFIG_BCM_WLAN)),)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/shared/impl1
EXTRA_CFLAGS += -DDSLCPE -DBCMDRIVER -DPKTC
endif

else
bcm_enet-objs += bcm_enet$(PRBM_EXT).o
endif   ## PREBUILT_EXTRAMOD


-include $(TOPDIR)/Rules.make

EXTRA_CFLAGS += -Werror -Wall

clean:
	rm -f core *.o *.a *.s *.ko .*.cmd .*.o.d Module.symvers modules.builtin modules.order *.mod.c
	rm -rf .tmp_versions

.PHONY: clean

