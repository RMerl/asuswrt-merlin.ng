BRCM_CHIP = $(patsubst "bcm%",%,$(CONFIG_SYS_SOC))
BCMBCA_PATH = $(srctree)/drivers/net/bcmbca

EXTRA_CFLAGS += -I$(BCMBCA_PATH)/phy
EXTRA_CFLAGS += -I$(BCMBCA_PATH)/lport
EXTRA_CFLAGS += -I$(BCMBCA_PATH)/lport/ag
EXTRA_CFLAGS += -I$(BCMBCA_PATH)/xrdp
EXTRA_CFLAGS += -I$(srctree)/include/dt-bindings/leds

EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP) -D_BYTE_ORDER_LITTLE_ENDIAN_

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6846_6856_6878_),)
    DRV_OBJS += unimac_drv_impl1.o
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63146_4912_6813_6855_),)
    DRV_OBJS += unimac_drv_impl2.o
endif

#ifeq ($(strip ${BRCM_CHIP}),4908)
#    DRV_OBJS += mdio_drv_sf2.o
#    DRV_OBJS += dt_access.o
#    DRV_OBJS += mdio_drv_common.o
#endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_6878_63178_47622_63146_4912_6813_6855_6756_),)
    DRV_OBJS += mac_drv.o
    DRV_OBJS += bus_drv.o
    DRV_OBJS += phy_drv.o
    DRV_OBJS += phy_drv_mii.o
    DRV_OBJS += phy_drv_brcm.o
    DRV_OBJS += dt_access.o
    DRV_OBJS += dt_parsing.o
ifeq ($(strip ${BRCM_CHIP}),6858)
    DRV_OBJS += bus_drv_6858.o
    DRV_OBJS += mac_drv_lport.o
    DRV_OBJS += phy_drv_6858_egphy.o
    DRV_OBJS += phy_drv_ext1.o
    DRV_OBJS += phy_drv_ext2.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += phy_drv_lport_serdes.o
    EXTRA_CFLAGS += -DMAC_LPORT
    EXTRA_CFLAGS += -DPHY_6858_EGPHY -DPHY_EXT1 -DPHY_EXT2 -DPHY_EXT3 -DPHY_LPORT_SERDES
    EXTRA_CFLAGS += -DLPORT_SERDES_POWER_SAVING -DLPORT_SERDES_SPEED_DETECT -DLPORT_SERDES_EXTERNAL_SIGNAL_DETECT
endif
ifeq ($(strip ${BRCM_CHIP}),6856)
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_impl5.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += phy_drv_6846_egphy.o
    DRV_OBJS += phy_drv_6856_sgmii.o
    DRV_OBJS += phy_drv_sgmii_plus2.o
    DRV_OBJS += phy_drv_ext1.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += brcm_rgmii.o
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_6846_EGPHY -DPHY_6856_SGMII -DPHY_EXT1 -DPHY_EXT3 
endif
ifeq ($(strip ${BRCM_CHIP}),6855)
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_impl5.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += phy_drv_6846_egphy.o
    DRV_OBJS += phy_drv_6856_sgmii.o
    DRV_OBJS += phy_drv_sgmii_plus2.o
    DRV_OBJS += phy_drv_ext1.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += brcm_rgmii.o
    DRV_OBJS += xrdp_led_init.o
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_6846_EGPHY -DPHY_6856_SGMII -DPHY_EXT1 -DPHY_EXT3
    EXTRA_CFLAGS += -DBRCM_64B_UMAC_MIB 
    EXTRA_CFLAGS += -DXRDP_LED
endif
ifeq ($(strip ${BRCM_CHIP}),6846)
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
    EXTRA_CFLAGS += -DPHY_6846_EGPHY -DPHY_EXT1 -DXRDP_LED
endif
ifeq ($(strip ${BRCM_CHIP}),47622)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mac_drv_sf2.o
    DRV_OBJS += ephy_led_init.o
    DRV_OBJS += brcm_rgmii.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_138class_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_ext3.o
    EXTRA_CFLAGS += -DBRCM_RGMII -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_SF2_EXTERNAL -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_M2M -DEPHY_LED
    EXTRA_CFLAGS += -DPHY_138CLASS_SERDES -DPHY_EXT3
endif
ifeq ($(strip ${BRCM_CHIP}),63178)
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mac_drv_sf2.o
    DRV_OBJS += ephy_led_init.o
    DRV_OBJS += brcm_rgmii.o
    DRV_OBJS += phy_drv_dsl_phy.o
    EXTRA_CFLAGS += -DBRCM_RGMII -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_SF2
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_M2M -DEPHY_LED
endif
ifeq ($(strip ${BRCM_CHIP}),6756)
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mac_drv_sf2.o
    DRV_OBJS += ephy_led_init.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += brcm_rgmii.o
    DRV_OBJS += Serdes6756Class/merlin28_shortfin_config.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_6756class_serdes.o
    EXTRA_CFLAGS += -DBRCM_RGMII -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_SF2_DUAL
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_M2M -DPHY_EXT3 -DEPHY_LED
    EXTRA_CFLAGS += -DPHY_6756CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes6756Class -DEXCLUDE_STD_HEADERS
endif
ifeq ($(strip ${BRCM_CHIP}),63146)
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_impl5.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_config.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_field_access.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_internal_error.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_debug_functions.o
    DRV_OBJS += phy_drv_146class_serdes.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += ephy_led_init.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += brcm_rgmii.o
    EXTRA_CFLAGS += -DBRCM_RGMII -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_EXT3 -DEPHY_LED
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
endif
ifeq ($(strip ${BRCM_CHIP}),4912)
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_impl5.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += mac_drv_xport.o
    DRV_OBJS += ../xport/xport_drv.o
    DRV_OBJS += ../xport/xport_stats.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_config.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_field_access.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_internal_error.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_debug_functions.o
    DRV_OBJS += phy_drv_146class_serdes.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += ephy_led_init.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += brcm_rgmii.o
    EXTRA_CFLAGS += -DBRCM_RGMII -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_UNIMAC -DMAC_XPORT
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_EXT3 -DEPHY_LED
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
    DRV_OBJS += $(patsubst %.c, ../%.o, $(shell cd $(BCMBCA_PATH) && find xport/ag/4912A0  -type f -name '*.c')) 
    EXTRA_CFLAGS += -I$(BCMBCA_PATH)/xport -I$(BCMBCA_PATH)/xport/ag/4912A0
endif
ifeq ($(strip ${BRCM_CHIP}),6813)
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_impl5.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += mac_drv_xport.o
    DRV_OBJS += ../xport/xport_drv.o
    DRV_OBJS += ../xport/xport_stats.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_config.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_field_access.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_internal_error.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_debug_functions.o
    DRV_OBJS += phy_drv_146class_serdes.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += ephy_led_init.o
    DRV_OBJS += phy_drv_ext3.o
    EXTRA_CFLAGS += -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_UNIMAC -DMAC_XPORT
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_EXT3 -DEPHY_LED
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
    DRV_OBJS += $(patsubst %.c, ../%.o, $(shell cd $(BCMBCA_PATH) && find xport/ag/4912A0  -type f -name '*.c')) 
    EXTRA_CFLAGS += -I$(BCMBCA_PATH)/xport -I$(BCMBCA_PATH)/xport/ag/4912A0
endif
endif

obj-y += DRV_OBJS

