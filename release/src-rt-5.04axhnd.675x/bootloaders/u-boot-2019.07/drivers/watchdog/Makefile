# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-$(CONFIG_WDT_AT91) += at91sam9_wdt.o
obj-$(CONFIG_FTWDT010_WATCHDOG) += ftwdt010_wdt.o
ifneq (,$(filter $(SOC), mx25 mx31 mx35 mx5 mx6 mx7 vf610))
obj-y += imx_watchdog.o
else
obj-$(CONFIG_IMX_WATCHDOG) += imx_watchdog.o
endif
obj-$(CONFIG_S5P)               += s5p_wdt.o
obj-$(CONFIG_XILINX_TB_WATCHDOG) += xilinx_tb_wdt.o
obj-$(CONFIG_OMAP_WATCHDOG) += omap_wdt.o
obj-$(CONFIG_DESIGNWARE_WATCHDOG) += designware_wdt.o
obj-$(CONFIG_ULP_WATCHDOG) += ulp_wdog.o
obj-$(CONFIG_WDT) += wdt-uclass.o
obj-$(CONFIG_WDT_SANDBOX) += sandbox_wdt.o
obj-$(CONFIG_WDT_ARMADA_37XX) += armada-37xx-wdt.o
obj-$(CONFIG_WDT_ASPEED) += ast_wdt.o
obj-$(CONFIG_WDT_BCM6345) += bcm6345_wdt.o
obj-$(CONFIG_BCM2835_WDT)       += bcm2835_wdt.o
obj-$(CONFIG_WDT_ORION) += orion_wdt.o
obj-$(CONFIG_WDT_CDNS) += cdns_wdt.o
obj-$(CONFIG_WDT_MPC8xx) += mpc8xx_wdt.o
obj-$(CONFIG_WDT_MT7621) += mt7621_wdt.o
obj-$(CONFIG_WDT_MTK) += mtk_wdt.o
obj-$(CONFIG_WDT_SP805) += sp805_wdt.o
obj-$(CONFIG_WDT_STM32MP) += stm32mp_wdt.o
obj-$(CONFIG_WDT_TANGIER) += tangier_wdt.o
