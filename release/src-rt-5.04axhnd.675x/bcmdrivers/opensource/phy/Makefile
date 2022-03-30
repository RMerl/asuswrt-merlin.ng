CUR_DIR := $(BRCMDRIVERS_DIR)/opensource/phy
DRV_DIR := ../../../shared/opensource/drv

ifeq ($(strip ${BRCM_CHIP}),6858)
    EXTRA_CFLAGS += -I$(CUR_DIR)/lport -I$(CUR_DIR)/lport/ag
    DRV_OBJS += lport/lport_drv.o
    DRV_OBJS += lport/lport_mdio.o
    DRV_OBJS += lport/lport_stats.o
    DRV_OBJS += lport/lport_intr.o
    DRV_OBJS += lport/lport_proc.o
    DRV_OBJS += lport/serdes_access.o
    DRV_OBJS += lport/merlin_direct_access.o
    DRV_OBJS += lport/ag/BCM6858_A0_LPORT_AG.o
    DRV_OBJS += lport/ag/BCM6858_A0LPORT_CTRL_AG.o
    DRV_OBJS += lport/ag/BCM6858_A0LPORT_INTR_AG.o
    DRV_OBJS += lport/ag/BCM6858_A0LPORT_LED_AG.o
    DRV_OBJS += lport/ag/BCM6858_A0LPORT_MAB_AG.o
    DRV_OBJS += lport/ag/BCM6858_A0LPORT_MDIO_AG.o
    DRV_OBJS += lport/ag/BCM6858_A0LPORT_MIB_AG.o
    DRV_OBJS += lport/ag/BCM6858_A0LPORT_RGMII_AG.o
    DRV_OBJS += lport/ag/BCM6858_A0LPORT_SRDS_AG.o
    DRV_OBJS += lport/ag/BCM6858_A0LPORT_XLMAC_AG.o
    DRV_OBJS += lport/ag/BCM6858_A0MIB_CONF_AG.o
    DRV_OBJS += lport/ag/BCM6858_A0XLMAC_CONF_AG.o
    DRV_OBJS += lport/ag/bcm6858_drivers_lport_ag.o
    DRV_OBJS += lport/ag/bcm6858_lport_ctrl_ag.o
    DRV_OBJS += lport/ag/bcm6858_lport_intr_ag.o
    DRV_OBJS += lport/ag/bcm6858_lport_led_ag.o
    DRV_OBJS += lport/ag/bcm6858_lport_mab_ag.o
    DRV_OBJS += lport/ag/bcm6858_lport_mdio_ag.o
    DRV_OBJS += lport/ag/bcm6858_lport_mib_ag.o
    DRV_OBJS += lport/ag/bcm6858_lport_rgmii_ag.o
    DRV_OBJS += lport/ag/bcm6858_lport_srds_ag.o
    DRV_OBJS += lport/ag/bcm6858_lport_xlmac_ag.o
    DRV_OBJS += lport/ag/bcm6858_mib_conf_ag.o
    DRV_OBJS += lport/ag/bcm6858_xlmac_conf_ag.o
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_6878_6855_),)
    DRV_OBJS += mac_drv.o
    DRV_OBJS += bus_drv.o
    DRV_OBJS += phy_drv.o
    DRV_OBJS += phy_drv_mii.o
    DRV_OBJS += phy_drv_brcm.o
    DRV_OBJS += dt_access.o
    DRV_OBJS += dt_parsing.o
    DRV_OBJS += phy_drv_crossbar.o
    EXTRA_CFLAGS += -DPHY_CROSSBAR

ifeq ($(strip ${BRCM_CHIP}),6858)
    DRV_OBJS += bus_drv_6858.o
    DRV_OBJS += mac_drv_lport.o
    DRV_OBJS += phy_drv_lport.o
    DRV_OBJS += phy_drv_6858_egphy.o
    DRV_OBJS += phy_drv_ext1.o
    DRV_OBJS += phy_drv_ext2.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += phy_mac_sec.o
    DRV_OBJS += phy_mac_sec_list.o
    DRV_OBJS += sha256.o
    DRV_OBJS += phy_drv_lport_serdes.o
    EXTRA_CFLAGS += -DMAC_LPORT
    EXTRA_CFLAGS += -DPHY_6858_EGPHY -DPHY_EXT1 -DPHY_EXT2 -DPHY_EXT3 -DPHY_LPORT_SERDES
    EXTRA_CFLAGS += -DLPORT_INTERRUPTS -DMACSEC_SUPPORT
    EXTRA_CFLAGS += -DLPORT_SERDES_POWER_SAVING
ifeq ($(strip $(CONFIG_BCM_FTTDP_G9991)),)
    EXTRA_CFLAGS += -DLPORT_SERDES_LINK_POWER_DOWN -DLPORT_SERDES_SPEED_DETECT -DLPORT_SERDES_EXTERNAL_SIGNAL_DETECT
    EXTRA_CFLAGS += -DLPORT_REMOTE_FAULT_HANDLER
endif
endif

ifeq ($(strip ${BRCM_CHIP}),6856)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_impl5.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += phy_drv_6846_egphy.o
    DRV_OBJS += phy_drv_6856_sgmii.o
    DRV_OBJS += phy_drv_sgmii_plus2.o
    DRV_OBJS += phy_drv_ext1.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += phy_mac_sec.o
    DRV_OBJS += phy_mac_sec_list.o
    DRV_OBJS += sha256.o
    DRV_OBJS += brcm_rgmii.o
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_6846_EGPHY -DPHY_6856_SGMII -DPHY_EXT1 -DPHY_EXT3 
endif

ifeq ($(strip ${BRCM_CHIP}),6855)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_impl5.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += phy_drv_6846_egphy.o
    DRV_OBJS += phy_drv_6856_sgmii.o
    DRV_OBJS += xrdp_led_init.o
    DRV_OBJS += phy_drv_sgmii_plus2.o
    DRV_OBJS += phy_drv_ext1.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += phy_mac_sec.o
    DRV_OBJS += phy_mac_sec_list.o
    DRV_OBJS += sha256.o   
    DRV_OBJS += brcm_rgmii.o
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_6846_EGPHY -DPHY_6856_SGMII -DPHY_EXT1 -DPHY_EXT3 -DMACSEC_SUPPORT
    EXTRA_CFLAGS += -DBRCM_64B_UMAC_MIB
    EXTRA_CFLAGS += -DXRDP_LED
endif


ifeq ($(strip ${BRCM_CHIP}),6846)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_impl5.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += phy_drv_6846_egphy.o
    DRV_OBJS += phy_drv_ext1.o
    DRV_OBJS += brcm_rgmii.o
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_6846_EGPHY -DPHY_EXT1
endif

ifeq ($(strip ${BRCM_CHIP}),6878)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_impl5.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += phy_drv_6846_egphy.o
    DRV_OBJS += xrdp_led_init.o
    DRV_OBJS += phy_drv_ext1.o
    DRV_OBJS += brcm_rgmii.o
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_6846_EGPHY -DPHY_EXT1
	EXTRA_CFLAGS += -DXRDP_LED
endif

endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_4908_63158_63178_47622_63146_4912_6813_6756_),)
    DRV_OBJS += mac_drv.o
    DRV_OBJS += bus_drv.o
    DRV_OBJS += phy_drv.o
    DRV_OBJS += phy_drv_mii.o
    DRV_OBJS += phy_drv_brcm.o
    DRV_OBJS += dt_access.o
    DRV_OBJS += dt_parsing.o
    DRV_OBJS += phy_drv_crossbar.o
    DRV_OBJS += brcm_rgmii.o
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DPHY_CROSSBAR

ifeq ($(strip ${BRCM_CHIP}),63138)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += mac_drv_sf2.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_138class_serdes.o
    DRV_OBJS += phy_drv_dsl_i2c.o
    DRV_OBJS += ephy_led_init_impl2.o
    EXTRA_CFLAGS += -DMAC_UNIMAC -DMAC_SF2 -DPHY_DSL_GPHY -DPHY_M2M -DDSL_DEVICES
    EXTRA_CFLAGS += -DPHY_138CLASS_SERDES -DPHY_138CLASS_SERDES_NO_2P5G
    EXTRA_CFLAGS += -DCRB_5X3_QGPHY3_WORKAROUND
endif

ifeq ($(strip ${BRCM_CHIP}),63148)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += mac_drv_sf2.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_138class_serdes.o
    DRV_OBJS += phy_drv_dsl_i2c.o
    DRV_OBJS += ephy_led_init_impl2.o
    EXTRA_CFLAGS += -DMAC_UNIMAC -DMAC_SF2 -DPHY_DSL_GPHY -DPHY_M2M -DDSL_DEVICES
    EXTRA_CFLAGS += -DPHY_138CLASS_SERDES -DPHY_138CLASS_SERDES_NO_2P5G
endif

ifeq ($(strip ${BRCM_CHIP}),4908)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += mac_drv_sf2.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_138class_serdes.o
    DRV_OBJS += phy_drv_dsl_i2c.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += phy_mac_sec.o
    DRV_OBJS += phy_mac_sec_list.o
    DRV_OBJS += sha256.o
    DRV_OBJS += ephy_led_init.o
    EXTRA_CFLAGS += -DMAC_UNIMAC -DMAC_SF2 -DPHY_DSL_GPHY -DPHY_138CLASS_SERDES -DPHY_M2M -DDSL_DEVICES -DSERDES_OPT_CHECK
    EXTRA_CFLAGS += -DPHY_EXT3 -DMACSEC_SUPPORT
endif

ifeq ($(strip ${BRCM_CHIP}),63158)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += mac_drv_sf2.o
    DRV_OBJS += mac_drv_xport.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_158class_serdes.o
    DRV_OBJS += phy_drv_138class_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_dsl_i2c.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += xport/xport_drv.o
    DRV_OBJS += xport/xport_stats.o
    DRV_OBJS += xport/xport_proc.o
    DRV_OBJS += phy_mac_sec.o
    DRV_OBJS += phy_mac_sec_list.o
    DRV_OBJS += sha256.o
    DRV_OBJS += ephy_led_init.o

    EXTRA_CFLAGS += -DMAC_UNIMAC -DMAC_SF2 -DPHY_DSL_GPHY -DPHY_M2M -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_XPORT
    EXTRA_CFLAGS += -DPHY_158CLASS_SERDES -DPHY_138CLASS_SERDES -DPHY_EXT3 -DMACSEC_SUPPORT
    DRV_OBJS += $(patsubst %.c, %.o, $(shell cd $(src) && find xport/ag/63158B0  -type f -name '*.c')) 
    EXTRA_CFLAGS += -I$(CUR_DIR)/xport -I$(CUR_DIR)/xport/ag/63158B0
endif

ifeq ($(strip ${BRCM_CHIP}),63178)
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += mac_drv_sf2.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += ephy_led_init.o

    EXTRA_CFLAGS += -DMAC_SF2 -DPHY_DSL_GPHY -DPHY_M2M -DARCHER_DEVICE -DDSL_DEVICES
    EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx
endif

ifeq ($(strip ${BRCM_CHIP}),47622)
    DRV_OBJS += unimac_drv_impl1.o

    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += mac_drv_sf2.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_138class_serdes.o
    DRV_OBJS += phy_drv_dsl_i2c.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_ext3.o
ifneq ($(findstring $(strip ${MODEL}),RTAX95Q RTAXE95Q),)
    DRV_OBJS += phys/nic_rtl8226b_init.o
    DRV_OBJS += phys/nic_rtl8226.o
    DRV_OBJS += phys/rtl8226.o
endif
    DRV_OBJS += phy_mac_sec.o
    DRV_OBJS += phy_mac_sec_list.o
    DRV_OBJS += sha256.o
    DRV_OBJS += ephy_led_init.o

    EXTRA_CFLAGS += -DMAC_UNIMAC -DMAC_SF2_EXTERNAL -DPHY_DSL_GPHY -DPHY_138CLASS_SERDES -DSERDES_OPT_CHECK
    EXTRA_CFLAGS += -DPHY_EXT3 -DPHY_M2M -DARCHER_DEVICE -DDSL_DEVICES -DMACSEC_SUPPORT

    EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx
endif

ifeq ($(strip ${BRCM_CHIP}),63146)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_config.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_access.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_field_access.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_internal.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_internal_error.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_debug_functions.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += phy_drv_146class_serdes.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_dsl_i2c.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += ephy_led_init.o

    EXTRA_CFLAGS += -DMAC_UNIMAC -DPHY_DSL_GPHY -DPHY_M2M -DDSL_DEVICES
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(CUR_DIR)/Serdes146Class -DEXCLUDE_STD_HEADERS
    EXTRA_CFLAGS += -DPHY_EXT3
endif

ifeq ($(strip ${BRCM_CHIP}),4912)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += mac_drv_xport.o
    DRV_OBJS += xport/xport_drv.o
    DRV_OBJS += xport/xport_stats.o
    DRV_OBJS += xport/xport_proc.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_config.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_access.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_field_access.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_internal.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_internal_error.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_debug_functions.o
    DRV_OBJS += phy_drv_dsl_i2c.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_146class_serdes.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += $(if $(wildcard gpy211.c),gpy211.o,prebuilt/gpy211.o)
    DRV_OBJS += phy_mac_sec.o
    DRV_OBJS += phy_mac_sec_list.o
    DRV_OBJS += sha256.o
    DRV_OBJS += ephy_led_init.o

    EXTRA_CFLAGS += -DMAC_UNIMAC -DPHY_DSL_GPHY -DPHY_M2M -DDSL_DEVICES
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(CUR_DIR)/Serdes146Class -DEXCLUDE_STD_HEADERS
    EXTRA_CFLAGS += -DMAC_XPORT -DPHY_EXT3 -DMACSEC_SUPPORT
    DRV_OBJS += $(patsubst %.c, %.o, $(shell cd $(src) && find xport/ag/4912A0  -type f -name '*.c')) 
    EXTRA_CFLAGS += -I$(CUR_DIR)/xport -I$(CUR_DIR)/xport/ag/4912A0
endif

ifeq ($(strip ${BRCM_CHIP}),6813)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += mac_drv_xport.o
    DRV_OBJS += xport/xport_drv.o
    DRV_OBJS += xport/xport_stats.o
    DRV_OBJS += xport/xport_proc.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_config.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_access.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_field_access.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_internal.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_internal_error.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_debug_functions.o
    DRV_OBJS += phy_drv_dsl_i2c.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_146class_serdes.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += ephy_led_init.o

    EXTRA_CFLAGS += -DMAC_UNIMAC -DPHY_DSL_GPHY -DPHY_M2M -DDSL_DEVICES
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(CUR_DIR)/Serdes146Class -DEXCLUDE_STD_HEADERS
    EXTRA_CFLAGS += -DMAC_XPORT -DPHY_EXT3
    DRV_OBJS += $(patsubst %.c, %.o, $(shell cd $(src) && find xport/ag/4912A0  -type f -name '*.c')) 
    EXTRA_CFLAGS += -I$(CUR_DIR)/xport -I$(CUR_DIR)/xport/ag/4912A0
endif

ifeq ($(strip ${BRCM_CHIP}),6756)
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += mac_drv_sf2.o
    DRV_OBJS += Serdes6756Class/merlin28_shortfin_config.o
#    DRV_OBJS += Serdes6756Class/merlin28_shortfin_access.o
#    DRV_OBJS += Serdes6756Class/merlin28_shortfin_field_access.o
#    DRV_OBJS += Serdes6756Class/merlin28_shortfin_internal.o
#    DRV_OBJS += Serdes6756Class/merlin28_shortfin_internal_error.o
#    DRV_OBJS += Serdes6756Class/merlin28_shortfin_debug_functions.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_dsl_i2c.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_6756class_serdes.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += $(if $(wildcard gpy211.c),gpy211.o,prebuilt/gpy211.o)
    DRV_OBJS += phy_mac_sec.o
    DRV_OBJS += phy_mac_sec_list.o
    DRV_OBJS += sha256.o
    DRV_OBJS += ephy_led_init.o

    EXTRA_CFLAGS += -DMAC_SF2_DUAL -DPHY_DSL_GPHY 
    EXTRA_CFLAGS += -DPHY_6756CLASS_SERDES -I$(CUR_DIR)/Serdes6756Class -DEXCLUDE_STD_HEADERS
    EXTRA_CFLAGS += -DPHY_EXT3 -DPHY_M2M -DARCHER_DEVICE -DDSL_DEVICES -DMACSEC_SUPPORT
    EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx
endif

endif



EXTRA_CFLAGS += -I$(CUR_DIR)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -DPHY_PROC_FS -DPHY_LINK_CHANGE_NOTIFY

ifeq (${BRCM_ENDIAN_FLAGS},-l)
EXTRA_CFLAGS += -D_BYTE_ORDER_LITTLE_ENDIAN_
endif

obj-y += $(DRV_OBJS)


ifneq ("$(CONFIG_BCM_SELECTIVE_PHYS)", "y")
    CONFIG_BCM_PHY_MAKO_A0=y
    CONFIG_BCM_PHY_ORCA_A0=y
    CONFIG_BCM_PHY_ORCA_B0=y
    CONFIG_BCM_PHY_BLACKFIN_A0=y
    CONFIG_BCM_PHY_BLACKFIN_B0=y
    CONFIG_BCM_PHY_SHORTFIN_B0=y
    CONFIG_BCM_PHY_LONGFIN_A0=y
    CONFIG_BCM_PHY_LONGFIN_B0=y
endif

ifeq ("$(CONFIG_BCM_PHY_MAKO_A0)", "y")
    EXTRA_CFLAGS += -DCONFIG_BCM_PHY_MAKO_A0
    PHY_FIRMWARE += firmware/mako_a0_firmware.bin
endif

ifeq ("$(CONFIG_BCM_PHY_ORCA_A0)", "y")
    EXTRA_CFLAGS += -DCONFIG_BCM_PHY_ORCA_A0
    PHY_FIRMWARE += firmware/orca_a0_firmware.bin
endif

ifeq ("$(CONFIG_BCM_PHY_ORCA_B0)", "y")
    EXTRA_CFLAGS += -DCONFIG_BCM_PHY_ORCA_B0
    PHY_FIRMWARE += firmware/orca_b0_firmware.bin
endif

ifeq ("$(CONFIG_BCM_PHY_BLACKFIN_A0)", "y")
    EXTRA_CFLAGS += -DCONFIG_BCM_PHY_BLACKFIN_A0
    PHY_FIRMWARE += firmware/blackfin_a0_firmware.bin
endif

ifeq ("$(CONFIG_BCM_PHY_BLACKFIN_B0)", "y")
    EXTRA_CFLAGS += -DCONFIG_BCM_PHY_BLACKFIN_B0
    PHY_FIRMWARE += firmware/blackfin_b0_firmware.bin
endif

ifeq ("$(CONFIG_BCM_PHY_SHORTFIN_B0)", "y")
    EXTRA_CFLAGS += -DCONFIG_BCM_PHY_SHORTFIN_B0
    PHY_FIRMWARE += firmware/shortfin_b0_firmware.bin
endif

ifneq ($(findstring _y_,_$(CONFIG_BCM_PHY_LONGFIN_A0)_$(CONFIG_BCM_PHY_LONGFIN_B0)_),)
    EXTRA_CFLAGS += -DCONFIG_BCM_PHY_LONGFIN_A0 -DCONFIG_BCM_PHY_LONGFIN_B0
    PHY_FIRMWARE += firmware/longfin_a0_firmware.bin
endif


-include $(TOPDIR)/Rules.make

INSTALL_DONE = $(src)/.done_install
INSTALL_PATH=$(INSTALL_DIR)/etc/fw
INSTALL_SRC = $(addprefix $(src)/, $(PHY_FIRMWARE))

clean-files := $(notdir $(INSTALL_DONE))

$(INSTALL_PATH):
	mkdir -p $(INSTALL_PATH)

$(INSTALL_DONE): $(INSTALL_SRC) | $(INSTALL_PATH)
	cp -f $^ $(INSTALL_PATH)
	$(Q)touch $@

$(obj)/phy_drv_ext3.o: $(INSTALL_DONE)
