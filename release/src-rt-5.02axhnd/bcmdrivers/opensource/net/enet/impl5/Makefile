# File: bcmdrivers/broadcom/net/enet/bcm963xx/Makefile
#
# Makefile for the BCM63xx Enet/switch driver
#
bcm_enet-objs += bcmenet.o
bcm_enet-objs += ethsw.o
bcm_enet-objs += bcmsw.o
bcm_enet-objs += bcmsw_cfp.o
bcm_enet-objs += ../shared/bcmswaccess.o
bcm_enet-objs += eth_pwrmngt.o
bcm_enet-objs += ethsw_phy.o
bcm_enet-objs += bcmenet_shared_src.o
ifeq ($(strip $(BRCM_CHIP)),47189)
bcm_enet-objs += bcmgmac_47189.o
endif

bcm_enet-objs += ../shared/bcmswaccess_dma.o
bcm_enet-objs += ../shared/bcmproc.o
bcm_enet-objs += ethsw_phy_dma.o
bcm_enet-objs += bcmenet_dma.o
bcm_enet-objs += ethsw_dma.o
bcm_enet-objs += bcmsw_dma.o

EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)

ifneq ($(strip $(CONFIG_BCM_GMAC)),)
ifneq ($(strip $(CONFIG_BCM_ENET_4908_GMAC)),y)
bcm_enet-objs += bcmgmac.o
endif
endif

ifneq ($(strip $(CONFIG_BCM_TM)),)
ifeq ($(strip $(BRCM_CHIP)),63381)
EXTRA_CFLAGS += -D_CONFIG_ENET_BCM_TM
endif
endif

ifneq ($(strip $(BUILD_ETHTOOL)),)
bcm_enet-objs += ../shared/bcmenet_ethtool.o
bcm_enet-objs += enet_ethtool.o
endif

ifneq ($(strip $(CONFIG_BCM_ENET)),)
obj-$(CONFIG_BCM_ENET) = bcm_enet.o
endif

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_4908_),_$(strip $(BRCM_CHIP))_)
EXTRA_CFLAGS += -DACB_ALGORITHM2
endif

ifeq ($(strip $(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)),y)
#EXTRA_CFLAGS += -DBCM_ENET_MULTI_IMP_SUPPORT_USE_LAG
endif

ifeq ($(strip $(BRCM_CHIP)),63381)
EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource/include/pmc
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
#-DUSE_DEFAULT_EGRESS_QUEUE -- Enable use of default egress queue
#-DSEPARATE_MAC_FOR_WAN_INTERFACES -- Enable separate MAC addresses for WAN ifs

ifneq ($(strip $(BUILD_HELLO)),)
# Enable support for HELLO Packet CMF
EXTRA_CFLAGS += -DSUPPORT_HELLO
endif

ifneq ($(strip $(BUILD_SWMDK)),)
EXTRA_CFLAGS += -DSUPPORT_SWMDK
endif

ifneq ($(strip $(BUILD_ETHTOOL)),)
EXTRA_CFLAGS += -DSUPPORT_ETHTOOL
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
ifneq ($(strip $(BRCM_CHIP)),47189)
EXTRA_CFLAGS +=  -DRXCHANNEL_PKT_RATE_LIMIT
endif
EXTRA_CFLAGS +=  -DVLAN_TAG_FFF_STRIP
EXTRA_CFLAGS +=  -DUSE_DEFAULT_EGRESS_QUEUE 
EXTRA_CFLAGS += -DSEPARATE_MAC_FOR_WAN_INTERFACES
# Stats from HW is supported only from Switch at this time; not from Runner
# This needs to be looked into again in 4.14L.02 when flows will be acceleated/switched by Runner
EXTRA_CFLAGS += -DREPORT_HARDWARE_STATS
#EXTRA_CFLAGS += -DDUMP_DATA
#EXTRA_CFLAGS += -DNON_LINUX_BUILD
#EXTRA_CFLAGS += -DNO_CFE
# BCM_ENET_CB_WAN_PORT_LNX_INTF_SUPPORT must only be enabled when customer is trying to connect
# different type of WAN interface (like Ethernet and MoCA) and need to have these two represeted 
# as different Linux interfaces to configure the WAN service
#EXTRA_CFLAGS += -DBCM_ENET_CB_WAN_PORT_LNX_INTF_SUPPORT
ifneq ($(strip $(BRCM_CONFIG_HIGH_RES_TIMERS)),)
EXTRA_CFLAGS += -DMOCA_HIGH_RES_TX
endif

ifneq ($(strip $(BRCM_SWITCH_SCHED_SP)),)
EXTRA_CFLAGS += -DBCM_SWITCH_SCHED_SP
endif
ifneq ($(strip $(BRCM_SWITCH_SCHED_WRR)),)
EXTRA_CFLAGS += -DBCM_SWITCH_SCHED_WRR
endif
ifneq ($(strip $(BUILD_ETHTOOL)),)
EXTRA_CFLAGS += -DSUPPORT_ETHTOOL
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
EXTRA_CFLAGS += -I$(KERNEL_DIR)/net/core -I$(KERNEL_DIR)/net/bridge

ifneq ($(strip $(CONFIG_BCM_WLAN)),)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/include
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/shared/impl1
EXTRA_CFLAGS += -DDSLCPE -DBCMDRIVER -DPKTC
endif

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

