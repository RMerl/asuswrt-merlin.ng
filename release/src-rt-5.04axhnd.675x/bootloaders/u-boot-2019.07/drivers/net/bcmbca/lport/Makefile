BRCM_CHIP = $(patsubst "bcm%",%,$(CONFIG_SYS_SOC))
BCMBCA_PATH = $(srctree)/drivers/net/bcmbca

EXTRA_CFLAGS += -I$(BCMBCA_PATH)/phy
EXTRA_CFLAGS += -I$(BCMBCA_PATH)/lport
EXTRA_CFLAGS += -I$(BCMBCA_PATH)/lport/ag
EXTRA_CFLAGS += -I$(BCMBCA_PATH)/xrdp
EXTRA_CFLAGS += -I$(srctree)/include/dt-bindings/leds

EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP)

obj-y += \
	lport_drv.o \
    lport_mdio.o \
    lport_stats.o \
    serdes_access.o \
    merlin_direct_access.o \
	ag/BCM6858_A0_LPORT_AG.o \
	ag/BCM6858_A0LPORT_CTRL_AG.o \
	ag/BCM6858_A0LPORT_INTR_AG.o \
	ag/BCM6858_A0LPORT_LED_AG.o \
	ag/BCM6858_A0LPORT_MAB_AG.o \
	ag/BCM6858_A0LPORT_MDIO_AG.o \
	ag/BCM6858_A0LPORT_MIB_AG.o \
	ag/BCM6858_A0LPORT_RGMII_AG.o \
	ag/BCM6858_A0LPORT_SRDS_AG.o \
	ag/BCM6858_A0LPORT_XLMAC_AG.o \
	ag/BCM6858_A0MIB_CONF_AG.o \
	ag/BCM6858_A0XLMAC_CONF_AG.o \
	ag/bcm6858_drivers_lport_ag.o \
	ag/bcm6858_lport_ctrl_ag.o \
	ag/bcm6858_lport_intr_ag.o \
	ag/bcm6858_lport_led_ag.o \
	ag/bcm6858_lport_mab_ag.o \
	ag/bcm6858_lport_mdio_ag.o \
	ag/bcm6858_lport_mib_ag.o \
	ag/bcm6858_lport_rgmii_ag.o \
	ag/bcm6858_lport_srds_ag.o \
	ag/bcm6858_lport_xlmac_ag.o \
	ag/bcm6858_mib_conf_ag.o \
	ag/bcm6858_xlmac_conf_ag.o
