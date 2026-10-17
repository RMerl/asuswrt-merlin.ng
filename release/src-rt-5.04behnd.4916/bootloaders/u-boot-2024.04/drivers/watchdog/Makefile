# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-$(CONFIG_WDT_AT91) += at91sam9_wdt.o
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
obj-$(CONFIG_$(SPL_TPL_)WDT) += wdt-uclass.o
obj-$(CONFIG_WDT_SANDBOX) += sandbox_wdt.o
obj-$(CONFIG_WDT_ALARM_SANDBOX) += sandbox_alarm-wdt.o
obj-$(CONFIG_WDT_APPLE) += apple_wdt.o
obj-$(CONFIG_WDT_ARM_SMC) += arm_smc_wdt.o
obj-$(CONFIG_WDT_ARMADA_37XX) += armada-37xx-wdt.o
obj-$(CONFIG_WDT_ASPEED) += ast_wdt.o
obj-$(CONFIG_WDT_AST2600) += ast2600_wdt.o
obj-$(CONFIG_WDT_ATCWDT200) += atcwdt200_wdt.o
obj-$(CONFIG_WDT_BCM2835) += bcm2835_wdt.o
obj-$(CONFIG_WDT_BCM6345) += bcm6345_wdt.o
obj-$(CONFIG_WDT_BOOKE) += booke_wdt.o
obj-$(CONFIG_WDT_CORTINA) += cortina_wdt.o
obj-$(CONFIG_WDT_ORION) += orion_wdt.o
obj-$(CONFIG_WDT_CDNS) += cdns_wdt.o
obj-$(CONFIG_WDT_FTWDT010) += ftwdt010_wdt.o
obj-$(CONFIG_WDT_GPIO) += gpio_wdt.o
obj-$(CONFIG_WDT_MAX6370) += max6370_wdt.o
obj-$(CONFIG_WDT_MCF) += mcf_wdt.o
obj-$(CONFIG_WDT_MESON_GXBB) += meson_gxbb_wdt.o
obj-$(CONFIG_WDT_MPC8xxx) += mpc8xxx_wdt.o
obj-$(CONFIG_WDT_MT7620) += mt7620_wdt.o
obj-$(CONFIG_WDT_MT7621) += mt7621_wdt.o
obj-$(CONFIG_WDT_MTK) += mtk_wdt.o
obj-$(CONFIG_WDT_NPCM) += npcm_wdt.o
obj-$(CONFIG_WDT_OCTEONTX) += octeontx_wdt.o
obj-$(CONFIG_WDT_OMAP3) += omap_wdt.o
obj-$(CONFIG_WDT_SBSA) += sbsa_gwdt.o
obj-$(CONFIG_WDT_K3_RTI) += rti_wdt.o
obj-$(CONFIG_WDT_SL28CPLD) += sl28cpld-wdt.o
obj-$(CONFIG_WDT_SP805) += sp805_wdt.o
obj-$(CONFIG_WDT_STARFIVE) += starfive_wdt.o
obj-$(CONFIG_WDT_STM32MP) += stm32mp_wdt.o
obj-$(CONFIG_WDT_SUNXI) += sunxi_wdt.o
obj-$(CONFIG_WDT_TANGIER) += tangier_wdt.o
obj-$(CONFIG_WDT_XILINX) += xilinx_wwdt.o
obj-$(CONFIG_WDT_BCM3390) += bcm3390_wdt.o
