ifeq ($(strip ${BRCM_CHIP}),6858)
    DRV_OBJS += lport/lport_drv.o
    DRV_OBJS += lport/lport_mdio.o
    DRV_OBJS += lport/lport_stats.o
    DRV_OBJS += lport/lport_intr.o
    DRV_OBJS += lport/lport_proc.o
    DRV_OBJS += lport/serdes_access.o
    DRV_OBJS += $(patsubst %.c, %.o, $(shell cd $(src) && find lport/ag  -type f -name '*.c')) 
ifeq ($(strip ${CONFIG_MERLIN_SDK}),)
    DRV_OBJS += lport/merlin_direct_access.o
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_63138_63148_4908_63158_6846_6856_),)
    DRV_OBJS += unimac/unimac_drv_impl1.o
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_6858_47189_63138_63148_4908_63158_6846_6856_),)
    EXTRA_CFLAGS += -DPHY_CROSSBAR
    DRV_OBJS += phys/phy_drv_crossbar.o
    DRV_OBJS += phys/mac_drv.o
    DRV_OBJS += phys/bus_drv.o
    DRV_OBJS += phys/phy_drv.o
    DRV_OBJS += phys/phy_drv_mii.o
    DRV_OBJS += phys/phy_drv_brcm.o
    DRV_OBJS += phys/phy_bp_parsing.o
ifeq ($(strip ${BRCM_CHIP}),6838)
    DRV_OBJS += mdio/mdio_drv_common.o
    DRV_OBJS += mdio/mdio_drv_impl1.o
    DRV_OBJS += phys/bus_drv_6838.o
    DRV_OBJS += phys/mac_drv_unimac.o
    DRV_OBJS += phys/phy_bp_parsing_6838.o
    DRV_OBJS += phys/phy_drv_6848_egphy.o
    DRV_OBJS += phys/phy_drv_ext1.o
    DRV_OBJS += phys/phy_drv_pcs.o
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_6848_EGPHY -DPHY_EXT1 -DPHY_PCS
endif
ifeq ($(strip ${BRCM_CHIP}),6848)
    DRV_OBJS += mdio/mdio_drv_common.o
    DRV_OBJS += mdio/mdio_drv_impl2.o
    DRV_OBJS += phys/bus_drv_6848.o
    DRV_OBJS += phys/mac_drv_unimac.o
    DRV_OBJS += phys/phy_bp_parsing_6848.o
    DRV_OBJS += phys/phy_drv_6848_egphy.o
    DRV_OBJS += phys/phy_drv_6848_ephy.o
    DRV_OBJS += phys/phy_drv_6848_sgmii.o
    DRV_OBJS += phys/phy_drv_sgmii_plus2.o
    DRV_OBJS += phys/phy_drv_ext1.o
    DRV_OBJS += phys/phy_drv_pcs.o
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_6848_EGPHY -DPHY_6848_EPHY -DPHY_6848_SGMII -DPHY_EXT1 -DPHY_PCS
endif
ifeq ($(strip ${BRCM_CHIP}),6858)
    DRV_OBJS += phys/bus_drv_6858.o
    DRV_OBJS += phys/mac_drv_lport.o
    DRV_OBJS += phys/phy_bp_parsing_6858.o
    DRV_OBJS += phys/phy_drv_lport.o
    DRV_OBJS += phys/phy_drv_6858_egphy.o
    DRV_OBJS += phys/phy_drv_ext1.o
    DRV_OBJS += phys/phy_drv_ext2.o
    DRV_OBJS += phys/phy_drv_ext3.o
    DRV_OBJS += phys/phy_drv_lport_serdes.o
    EXTRA_CFLAGS += -DMAC_LPORT
    EXTRA_CFLAGS += -DPHY_6858_EGPHY -DPHY_EXT1 -DPHY_EXT2 -DPHY_EXT3 -DPHY_LPORT_SERDES
endif
ifeq ($(strip ${BRCM_CHIP}),6856)
    DRV_OBJS += mdio/mdio_drv_common.o
    DRV_OBJS += mdio/mdio_drv_impl5.o
    DRV_OBJS += phys/bus_drv_6846.o
    DRV_OBJS += phys/mac_drv_unimac.o
    DRV_OBJS += phys/phy_bp_parsing_6856.o
    DRV_OBJS += phys/phy_drv_6846_egphy.o
    DRV_OBJS += phys/phy_drv_6856_sgmii.o
    DRV_OBJS += phys/phy_drv_sgmii_plus2.o
    DRV_OBJS += phys/phy_drv_ext1.o
    DRV_OBJS += phys/phy_drv_ext3.o
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_6846_EGPHY -DPHY_6856_SGMII -DPHY_EXT1 -DPHY_EXT3 
endif
ifeq ($(strip ${BRCM_CHIP}),6846)
    DRV_OBJS += mdio/mdio_drv_common.o
    DRV_OBJS += mdio/mdio_drv_impl5.o
    DRV_OBJS += phys/bus_drv_6846.o
    DRV_OBJS += phys/mac_drv_unimac.o
    DRV_OBJS += phys/phy_bp_parsing_6846.o
    DRV_OBJS += phys/phy_drv_6846_egphy.o
    DRV_OBJS += phys/phy_drv_ext1.o
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_6846_EGPHY -DPHY_EXT1
endif

ifeq ($(strip ${BRCM_CHIP}),47189)
    DRV_OBJS += phys/bus_drv_47189.o
    DRV_OBJS += mdio/mdio_drv_impl3.o
    DRV_OBJS += phys/mac_drv_gmac.o
    DRV_OBJS += phys/phy_bp_parsing_47189.o
    DRV_OBJS += phys/phy_drv_53125.o
    DRV_OBJS += phys/phy_drv_ext1.o
    EXTRA_CFLAGS += -DMAC_GMAC
    EXTRA_CFLAGS += -DPHY_EXT_SWITCH -DPHY_EXT1
endif
ifeq ($(strip ${BRCM_CHIP}),63138)
    DRV_OBJS += phys/bus_drv_sf2.o
    DRV_OBJS += mdio/mdio_drv_common.o
    DRV_OBJS += mdio/mdio_drv_sf2.o
    DRV_OBJS += phys/mac_drv_unimac.o
    DRV_OBJS += phys/mac_drv_sf2.o
    DRV_OBJS += phys/phy_bp_parsing_63138.o
    DRV_OBJS += phys/phy_drv_sf2.o
    DRV_OBJS += ../drivers/bcm_ethsw_impl1.o
    EXTRA_CFLAGS += -DMAC_UNIMAC -DMAC_SF2 -DPHY_SF2 -DPHY_M2M -DSF2_DEVICE -DDSL_RUNNER_DEVICE
    EXTRA_CFLAGS += -DCRB_5X3_QGPHY3_WORKAROUND
endif
ifeq ($(strip ${BRCM_CHIP}),63148)
    DRV_OBJS += phys/bus_drv_sf2.o
    DRV_OBJS += mdio/mdio_drv_common.o
    DRV_OBJS += mdio/mdio_drv_sf2.o
    DRV_OBJS += phys/mac_drv_unimac.o
    DRV_OBJS += phys/mac_drv_sf2.o
    DRV_OBJS += phys/phy_bp_parsing_63148.o
    DRV_OBJS += phys/phy_drv_sf2.o
    DRV_OBJS += ../drivers/bcm_ethsw_impl1.o
    EXTRA_CFLAGS += -DMAC_UNIMAC -DMAC_SF2 -DPHY_SF2 -DPHY_M2M -DSF2_DEVICE -DDSL_RUNNER_DEVICE
endif
ifeq ($(strip ${BRCM_CHIP}),63158)
    DRV_OBJS += phys/bus_drv_sf2.o
    DRV_OBJS += mdio/mdio_drv_common.o
    DRV_OBJS += mdio/mdio_drv_sf2.o
    DRV_OBJS += phys/mac_drv_unimac.o
    DRV_OBJS += phys/mac_drv_sf2.o
    DRV_OBJS += phys/mac_drv_xport.o
    DRV_OBJS += phys/phy_bp_parsing_63158.o
    DRV_OBJS += phys/phy_drv_sf2.o
    DRV_OBJS += phys/phy_drv_xgae.o
    DRV_OBJS += phys/phy_drv_ext3.o
    DRV_OBJS += ../drivers/bcm_ethsw_impl1.o
    DRV_OBJS += xport/impl1/xport_drv.o
    DRV_OBJS += xport/impl1/xport_stats.o
    DRV_OBJS += xport/impl1/xport_proc.o
    DRV_OBJS += $(patsubst %.c, %.o, $(shell cd $(src) && find xport/impl1/ag  -type f -name '*.c')) 

    EXTRA_CFLAGS += -DMAC_UNIMAC -DMAC_SF2 -DPHY_SF2 -DPHY_M2M -DSF2_DEVICE -DDSL_RUNNER_DEVICE
    EXTRA_CFLAGS += -DMAC_XPORT
    EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource/drv/xport/impl1 -I$(SHARED_DIR)/opensource/drv/xport/impl1/ag
    EXTRA_CFLAGS += -DPHY_SERDES_2P5G_CAPABLE -DPHY_SERDES_10G_CAPABLE -DPHY_EXT3
endif
ifeq ($(strip ${BRCM_CHIP}),4908)
    DRV_OBJS += phys/bus_drv_sf2.o
    DRV_OBJS += mdio/mdio_drv_common.o
    DRV_OBJS += mdio/mdio_drv_sf2.o
    DRV_OBJS += phys/mac_drv_unimac.o
    DRV_OBJS += phys/mac_drv_sf2.o
    DRV_OBJS += phys/phy_bp_parsing_4908.o
    DRV_OBJS += phys/phy_drv_sf2.o
    DRV_OBJS += phys/phy_drv_ext3.o
    DRV_OBJS += ../drivers/bcm_ethsw_impl1.o
    EXTRA_CFLAGS += -DMAC_UNIMAC -DMAC_SF2 -DPHY_SF2 -DPHY_M2M -DSF2_DEVICE -DDSL_RUNNER_DEVICE
    EXTRA_CFLAGS += -DPHY_SERDES_2P5G_CAPABLE -DPHY_EXT3
ifeq ($(HND_EXT_53134),y)
    DRV_OBJS += phys/phy_drv_53125.o
    EXTRA_CFLAGS += -DPHY_EXT_SWITCH -DEXT_BCM53134
endif
endif
endif

obj-y +=  $(DRV_OBJS)

EXTRA_CFLAGS += -I$(INC_BDMF_PATH)/framework
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/drv/phys -I$(INC_BRCMSHARED_PUB_PATH)/drv/mdio
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/drv/unimac  -I$(INC_BRCMSHARED_PUB_PATH)/pmc 
EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource/drv/lport -I$(SHARED_DIR)/opensource/drv/lport/ag
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += $(INC_RDP_FLAGS)

-include $(TOPDIR)/Rules.make

