BRCM_CHIP = $(patsubst "bcm%",%,$(CONFIG_SYS_SOC))
BCMBCA_PATH = $(srctree)/drivers/net/bcmbca
DRV_OBJS :=

EXTRA_CFLAGS += -I$(BCMBCA_PATH)/phy
EXTRA_CFLAGS += -I$(BCMBCA_PATH)/lport
EXTRA_CFLAGS += -I$(BCMBCA_PATH)/lport/ag
EXTRA_CFLAGS += -I$(BCMBCA_PATH)/xrdp
EXTRA_CFLAGS += -I$(srctree)/include/dt-bindings/leds

ifdef CONFIG_BCMBCA_PMC
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/pmc
endif

EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP) -D_BYTE_ORDER_LITTLE_ENDIAN_

# XPORT
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4912_6813_6888_68880_6837_),)
ifeq ($(strip $(BRCM_CHIP)),4912) 
    XPORT_CHIP=4912A0
endif
ifeq ($(strip $(BRCM_CHIP)),6813) 
    XPORT_CHIP=6813A0
endif
ifeq ($(strip $(BRCM_CHIP)),6888) 
    XPORT_CHIP=6888A0
endif
ifeq ($(strip $(BRCM_CHIP)),68880) 
    XPORT_CHIP=6888A0
endif
ifeq ($(strip $(BRCM_CHIP)),6837) 
    XPORT_CHIP=6837A0
endif
    DRV_OBJS += ../xport/xport_drv.o
    DRV_OBJS += ../xport/xport_stats.o
    DRV_OBJS += $(patsubst %.c, ../%.o, $(shell cd $(BCMBCA_PATH) && find xport/ag/$(XPORT_CHIP) -type f -name '*.c')) 
    EXTRA_CFLAGS += -I$(BCMBCA_PATH)/xport -I$(BCMBCA_PATH)/xport/ag/$(XPORT_CHIP)
endif

# Merlin mptwo
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6756_),)
    EXTRA_CFLAGS += -DEXCLUDE_STD_HEADERS -DUINT16_MAX=0xFFFF
    EXTRA_CFLAGS += -include linux/kernel.h -include linux/types.h -include string.h
    EXTRA_CFLAGS += -I$(BCMBCA_PATH)/phy/merlin_mptwo/include
    EXTRA_CFLAGS += -I$(BCMBCA_PATH)/phy/merlin_mptwo/src
    DRV_OBJS += merlin_mptwo/serdes_wrapper.o
endif

# Merlin shortfin
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6888_68880_6837_63146_4912_6813_6765_),)
ifdef CONFIG_BCMBCA_SERDES
    EXTRA_CFLAGS += -DEXCLUDE_STD_HEADERS -DUINT16_MAX=0xFFFF
    EXTRA_CFLAGS += -include linux/kernel.h -include linux/types.h -include string.h -include vsprintf.h
    EXTRA_CFLAGS += -I$(BCMBCA_PATH)/phy/merlin_shortfin/include
    DRV_OBJS += $(patsubst %.c, %.o, $(shell cd $(BCMBCA_PATH)/phy && find merlin_shortfin/src -type f -name '*.c')) 
    DRV_OBJS += merlin_shortfin/serdes_wrapper.o
endif
endif

# Merlin shasta
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6888_68880_6837_),)
    EXTRA_CFLAGS += -DEXCLUDE_STD_HEADERS -DUINT16_MAX=0xFFFF
    EXTRA_CFLAGS += -include linux/kernel.h -include linux/types.h -include string.h -include vsprintf.h
    EXTRA_CFLAGS += -I$(BCMBCA_PATH)/phy/merlin_shasta/include
    DRV_OBJS += $(patsubst %.c, %.o, $(shell cd $(BCMBCA_PATH)/phy && find merlin_shasta/src -type f -name '*.c')) 
    DRV_OBJS += merlin_shasta/serdes_wrapper.o
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_6878_6855_6888_68880_6837_4908_63178_47622_6756_63146_4912_6813_6765_),)
    DRV_OBJS += mac_drv.o
    DRV_OBJS += bus_drv.o
    DRV_OBJS += phy_drv.o
    DRV_OBJS += phy_drv_mii.o
    DRV_OBJS += phy_drv_brcm.o
    DRV_OBJS += dt_access.o
    DRV_OBJS += dt_parsing.o
ifeq ($(strip ${BRCM_CHIP}),6858)
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_v1.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_lport.o
    DRV_OBJS += phy_drv_egphy.o
    DRV_OBJS += phy_drv_ext1.o
    DRV_OBJS += phy_drv_ext2.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += serdes_access.o
    DRV_OBJS += phy_drv_mptwo.o
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DMAC_LPORT
    EXTRA_CFLAGS += -DPHY_EGPHY -DPHY_EXT1 -DPHY_EXT2 -DPHY_EXT3 -DPHY_MPTWO
    EXTRA_CFLAGS += -DSERDES_LINK_POWER_DOWN -DSERDES_SPEED_DETECT -DSERDES_MODULE_DETECT
endif
ifeq ($(strip ${BRCM_CHIP}),6856)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_v1.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += phy_drv_egphy.o
    DRV_OBJS += phy_drv_sgmii_plus2.o
    DRV_OBJS += phy_drv_ext1.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += brcm_rgmii.o
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_EGPHY -DPHY_SGMII -DPHY_EXT1 -DPHY_EXT3 
endif
ifeq ($(strip ${BRCM_CHIP}),6855)
    DRV_OBJS += unimac_drv_impl2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_v1.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += phy_drv_egphy.o
    DRV_OBJS += phy_drv_sgmii_plus2.o
    DRV_OBJS += phy_drv_ext1.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += brcm_rgmii.o
    DRV_OBJS += xrdp_led_init.o
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC -DBRCM_64B_UMAC_MIB
    EXTRA_CFLAGS += -DPHY_EGPHY -DPHY_SGMII -DPHY_EXT1 -DPHY_EXT3
    EXTRA_CFLAGS += -DXRDP_LED
endif
ifeq ($(strip ${BRCM_CHIP}),6846)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_v1.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += phy_drv_egphy.o
    DRV_OBJS += phy_drv_ext1.o
    DRV_OBJS += brcm_rgmii.o
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_EGPHY -DPHY_EXT1
endif
ifeq ($(strip ${BRCM_CHIP}),6878)
    DRV_OBJS += unimac_drv_impl1.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_v1.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += phy_drv_egphy.o
    DRV_OBJS += xrdp_led_init.o
    DRV_OBJS += phy_drv_ext1.o
    DRV_OBJS += brcm_rgmii.o
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_EGPHY -DPHY_EXT1 -DXRDP_LED
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6888_68880_6837_),)
    DRV_OBJS += unimac_drv_impl2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_v1.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += ephy_led_init.o
    DRV_OBJS += eth_phy_top.o
    DRV_OBJS += mac_drv_xport.o
    DRV_OBJS += phy_drv_egphy.o
    DRV_OBJS += serdes_access.o
    DRV_OBJS += phy_drv_shortfin.o
    DRV_OBJS += phy_drv_shasta.o
    DRV_OBJS += phy_drv_ext3.o
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DMAC_UNIMAC -DBRCM_64B_UMAC_MIB -DMAC_XPORT
    EXTRA_CFLAGS += -DPHY_EGPHY -DPHY_SHORTFIN -DPHY_SHASTA -DPHY_EXT3
    EXTRA_CFLAGS += -DSERDES_LINK_POWER_DOWN -DSERDES_MODULE_DETECT
    EXTRA_CFLAGS += -DEPHY_LED
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6888_68880_),)
    EXTRA_CFLAGS += -DPHY_XPHY
endif
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
ifeq ($(strip ${BRCM_CHIP}),4908)
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mac_drv_sf2.o
    DRV_OBJS += ephy_led_init.o
    DRV_OBJS += brcm_rgmii.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_138class_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += phy_drv_ext3.o
    EXTRA_CFLAGS += -DBRCM_RGMII -DPHY_EXT3 -DPHY_138CLASS_SERDES -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_SF2
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_M2M -DEPHY_LED
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
    DRV_OBJS += serdes_access.o
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
    DRV_OBJS += unimac_drv_impl2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_v1.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += ephy_led_init.o
    DRV_OBJS += brcm_rgmii.o
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DBRCM_RGMII -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_UNIMAC -DBRCM_64B_UMAC_MIB
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DEPHY_LED
ifdef CONFIG_BCMBCA_SERDES
    DRV_OBJS += serdes_access.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_config.o
    DRV_OBJS += phy_drv_146class_serdes.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_ext3.o
    EXTRA_CFLAGS += -DPHY_EXT3
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
endif
endif
ifeq ($(strip ${BRCM_CHIP}),4912)
    DRV_OBJS += unimac_drv_impl2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_v1.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += mac_drv_xport.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += ephy_led_init.o
    DRV_OBJS += brcm_rgmii.o
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DBRCM_RGMII -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_UNIMAC -DBRCM_64B_UMAC_MIB -DMAC_XPORT
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DEPHY_LED
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
ifdef CONFIG_BCMBCA_SERDES
    DRV_OBJS += serdes_access.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_config.o
    DRV_OBJS += phy_drv_146class_serdes.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_ext3.o
    EXTRA_CFLAGS += -DPHY_EXT3
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
endif
endif
ifeq ($(strip ${BRCM_CHIP}),6813)
    DRV_OBJS += unimac_drv_impl2.o
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_v1.o
    DRV_OBJS += bus_drv_mdio_v1.o
    DRV_OBJS += eth_phy_top.o
    DRV_OBJS += mac_drv_unimac.o
    DRV_OBJS += mac_drv_xport.o
    DRV_OBJS += phy_drv_dsl_phy.o
    DRV_OBJS += ephy_led_init.o
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_UNIMAC -DBRCM_64B_UMAC_MIB -DMAC_XPORT
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DEPHY_LED
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
ifdef CONFIG_BCMBCA_SERDES
    DRV_OBJS += serdes_access.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_config.o
    DRV_OBJS += phy_drv_146class_serdes.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_ext3.o
    EXTRA_CFLAGS += -DPHY_EXT3 -DPHY_XPHY
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
endif
endif
ifeq ($(strip ${BRCM_CHIP}),6765)
    DRV_OBJS += mdio_drv_common.o
    DRV_OBJS += mdio_drv_sf2.o
    DRV_OBJS += bus_drv_sf2.o
    DRV_OBJS += mac_drv_sf2.o
    DRV_OBJS += ephy_led_init.o
    DRV_OBJS += phy_drv_ext3.o
    DRV_OBJS += eth_phy_top.o
    DRV_OBJS += serdes_access.o
    DRV_OBJS += Serdes146Class/merlin16_shortfin_config.o
    DRV_OBJS += phy_drv_146class_serdes.o
    DRV_OBJS += phy_drv_dsl_serdes.o
    DRV_OBJS += phy_drv_dsl_848xx.o
    DRV_OBJS += phy_drv_dsl_phy.o
    EXTRA_CFLAGS += -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_SF2_DUAL -DRESCAL_BY_XPHY -DPHY_SHARED_REF_CLK
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_M2M -DPHY_EXT3 -DPHY_XPHY -DEPHY_LED
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63158_4908_),)
    DRV_OBJS += ephy_led_init.o
    DRV_OBJS += dt_access.o
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_),)
    DRV_OBJS += ephy_led_init_impl2.o
    DRV_OBJS += dt_access.o
endif


obj-y += DRV_OBJS

