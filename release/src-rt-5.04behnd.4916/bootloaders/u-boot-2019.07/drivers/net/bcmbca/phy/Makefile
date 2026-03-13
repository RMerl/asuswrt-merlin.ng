PROFILE  = $(shell cat ../../../.last_profile)
ifneq ($(findstring BUILD_HND_MFG=, $(shell cat ../../../targets/$(PROFILE)/$(PROFILE))),)
EXTRA_CFLAGS += -DHND_MFG
endif

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
    XPORT_AG_MODULES := bcm6813_drivers_xport_ag \
                     bcm6813_xport_mib_reg_ag \
                     bcm6813_xport_wol_ard_ag \
                     bcm6813_xport_xlmac_reg_ag \
                     bcm6813_xport_mab_ag \
                     bcm6813_xport_portreset_ag \
                     bcm6813_xport_wol_mpd_ag \
                     bcm6813_xport_mib_core_ag \
                     bcm6813_xport_top_ag \
                     bcm6813_xport_xlmac_core_ag \
                     XPORT_MAB_AG \
                     XPORT_PORTRESET_AG \
                     XPORT_WOL_MPD_AG \
                     XPORT_AG \
                     XPORT_MIB_CORE_AG \
                     XPORT_TOP_AG \
                     XPORT_XLMAC_CORE_AG \
                     XPORT_INTR_AG \
                     XPORT_MIB_REG_AG \
                     XPORT_WOL_ARD_AG \
                     XPORT_XLMAC_REG_AG
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
    DRV_OBJS += $(if $(wildcard $(srctree)/drivers/net/bcmbca/xport/xport_drv.c),../xport/xport_drv.o,../xport/xport_drv_preb.o)
    DRV_OBJS += $(if $(wildcard $(srctree)/drivers/net/bcmbca/xport/xport_stats.c),../xport/xport_stats.o,../xport/xport_stats_preb.o)
    XPORT_AG_SRC := $(addprefix $(srctree)/drivers/net/bcmbca/xport/ag/$(XPORT_CHIP)/, $(addsuffix .c,$(XPORT_AG_MODULES)))
    XPORT_AG_OBJS := $(wildcard $(XPORT_AG_SRC))
    XPORT_AG_PREB_OBJS := $(filter-out $(XPORT_AG_OBJS),$(XPORT_AG_SRC))
    DRV_OBJS += $(patsubst $(srctree)/drivers/net/bcmbca/xport/ag/$(XPORT_CHIP)/%.c, ../xport/ag/$(XPORT_CHIP)/%.o, $(XPORT_AG_OBJS))
    DRV_OBJS += $(patsubst $(srctree)/drivers/net/bcmbca/xport/ag/$(XPORT_CHIP)/%.c, ../xport/ag/$(XPORT_CHIP)/%_preb.o, $(XPORT_AG_PREB_OBJS))
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
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6888_68880_6837_63146_4912_6813_6765_6766_6764_),)
ifdef CONFIG_BCMBCA_SERDES
    EXTRA_CFLAGS += -DEXCLUDE_STD_HEADERS -DUINT16_MAX=0xFFFF
    EXTRA_CFLAGS += -include linux/kernel.h -include linux/types.h -include string.h -include vsprintf.h
    EXTRA_CFLAGS += -I$(BCMBCA_PATH)/phy/merlin_shortfin/include
    MERLIN_SHORTFIN_MODULES := merlin16_pll_config \
                               merlin16_shortfin_access \
                               merlin16_shortfin_config \
                               merlin16_shortfin_debug_functions \
                               merlin16_shortfin_diag \
                               merlin16_shortfin_field_access \
                               merlin16_shortfin_internal \
                               merlin16_shortfin_internal_error \
                               merlin16_shortfin_prbs \
                               merlin16_shortfin_pwr_mgt \
                               merlin16_shortfin_reg_dump \
                               merlin16_tx_analog_functions
    MERLIN_SHORTFIN_SRC := $(addprefix $(srctree)/drivers/net/bcmbca/phy/merlin_shortfin/src/, $(addsuffix .c,$(MERLIN_SHORTFIN_MODULES)))
    MERLIN_SHORTFIN_OBJS := $(wildcard $(MERLIN_SHORTFIN_SRC))
    MERLIN_SHORTFIN_PREB_OBJS := $(filter-out $(MERLIN_SHORTFIN_OBJS),$(MERLIN_SHORTFIN_SRC))
    DRV_OBJS += $(patsubst $(srctree)/drivers/net/bcmbca/phy/merlin_shortfin/src/%.c, merlin_shortfin/src/%.o, $(MERLIN_SHORTFIN_OBJS))
    DRV_OBJS += $(patsubst $(srctree)/drivers/net/bcmbca/phy/merlin_shortfin/src/%.c, merlin_shortfin/src/%_preb.o, $(MERLIN_SHORTFIN_PREB_OBJS))
    DRV_OBJS += $(if $(wildcard $(srctree)/drivers/net/bcmbca/phy/merlin_shortfin/serdes_wrapper.c),merlin_shortfin/serdes_wrapper.o,merlin_shortfin/serdes_wrapper_preb.o)
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

# PHY
PHY_MODULES :=
PHY_SRC :=
PHY_OBJS :=
PHY_PREB_OBJS :=
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_6878_6855_6888_68880_6837_4908_63178_47622_6756_63146_4912_6813_6765_6766_6764_),)
    PHY_MODULES += mac_drv
    PHY_MODULES += bus_drv
    PHY_MODULES += phy_drv
    PHY_MODULES += phy_drv_mii
    PHY_MODULES += phy_drv_brcm
    PHY_MODULES += dt_access
    PHY_MODULES += dt_parsing
ifeq ($(strip ${BRCM_CHIP}),6858)
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_v1
    PHY_MODULES += bus_drv_mdio_v1
    PHY_MODULES += mac_drv_lport
    PHY_MODULES += phy_drv_egphy
    PHY_MODULES += phy_drv_ext1
    PHY_MODULES += phy_drv_ext2
    PHY_MODULES += phy_drv_ext3
    PHY_MODULES += serdes_access
    PHY_MODULES += phy_drv_mptwo
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DMAC_LPORT
    EXTRA_CFLAGS += -DPHY_EGPHY -DPHY_EXT1 -DPHY_EXT2 -DPHY_EXT3 -DPHY_MPTWO
    EXTRA_CFLAGS += -DSERDES_LINK_POWER_DOWN -DSERDES_SPEED_DETECT -DSERDES_MODULE_DETECT
endif
ifeq ($(strip ${BRCM_CHIP}),6856)
    PHY_MODULES += unimac_drv_impl1
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_v1
    PHY_MODULES += bus_drv_mdio_v1
    PHY_MODULES += mac_drv_unimac
    PHY_MODULES += phy_drv_egphy
    PHY_MODULES += phy_drv_sgmii_plus2
    PHY_MODULES += phy_drv_ext1
    PHY_MODULES += phy_drv_ext3
    PHY_MODULES += brcm_rgmii
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_EGPHY -DPHY_SGMII -DPHY_EXT1 -DPHY_EXT3 
endif
ifeq ($(strip ${BRCM_CHIP}),6855)
    PHY_MODULES += unimac_drv_impl2
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_v1
    PHY_MODULES += bus_drv_mdio_v1
    PHY_MODULES += mac_drv_unimac
    PHY_MODULES += phy_drv_egphy
    PHY_MODULES += phy_drv_sgmii_plus2
    PHY_MODULES += phy_drv_ext1
    PHY_MODULES += phy_drv_ext3
    PHY_MODULES += brcm_rgmii
    PHY_MODULES += xrdp_led_init
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC -DBRCM_64B_UMAC_MIB
    EXTRA_CFLAGS += -DPHY_EGPHY -DPHY_SGMII -DPHY_EXT1 -DPHY_EXT3
    EXTRA_CFLAGS += -DXRDP_LED
endif
ifeq ($(strip ${BRCM_CHIP}),6846)
    PHY_MODULES += unimac_drv_impl1
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_v1
    PHY_MODULES += bus_drv_mdio_v1
    PHY_MODULES += mac_drv_unimac
    PHY_MODULES += phy_drv_egphy
    PHY_MODULES += phy_drv_ext1
    PHY_MODULES += brcm_rgmii
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_EGPHY -DPHY_EXT1
endif
ifeq ($(strip ${BRCM_CHIP}),6878)
    PHY_MODULES += unimac_drv_impl1
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_v1
    PHY_MODULES += bus_drv_mdio_v1
    PHY_MODULES += mac_drv_unimac
    PHY_MODULES += phy_drv_egphy
    PHY_MODULES += xrdp_led_init
    PHY_MODULES += phy_drv_ext1
    PHY_MODULES += brcm_rgmii
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DBRCM_RGMII
    EXTRA_CFLAGS += -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_EGPHY -DPHY_EXT1 -DXRDP_LED
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6888_68880_6837_),)
    PHY_MODULES += unimac_drv_impl2
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_v1
    PHY_MODULES += bus_drv_mdio_v1
    PHY_MODULES += mac_drv_unimac
    PHY_MODULES += ephy_led_init
    PHY_MODULES += eth_phy_top
    PHY_MODULES += mac_drv_xport
    PHY_MODULES += phy_drv_egphy
    PHY_MODULES += serdes_access
    PHY_MODULES += phy_drv_shortfin
    PHY_MODULES += phy_drv_shasta
    PHY_MODULES += phy_drv_ext3
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
    PHY_MODULES += unimac_drv_impl1
    PHY_MODULES += mac_drv_unimac
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_sf2
    PHY_MODULES += bus_drv_sf2
    PHY_MODULES += mac_drv_sf2
    PHY_MODULES += ephy_led_init
    PHY_MODULES += brcm_rgmii
    PHY_MODULES += phy_drv_dsl_phy
    PHY_MODULES += phy_drv_dsl_serdes
    PHY_MODULES += phy_drv_138class_serdes
    PHY_MODULES += phy_drv_dsl_848xx
    PHY_MODULES += phy_drv_ext3
    EXTRA_CFLAGS += -DBRCM_RGMII -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_SF2_EXTERNAL -DMAC_UNIMAC
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_M2M -DEPHY_LED
    EXTRA_CFLAGS += -DPHY_138CLASS_SERDES -DPHY_EXT3
endif
ifeq ($(strip ${BRCM_CHIP}),4908)
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_sf2
    PHY_MODULES += bus_drv_sf2
    PHY_MODULES += mac_drv_sf2
    PHY_MODULES += ephy_led_init
    PHY_MODULES += brcm_rgmii
    PHY_MODULES += phy_drv_dsl_serdes
    PHY_MODULES += phy_drv_138class_serdes
    PHY_MODULES += phy_drv_dsl_848xx
    PHY_MODULES += phy_drv_dsl_phy
    PHY_MODULES += phy_drv_ext3
    EXTRA_CFLAGS += -DBRCM_RGMII -DPHY_EXT3 -DPHY_138CLASS_SERDES -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_SF2
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_M2M -DEPHY_LED
endif
ifeq ($(strip ${BRCM_CHIP}),63178)
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_sf2
    PHY_MODULES += bus_drv_sf2
    PHY_MODULES += mac_drv_sf2
    PHY_MODULES += ephy_led_init
    PHY_MODULES += brcm_rgmii
    PHY_MODULES += phy_drv_dsl_phy
    EXTRA_CFLAGS += -DBRCM_RGMII -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_SF2
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_M2M -DEPHY_LED
endif
ifeq ($(strip ${BRCM_CHIP}),6756)
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_sf2
    PHY_MODULES += bus_drv_sf2
    PHY_MODULES += mac_drv_sf2
    PHY_MODULES += ephy_led_init
    PHY_MODULES += phy_drv_ext3
    PHY_MODULES += brcm_rgmii
    PHY_MODULES += serdes_access
    PHY_MODULES += phy_drv_dsl_phy
    PHY_MODULES += phy_drv_dsl_serdes
    PHY_MODULES += phy_drv_dsl_848xx
    PHY_MODULES += phy_drv_6756class_serdes
    DRV_OBJS += $(if $(wildcard $(srctree)/drivers/net/bcmbca/phy/Serdes6756Class/merlin28_shortfin_config.c),Serdes6756Class/merlin28_shortfin_config.o,Serdes6756Class/merlin28_shortfin_config_preb.o)
    EXTRA_CFLAGS += -DBRCM_RGMII -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_SF2_DUAL
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_M2M -DPHY_EXT3 -DEPHY_LED
    EXTRA_CFLAGS += -DPHY_6756CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes6756Class -DEXCLUDE_STD_HEADERS
endif
ifeq ($(strip ${BRCM_CHIP}),63146)
    PHY_MODULES += unimac_drv_impl2
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_v1
    PHY_MODULES += bus_drv_mdio_v1
    PHY_MODULES += mac_drv_unimac
    PHY_MODULES += phy_drv_dsl_phy
    PHY_MODULES += ephy_led_init
    PHY_MODULES += brcm_rgmii
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DBRCM_RGMII -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_UNIMAC -DBRCM_64B_UMAC_MIB
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DEPHY_LED
ifdef CONFIG_BCMBCA_SERDES
    PHY_MODULES += serdes_access
    PHY_MODULES += phy_drv_146class_serdes
    PHY_MODULES += phy_drv_dsl_serdes
    PHY_MODULES += phy_drv_dsl_848xx
    PHY_MODULES += phy_drv_ext3
    DRV_OBJS += $(if $(wildcard $(srctree)/drivers/net/bcmbca/phy/Serdes146Class/merlin16_shortfin_config.c),Serdes146Class/merlin16_shortfin_config.o,Serdes146Class/merlin16_shortfin_config_preb.o)
    EXTRA_CFLAGS += -DPHY_EXT3
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
endif
endif
ifeq ($(strip ${BRCM_CHIP}),4912)
    PHY_MODULES += unimac_drv_impl2
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_v1
    PHY_MODULES += bus_drv_mdio_v1
    PHY_MODULES += mac_drv_unimac
    PHY_MODULES += mac_drv_xport
    PHY_MODULES += phy_drv_dsl_phy
    PHY_MODULES += ephy_led_init
    PHY_MODULES += brcm_rgmii
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DBRCM_RGMII -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_UNIMAC -DBRCM_64B_UMAC_MIB -DMAC_XPORT
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DEPHY_LED
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
ifdef CONFIG_BCMBCA_SERDES
    PHY_MODULES += serdes_access
    PHY_MODULES += phy_drv_146class_serdes
    PHY_MODULES += phy_drv_dsl_serdes
    PHY_MODULES += phy_drv_dsl_848xx
    PHY_MODULES += phy_drv_ext3
    DRV_OBJS += $(if $(wildcard $(srctree)/drivers/net/bcmbca/phy/Serdes146Class/merlin16_shortfin_config.c),Serdes146Class/merlin16_shortfin_config.o,Serdes146Class/merlin16_shortfin_config_preb.o)
    EXTRA_CFLAGS += -DPHY_EXT3
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
endif
endif
ifeq ($(strip ${BRCM_CHIP}),6813)
    PHY_MODULES += unimac_drv_impl2
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_v1
    PHY_MODULES += bus_drv_mdio_v1
    PHY_MODULES += eth_phy_top
    PHY_MODULES += mac_drv_unimac
    PHY_MODULES += mac_drv_xport
    PHY_MODULES += phy_drv_dsl_phy
    PHY_MODULES += ephy_led_init
    EXTRA_CFLAGS += -DBUS_MDIO_V1
    EXTRA_CFLAGS += -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_UNIMAC -DBRCM_64B_UMAC_MIB -DMAC_XPORT
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DEPHY_LED -DXPHY_LED_FIXUP
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
ifdef CONFIG_BCMBCA_SERDES
    PHY_MODULES += serdes_access
    PHY_MODULES += phy_drv_146class_serdes
    PHY_MODULES += phy_drv_dsl_serdes
    PHY_MODULES += phy_drv_dsl_848xx
    PHY_MODULES += phy_drv_ext3
    DRV_OBJS += $(if $(wildcard $(srctree)/drivers/net/bcmbca/phy/Serdes146Class/merlin16_shortfin_config.c),Serdes146Class/merlin16_shortfin_config.o,Serdes146Class/merlin16_shortfin_config_preb.o)
    EXTRA_CFLAGS += -DPHY_EXT3 -DPHY_XPHY
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
endif
endif
ifeq ($(strip ${BRCM_CHIP}),6765)
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_sf2
    PHY_MODULES += bus_drv_sf2
    PHY_MODULES += mac_drv_sf2
    PHY_MODULES += ephy_led_init
    PHY_MODULES += phy_drv_ext3
    PHY_MODULES += eth_phy_top
    PHY_MODULES += serdes_access
    PHY_MODULES += phy_drv_146class_serdes
    PHY_MODULES += phy_drv_dsl_serdes
    PHY_MODULES += phy_drv_dsl_848xx
    PHY_MODULES += phy_drv_dsl_phy
    DRV_OBJS += $(if $(wildcard $(srctree)/drivers/net/bcmbca/phy/Serdes146Class/merlin16_shortfin_config.c),Serdes146Class/merlin16_shortfin_config.o,Serdes146Class/merlin16_shortfin_config_preb.o)
    EXTRA_CFLAGS += -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_SF2_DUAL -DRESCAL_BY_XPHY -DPHY_SHARED_REF_CLK
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_M2M -DPHY_EXT3 -DPHY_XPHY -DEPHY_LED -DXPHY_LED_FIXUP
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
endif
ifeq ($(strip ${BRCM_CHIP}),6766)
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_sf2
    PHY_MODULES += bus_drv_sf2
    PHY_MODULES += mac_drv_sf2
    PHY_MODULES += ephy_led_init
    PHY_MODULES += phy_drv_ext3
    PHY_MODULES += serdes_access
    PHY_MODULES += phy_drv_146class_serdes
    PHY_MODULES += phy_drv_dsl_serdes
    PHY_MODULES += phy_drv_dsl_848xx
    PHY_MODULES += phy_drv_dsl_phy
    DRV_OBJS += $(if $(wildcard $(srctree)/drivers/net/bcmbca/phy/Serdes146Class/merlin16_shortfin_config.c),Serdes146Class/merlin16_shortfin_config.o,Serdes146Class/merlin16_shortfin_config_preb.o)
    EXTRA_CFLAGS += -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_SF2_DUAL -DPHY_SHARED_REF_CLK
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_M2M -DPHY_EXT3 -DEPHY_LED
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
endif
ifeq ($(strip ${BRCM_CHIP}),6764)
    PHY_MODULES += mdio_drv_common
    PHY_MODULES += mdio_drv_sf2
    PHY_MODULES += bus_drv_sf2
    PHY_MODULES += mac_drv_sf2
    PHY_MODULES += ephy_led_init
    PHY_MODULES += phy_drv_ext3
    PHY_MODULES += serdes_access
    PHY_MODULES += phy_drv_146class_serdes
    PHY_MODULES += phy_drv_dsl_serdes
    PHY_MODULES += phy_drv_dsl_848xx
    PHY_MODULES += phy_drv_dsl_phy
    DRV_OBJS += $(if $(wildcard $(srctree)/drivers/net/bcmbca/phy/Serdes146Class/merlin16_shortfin_config.c),Serdes146Class/merlin16_shortfin_config.o,Serdes146Class/merlin16_shortfin_config_preb.o)
    EXTRA_CFLAGS += -DDSL_DEVICES
    EXTRA_CFLAGS += -DMAC_SF2_DUAL -DPHY_SHARED_REF_CLK
    EXTRA_CFLAGS += -DPHY_DSL_GPHY -DPHY_M2M -DPHY_EXT3 -DEPHY_LED
    EXTRA_CFLAGS += -DPHY_146CLASS_SERDES -I$(BCMBCA_PATH)/phy/Serdes146Class -DEXCLUDE_STD_HEADERS
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63158_4908_),)
    PHY_MODULES += ephy_led_init
    PHY_MODULES += dt_access
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_),)
    PHY_MODULES += ephy_led_init_impl2
    PHY_MODULES += dt_access
endif
PHY_SRC := $(addprefix $(srctree)/drivers/net/bcmbca/phy/, $(addsuffix .c,$(PHY_MODULES)))
PHY_OBJS := $(wildcard $(PHY_SRC))
PHY_PREB_OBJS := $(filter-out $(PHY_OBJS),$(PHY_SRC))
DRV_OBJS += $(patsubst $(srctree)/drivers/net/bcmbca/phy/%.c, %.o, $(PHY_OBJS))
DRV_OBJS += $(patsubst $(srctree)/drivers/net/bcmbca/phy/%.c, %_preb.o, $(PHY_PREB_OBJS))

obj-y += DRV_OBJS

$(obj)/%_preb.o: $(srctree)/drivers/net/bcmbca/phy/%.o
	cp $< $@
$(obj)/../xport/ag/$(XPORT_CHIP)/%_preb.o: $(srctree)/drivers/net/bcmbca/xport/ag/$(XPORT_CHIP)/%.o
	cp $< $@
$(obj)/merlin_shortfin/serdes_wrapper_preb.o:
	cp $(srctree)/drivers/net/bcmbca/phy/merlin_shortfin/serdes_wrapper.o $(obj)/merlin_shortfin/serdes_wrapper_preb.o
$(obj)/../xport/xport_drv_preb.o:
	cp $(srctree)/drivers/net/bcmbca/xport/xport_drv.o $(obj)/../xport/xport_drv_preb.o
$(obj)/../xport/xport_stats_preb.o:
	cp $(srctree)/drivers/net/bcmbca/xport/xport_stats.o $(obj)/../xport/xport_stats_preb.o
