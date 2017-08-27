ifeq ($(strip ${BRCM_CHIP}),6858)
    DRV_OBJS += phys/mac_drv_lport.o
    DRV_OBJS += lport/lport_drv.o
    DRV_OBJS += lport/lport_mdio.o
    DRV_OBJS += lport/lport_stats.o
    DRV_OBJS += lport/serdes_access.o
    DRV_OBJS += lport/lport_intr.o
    DRV_OBJS += lport/lport_proc.o
    DRV_OBJS += $(patsubst %.c, %.o, $(shell cd $(src) && find lport/ag  -type f -name '*.c')) 
ifeq ($(strip ${CONFIG_MERLIN_SDK}),)
    DRV_OBJS += lport/merlin_direct_access.o
endif
else
ifneq ($(strip ${BRCM_CHIP}),47189)
    DRV_OBJS += unimac/unimac_drv_impl1.o
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_6858_),)
ifeq ($(strip ${CONFIG_BCM_ENET_IMPL}),5)
    DRV_OBJS += phys/phys_common_drv.o
endif
    DRV_OBJS += phys/mac_drv.o
    DRV_OBJS += phys/bus_drv.o
    DRV_OBJS += phys/phy_drv.o
    DRV_OBJS += phys/phy_drv_mii.o
    DRV_OBJS += phys/phy_drv_brcm.o
    DRV_OBJS += phys/phy_bp_parsing.o
ifeq ($(strip ${BRCM_CHIP}),6838)
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
    DRV_OBJS += mdio/mdio_drv_impl2.o
    DRV_OBJS += phys/bus_drv_6848.o
    DRV_OBJS += phys/mac_drv_unimac.o
    DRV_OBJS += phys/phy_bp_parsing_6848.o
    DRV_OBJS += phys/phy_drv_6848_egphy.o
    DRV_OBJS += phys/phy_drv_6848_ephy.o
    DRV_OBJS += phys/phy_drv_6848_sgmii.o
    DRV_OBJS += phys/phy_drv_pcs.o
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_6848_EGPHY -DPHY_6848_EPHY -DPHY_6848_SGMII -DPHY_PCS
endif
ifeq ($(strip ${BRCM_CHIP}),6858)
    DRV_OBJS += phys/bus_drv_6858.o
    DRV_OBJS += phys/phy_bp_parsing_6858.o
    DRV_OBJS += phys/phy_drv_lport.o
    DRV_OBJS += phys/phy_drv_6858_egphy.o
    DRV_OBJS += phys/phy_drv_ext2.o
    DRV_OBJS += phys/BCM8486-A0-v00-00-12-TCM.o
    DRV_OBJS += phys/phy_drv_ext3.o
    DRV_OBJS += phys/phy_drv_lport_serdes.o
    EXTRA_CFLAGS += -DMAC_LPORT
    EXTRA_CFLAGS += -DPHY_6858_EGPHY -DPHY_EXT2 -DPHY_EXT3 -DPHY_LPORT_SERDES
endif
endif

ifeq ($(strip ${BRCM_CHIP}),47189)
    DRV_OBJS += phys/bus_drv.o
    DRV_OBJS += phys/mac_drv.o
    DRV_OBJS += phys/phy_drv.o
    DRV_OBJS += phys/bus_drv_47189.o
    DRV_OBJS += mdio/mdio_drv_impl3.o
    DRV_OBJS += phys/mac_drv_gmac.o
    DRV_OBJS += phys/phy_bp_parsing.o
    DRV_OBJS += phys/phy_bp_parsing_47189.o
    DRV_OBJS += phys/phy_drv_mii.o
    DRV_OBJS += phys/phy_drv_brcm.o
    DRV_OBJS += phys/phy_drv_53125.o
    DRV_OBJS += phys/phy_drv_ext1.o
    EXTRA_CFLAGS += -DMAC_GMAC
    EXTRA_CFLAGS += -DPHY_EXT_SWITCH
    EXTRA_CFLAGS += -DPHY_EXT1
    EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/bcm963xx
endif
obj-y +=  $(DRV_OBJS)

EXTRA_CFLAGS += -I$(INC_BDMF_PATH)/framework
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/drv/phys -I$(INC_BRCMSHARED_PUB_PATH)/drv/mdio
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/drv/unimac  -I$(INC_BRCMSHARED_PUB_PATH)/pmc 
EXTRA_CFLAGS += -I$(SHARED_DIR)/opensource/drv/lport -I$(SHARED_DIR)/opensource/drv/lport/ag
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += $(INC_RDP_FLAGS)



-include $(TOPDIR)/Rules.make

