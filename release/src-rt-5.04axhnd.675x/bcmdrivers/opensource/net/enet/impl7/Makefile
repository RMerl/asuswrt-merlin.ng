#
# Makefile for the enetx driver
#

NAME = bcm_enet

obj-$(CONFIG_BCM_ENET) = $(NAME).o

ifneq (,$(filter $(BRCM_CHIP),6858 6846 6856 6878 6855))
ifneq ($(wildcard $(BRCMDRIVERS_DIR)/opensource/net/enet/impl7/enet_ingress),)
obj-y += enet_ingress/
endif
endif

EXTRA_CFLAGS += -g

EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/include/bcm963xx
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/include/bcm963xx/pmc
EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource/include/bcm963xx
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx
EXTRA_CFLAGS += -I$(KERNEL_DIR)/net/bridge

ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
EXTRA_CFLAGS += -DBCM_PKTFWD

ifneq ($(strip $(CONFIG_BCM_EAPFWD)),)
EXTRA_CFLAGS += -DBCM_EAPFWD
endif

endif

ifneq ($(CONFIG_BCM_ARCHER),)
$(NAME)-objs += archer_enet.o
else
EXTRA_CFLAGS += -DEMBEDDED_BRCMTAG_TX_INSERT
EXTRA_CFLAGS += -DEMBEDDED_BRCMTAG_RX_REMOVE
endif

$(NAME)-objs += enet.o port.o mux_index.o port_types.o
$(NAME)-objs += stub.o

EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/phy
EXTRA_CFLAGS += -DENET_DT
$(NAME)-objs += dt_parsing.o

# proc interface
$(NAME)-objs += bcmenet_proc.o
EXTRA_CFLAGS += -DENET_PROC

ifneq ($(strip $(CONFIG_BRCM_QEMU)),)
$(NAME)-objs += bcmenet_proc_qemu.o
endif

# Debug capability rx cpu traffic; depends on proc interface
# EXTRA_CFLAGS += -DENET_DEBUG_RX_CPU_TRAFFIC

EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/drv -I$(SHARED_DIR)/opensource

ifneq ($(strip $(CONFIG_BCM_RDPA)),)
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -DRUNNER
EXTRA_CFLAGS += -DONE_CHNL_TWO_QUEUE_PER_CHANNEL
$(NAME)-objs += runner.o
ifeq (,$(filter $(BRCM_CHIP),63138 63148 4908 63158))
$(NAME)-objs += runner_standalone.o
ifneq ($(strip $(CONFIG_NET_SWITCHDEV)),)
$(NAME)-objs += runner_sdev.o
endif
endif
ifneq ($(strip $(CONFIG_BCM_RNR_CPU_RX_DYN_METER)),)
$(NAME)-objs += dynamic_meters.o
endif
ifneq ($(strip $(CONFIG_BCM_PTP_1588)),)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/time_sync/impl$(CONFIG_BCM_TIME_SYNC_IMPL)
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_63158_6878_6855_),)
$(NAME)-objs += ptp_1588_hwts.o
endif
endif

# CPU ring implementation
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_63158_6878_63146_4912_6813_6855_),)
RDP_RING = 1
endif
ifneq ($(strip $(RDP_RING)),)
$(NAME)-objs += rdp_ring.o
else
$(NAME)-objs += enet_ring.o
endif

EXTRA_CFLAGS += -DENET_INT_COALESCING_ENABLE -DENET_INTERRUPT_COALESCING_TIMEOUT_US=500 -DENET_INTERRUPT_COALESCING_MAX_PKT_CNT=32 


ifneq ($(strip $(CONFIG_BCM_FTTDP_G9991)),)
$(NAME)-objs += g9991.o
endif
endif

ifneq ($(strip $(CONFIG_BCM_ETHTOOL)),)
$(NAME)-objs += enet_ethtool.o
$(NAME)-objs += ../shared/bcmenet_ethtool.o
endif

ifneq ($(strip $(CONFIG_BCM_GPON)),)
EXTRA_CFLAGS += -DGPON
EXTRA_CFLAGS += -DCC_DROP_PRECEDENCE
endif
ifneq ($(strip $(CONFIG_BCM_EPON)),)
EXTRA_CFLAGS += -DEPON
endif

ifneq (,$(filter $(BRCM_CHIP),6858 6846 6856 63158 6878 6855))
ifeq ($(strip $(CONFIG_BCM_ARCHER)),)
$(NAME)-objs += phy_drv_pon.o
EXTRA_CFLAGS += -DPHY_PON
endif
endif

# Except HNDROUTER, all BCACPE platforms
ifneq ($(strip $(BUILD_BRCM_HNDROUTER)),)
else
EXTRA_CFLAGS += -DSEPARATE_MAC_FOR_WAN_INTERFACES
endif

ifneq (,$(filter $(BRCM_CHIP),63158))
    EXTRA_CFLAGS += -DRUNNER_PFC_TX
endif

ifeq ($(strip $(CONFIG_BCM_PON)),)
ifeq ($(strip $(CONFIG_BCM_DSL_XRDP)),)
EXTRA_CFLAGS += -DENET_ARP_LCP_HI_PRIO
endif
endif

ifneq (,$(filter $(BRCM_CHIP),63158 63146 6756 4912 6813))
    EXTRA_CFLAGS += -DHALT_ON_ANY_PORT_FAILURE
endif

ifneq (,$(filter $(BRCM_CHIP),63138 63148 4908 63158))
ifeq ($(CONFIG_BCM_ARCHER),)
    $(NAME)-objs += runner_with_switch.o
else
    $(NAME)-objs += sf2_standalone.o
    EXTRA_CFLAGS += -DARCHER_DEVICE
endif
# DSL devices
    EXTRA_CFLAGS += -DDSL_DEVICES -DPHY_SF2_SERDES
# devices with starfighter2
    EXTRA_CFLAGS += -DNETDEV_HW_SWITCH -DSF2_DEVICE -DSF2_CFP
    $(NAME)-objs += sf2.o sf2_sdev.o sf2_cfp.o enet_extlh.o
    $(NAME)-objs += sw_common.o
ifneq (,$(filter $(BRCM_CHIP),63138 63158 4908))
    EXTRA_CFLAGS += -DACB_ALGORITHM2
endif
ifneq (,$(filter $(BRCM_CHIP),63138))
    EXTRA_CFLAGS += -DCRB_5X3_QGPHY3_WORKAROUND
endif
endif

ifneq (,$(filter $(BRCM_CHIP),63178))
    EXTRA_CFLAGS += -DNETDEV_HW_SWITCH -DDSL_DEVICES
    EXTRA_CFLAGS += -DARCHER_DEVICE
    $(NAME)-objs += sf2_standalone.o
    EXTRA_CFLAGS += -DSF2_DEVICE
    $(NAME)-objs += sf2.o  sf2_sdev.o enet_extlh.o
    $(NAME)-objs += sw_common.o
    EXTRA_CFLAGS += -DACB_ALGORITHM2
endif

ifneq (,$(filter $(BRCM_CHIP),47622))
    EXTRA_CFLAGS += -DNETDEV_HW_SWITCH -DDSL_DEVICES
    EXTRA_CFLAGS += -DARCHER_DEVICE
    EXTRA_CFLAGS += -DSYSPVSW_DEVICE
    $(NAME)-objs += syspvsw.o
    
    EXTRA_CFLAGS += -DSF2_DEVICE -DSF2_EXTERNAL
    $(NAME)-objs += sf2.o sf2_sdev.o syspvsw_with_switch.o enet_extlh.o
    $(NAME)-objs += sw_common.o
endif

ifneq (,$(filter $(BRCM_CHIP),6756))
    EXTRA_CFLAGS += -DNETDEV_HW_SWITCH -DDSL_DEVICES
    EXTRA_CFLAGS += -DARCHER_DEVICE
    $(NAME)-objs += sf2_with_switch.o
    EXTRA_CFLAGS += -DSF2_DEVICE
    EXTRA_CFLAGS += -DSF2_DUAL
    $(NAME)-objs += sf2.o sf2_sdev.o enet_extlh.o
    $(NAME)-objs += sw_common.o
    EXTRA_CFLAGS += -DACB_ALGORITHM2
endif

ifneq (,$(filter $(BRCM_CHIP),63146 4912 6813))
    EXTRA_CFLAGS += -DDSL_DEVICES
    $(NAME)-objs += enet_extlh.o
endif

EXTRA_CFLAGS += -DIOCTL_COMPAT -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
$(NAME)-objs += bcmenet_ioctl_compat.o

# data demuxing example: (TODO: (de)mux working, vlan_id add/strip not implemented)
# EXTRA_CFLAGS += -DVLANTAG -DNEXT_LEVEL_MUX_REQUIRED -DNEXT_LEVEL_DEMUX_REQUIRED
# $(NAME)-objs += vlan_tag.o

ifneq ($(strip $(CONFIG_BCM_WLAN)),)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/shared/impl1
EXTRA_CFLAGS += -DDSLCPE -DBCMDRIVER -DPKTC
endif

-include $(TOPDIR)/Rules.make

EXTRA_CFLAGS += -Werror -Wall

clean:
	rm -f core *.o *.a *.s *.ko .*.cmd .*.o.d Module.symvers modules.builtin modules.order *.mod.c
	rm -rf .tmp_versions

.PHONY: clean

