# File: bcmdrivers/broadcom/net/enet/impl6/Makefile
#
# Makefile for the BCM63xx Enet/switch driver
#

bcm_enet-objs += bcmenet.o

bcm_enet-objs += ../shared/bcmproc.o
bcm_enet-objs += bcmenet_dma.o
bcm_enet-objs += bcm_eth.o
bcm_enet-objs += bcmeth_dma.o

ifneq ($(strip $(BUILD_ETHTOOL)),)
bcm_enet-objs += ../shared/bcmenet_ethtool.o
endif


#-DDUMP_DATA  -- dump data
#-DUSE_BH     -- use bottom half
#-DUSE_PROC   -- display ethernet driver internal info
#-DUSE_EMAC   -- need MDC/MDIO functions
#-DBCM_ENET_RX_LOG   -- Enable rx debug log
#-DBCM_ENET_LOG      -- Enable debug log
#-DBCM_ENET_TX_LOG   -- Enable tx debug log
#-DBCM_ENET_LINK_LOG   -- Enable link debug log
#-DBCM_ENET_DEBUG_BUILD   -- Enable Debug Build
#-DRXCHANNEL_BYTE_RATE_LIMIT -- Enable rx byte rate limiting of a channel
#-DRXCHANNEL_PKT_RATE_LIMIT -- Enable rx pkt rate limiting of a channel
#-DVLAN_TAG_FFF_STRIP -- Enable Stripping of 0xFFF tagged packets
#-DUSE_DEFAULT_EGRESS_QUEUE -- Enable use of default egress queue
#-DSEPARATE_MAC_FOR_WAN_INTERFACES -- Enable separate MAC addresses for WAN ifs

ifneq ($(strip $(CONFIG_BCM_ENET)),)
obj-$(CONFIG_BCM_ENET) = bcm_enet.o
endif

ifneq ($(strip $(BUILD_HELLO)),)
# Enable support for HELLO Packet CMF
EXTRA_CFLAGS += -DSUPPORT_HELLO
endif

ifneq ($(strip $(BUILD_SWMDK)),)
EXTRA_CFLAGS += -DSUPPORT_SWMDK
endif


EXTRA_CFLAGS += -DUSE_BH
EXTRA_CFLAGS += -DUSE_VPORTS
EXTRA_CFLAGS += -DEXPORT_SYMTAB
EXTRA_CFLAGS += -Werror 
EXTRA_CFLAGS += -g
#EXTRA_CFLAGS +=  -DBCM_ENET_DEBUG_BUILD
#EXTRA_CFLAGS +=  -DBCM_ENET_RX_LOG
#EXTRA_CFLAGS +=  -DBCM_ENET_TX_LOG
EXTRA_CFLAGS +=  -DBCM_ENET_LOG
#EXTRA_CFLAGS +=  -DRXCHANNEL_BYTE_RATE_LIMIT
#EXTRA_CFLAGS +=  -DRXCHANNEL_PKT_RATE_LIMIT
EXTRA_CFLAGS +=  -DVLAN_TAG_FFF_STRIP
EXTRA_CFLAGS +=  -DUSE_DEFAULT_EGRESS_QUEUE 
EXTRA_CFLAGS += -DSEPARATE_MAC_FOR_WAN_INTERFACES
EXTRA_CFLAGS += -DREPORT_HARDWARE_STATS
#EXTRA_CFLAGS += -DDUMP_DATA
#EXTRA_CFLAGS += -DNON_LINUX_BUILD
#EXTRA_CFLAGS += -DNO_CFE
EXTRA_CFLAGS += -DBCM_ENET_UNIMAC
#EXTRA_CFLAGS += -DMEMCPY_UNALIGNED_WORKAROUND

ifneq ($(strip $(BUILD_ETHTOOL)),)
EXTRA_CFLAGS += -DSUPPORT_ETHTOOL
endif

ifneq ($(strip $(BRCM_CONFIG_HIGH_RES_TIMERS)),)
EXTRA_CFLAGS += -DMOCA_HIGH_RES_TX
endif

ifneq ($(strip $(BRCM_SWITCH_SCHED_SP)),)
EXTRA_CFLAGS += -DBCM_SWITCH_SCHED_SP
endif
ifneq ($(strip $(BRCM_SWITCH_SCHED_WRR)),)
EXTRA_CFLAGS += -DBCM_SWITCH_SCHED_WRR
endif


EXTRA_CFLAGS += -I$(obj) -I$(obj)/../shared
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/include/$(BRCM_BOARD)
ifeq ($(CONFIG_BCM_XTMRT_IMPL),1)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/xtmrt/impl3
else
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/xtmrt/impl$(CONFIG_BCM_XTMRT_IMPL)
endif
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/fap/impl$(CONFIG_BCM_FAP_IMPL)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/fap/impl$(CONFIG_BCM_FAP_IMPL)/4ke
EXTRA_CFLAGS += -I$(KERNEL_DIR)/net/core
ifneq ($(strip $(CONFIG_BCM_RDPA)),)
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
endif

ifneq ($(strip $(CONFIG_BCM_WLAN)),)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/include
EXTRA_CFLAGS += -DDSLCPE -DBCMDRIVER -DPKTC
endif
ifneq ($(strip $(BUILD_ETHTOOL)),)
EXTRA_CFLAGS += -DSUPPORT_ETHTOOL
endif

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s


