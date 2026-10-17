/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018-2019 NXP
 */

#ifndef __ASM_ARCH_IMX8MM_PINS_H__
#define __ASM_ARCH_IMX8MM_PINS_H__

#include <asm/mach-imx/iomux-v3.h>

enum {
	IMX8MM_PAD_GPIO1_IO00_GPIO1_IO0                               =  IOMUX_PAD(0x0290, 0x0028, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO00_CCM_ENET_PHY_REF_CLK_ROOT               =  IOMUX_PAD(0x0290, 0x0028, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO00_XTALOSC_REF_CLK_32K                     =  IOMUX_PAD(0x0290, 0x0028, 5, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO00_CCM_EXT_CLK1                            =  IOMUX_PAD(0x0290, 0x0028, 6, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO01_GPIO1_IO1                               =  IOMUX_PAD(0x0294, 0x002C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO01_PWM1_OUT                                =  IOMUX_PAD(0x0294, 0x002C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO01_XTALOSC_REF_CLK_24M                     =  IOMUX_PAD(0x0294, 0x002C, 5, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO01_CCM_EXT_CLK2                            =  IOMUX_PAD(0x0294, 0x002C, 6, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO02_GPIO1_IO2                               =  IOMUX_PAD(0x0298, 0x0030, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO02_WDOG1_WDOG_B                            =  IOMUX_PAD(0x0298, 0x0030, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO02_WDOG1_WDOG_ANY                          =  IOMUX_PAD(0x0298, 0x0030, 5, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO03_GPIO1_IO3                               =  IOMUX_PAD(0x029C, 0x0034, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO03_USDHC1_VSELECT                          =  IOMUX_PAD(0x029C, 0x0034, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO03_SDMA1_EXT_EVENT0                        =  IOMUX_PAD(0x029C, 0x0034, 5, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO04_GPIO1_IO4                               =  IOMUX_PAD(0x02A0, 0x0038, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO04_USDHC2_VSELECT                          =  IOMUX_PAD(0x02A0, 0x0038, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO04_SDMA1_EXT_EVENT1                        =  IOMUX_PAD(0x02A0, 0x0038, 5, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO05_GPIO1_IO5                               =  IOMUX_PAD(0x02A4, 0x003C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO05_ARM_PLATFORM_M4_NMI                     =  IOMUX_PAD(0x02A4, 0x003C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO05_CCM_PMIC_READY                          =  IOMUX_PAD(0x02A4, 0x003C, 5, 0x04BC, 0, 0),
	IMX8MM_PAD_GPIO1_IO05_SRC_INT_BOOT                            =  IOMUX_PAD(0x02A4, 0x003C, 6, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO06_GPIO1_IO6                               =  IOMUX_PAD(0x02A8, 0x0040, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO06_ENET1_MDC                               =  IOMUX_PAD(0x02A8, 0x0040, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO06_USDHC1_CD_B                             =  IOMUX_PAD(0x02A8, 0x0040, 5, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO06_CCM_EXT_CLK3                            =  IOMUX_PAD(0x02A8, 0x0040, 6, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO07_GPIO1_IO7                               =  IOMUX_PAD(0x02AC, 0x0044, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO07_ENET1_MDIO                              =  IOMUX_PAD(0x02AC, 0x0044, 1, 0x04C0, 0, 0),
	IMX8MM_PAD_GPIO1_IO07_USDHC1_WP                               =  IOMUX_PAD(0x02AC, 0x0044, 5, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO07_CCM_EXT_CLK4                            =  IOMUX_PAD(0x02AC, 0x0044, 6, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO08_GPIO1_IO8                               =  IOMUX_PAD(0x02B0, 0x0048, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO08_ENET1_1588_EVENT0_IN                    =  IOMUX_PAD(0x02B0, 0x0048, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO08_USDHC2_RESET_B                          =  IOMUX_PAD(0x02B0, 0x0048, 5, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO08_CCM_WAIT                                =  IOMUX_PAD(0x02B0, 0x0048, 6, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO09_GPIO1_IO9                               =  IOMUX_PAD(0x02B4, 0x004C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO09_ENET1_1588_EVENT0_OUT                   =  IOMUX_PAD(0x02B4, 0x004C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO09_USDHC3_RESET_B                          =  IOMUX_PAD(0x02B4, 0x004C, 4, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO09_SDMA2_EXT_EVENT0                        =  IOMUX_PAD(0x02B4, 0x004C, 5, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO09_CCM_STOP                                =  IOMUX_PAD(0x02B4, 0x004C, 6, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO10_GPIO1_IO10                              =  IOMUX_PAD(0x02B8, 0x0050, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO10_USB1_OTG_ID                             =  IOMUX_PAD(0x02B8, 0x0050, 1, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO11_GPIO1_IO11                              =  IOMUX_PAD(0x02BC, 0x0054, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO11_USB2_OTG_ID                             =  IOMUX_PAD(0x02BC, 0x0054, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO11_USDHC3_VSELECT                          =  IOMUX_PAD(0x02BC, 0x0054, 4, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO11_CCM_PMIC_READY                          =  IOMUX_PAD(0x02BC, 0x0054, 5, 0x04BC, 1, 0),
	IMX8MM_PAD_GPIO1_IO11_CCM_OUT0                                =  IOMUX_PAD(0x02BC, 0x0054, 6, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO12_GPIO1_IO12                              =  IOMUX_PAD(0x02C0, 0x0058, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO12_USB1_OTG_PWR                            =  IOMUX_PAD(0x02C0, 0x0058, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO12_SDMA2_EXT_EVENT1                        =  IOMUX_PAD(0x02C0, 0x0058, 5, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO12_CCM_OUT1                                =  IOMUX_PAD(0x02C0, 0x0058, 6, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO13_GPIO1_IO13                              =  IOMUX_PAD(0x02C4, 0x005C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO13_USB1_OTG_OC                             =  IOMUX_PAD(0x02C4, 0x005C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO13_PWM2_OUT                                =  IOMUX_PAD(0x02C4, 0x005C, 5, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO13_CCM_OUT2                                =  IOMUX_PAD(0x02C4, 0x005C, 6, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO14_GPIO1_IO14                              =  IOMUX_PAD(0x02C8, 0x0060, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO14_USB2_OTG_PWR                            =  IOMUX_PAD(0x02C8, 0x0060, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO14_USDHC3_CD_B                             =  IOMUX_PAD(0x02C8, 0x0060, 4, 0x0544, 2, 0),
	IMX8MM_PAD_GPIO1_IO14_PWM3_OUT                                =  IOMUX_PAD(0x02C8, 0x0060, 5, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO14_CCM_CLKO1                               =  IOMUX_PAD(0x02C8, 0x0060, 6, 0x0000, 0, 0),

	IMX8MM_PAD_GPIO1_IO15_GPIO1_IO15                              =  IOMUX_PAD(0x02CC, 0x0064, 0, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO15_USB2_OTG_OC                             =  IOMUX_PAD(0x02CC, 0x0064, 1, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO15_USDHC3_WP                               =  IOMUX_PAD(0x02CC, 0x0064, 4, 0x0548, 2, 0),
	IMX8MM_PAD_GPIO1_IO15_PWM4_OUT                                =  IOMUX_PAD(0x02CC, 0x0064, 5, 0x0000, 0, 0),
	IMX8MM_PAD_GPIO1_IO15_CCM_CLKO2                               =  IOMUX_PAD(0x02CC, 0x0064, 6, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_MDC_ENET1_MDC                                 =  IOMUX_PAD(0x02D0, 0x0068, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_MDC_GPIO1_IO16                                =  IOMUX_PAD(0x02D0, 0x0068, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_MDIO_ENET1_MDIO                               =  IOMUX_PAD(0x02D4, 0x006C, 0, 0x04C0, 1, 0),
	IMX8MM_PAD_ENET_MDIO_GPIO1_IO17                               =  IOMUX_PAD(0x02D4, 0x006C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_TD3_ENET1_RGMII_TD3                           =  IOMUX_PAD(0x02D8, 0x0070, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_TD3_GPIO1_IO18                                =  IOMUX_PAD(0x02D8, 0x0070, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_TD2_ENET1_RGMII_TD2                           =  IOMUX_PAD(0x02DC, 0x0074, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_TD2_ENET1_TX_CLK                              =  IOMUX_PAD(0x02DC, 0x0074, 1, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_TD2_CCM_ENET_REF_CLK_ROOT                     =  IOMUX_PAD(0x02DC, 0x0074, 1, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_TD2_GPIO1_IO19                                =  IOMUX_PAD(0x02DC, 0x0074, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_TD1_ENET1_RGMII_TD1                           =  IOMUX_PAD(0x02E0, 0x0078, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_TD1_GPIO1_IO20                                =  IOMUX_PAD(0x02E0, 0x0078, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_TD0_ENET1_RGMII_TD0                           =  IOMUX_PAD(0x02E4, 0x007C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_TD0_GPIO1_IO21                                =  IOMUX_PAD(0x02E4, 0x007C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_TX_CTL_ENET1_RGMII_TX_CTL                     =  IOMUX_PAD(0x02E8, 0x0080, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_TX_CTL_GPIO1_IO22                             =  IOMUX_PAD(0x02E8, 0x0080, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_TXC_ENET1_RGMII_TXC                           =  IOMUX_PAD(0x02EC, 0x0084, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_TXC_ENET1_TX_ER                               =  IOMUX_PAD(0x02EC, 0x0084, 1, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_TXC_GPIO1_IO23                                =  IOMUX_PAD(0x02EC, 0x0084, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_RX_CTL_ENET1_RGMII_RX_CTL                     =  IOMUX_PAD(0x02F0, 0x0088, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_RX_CTL_GPIO1_IO24                             =  IOMUX_PAD(0x02F0, 0x0088, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_RXC_ENET1_RGMII_RXC                           =  IOMUX_PAD(0x02F4, 0x008C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_RXC_ENET1_RX_ER                               =  IOMUX_PAD(0x02F4, 0x008C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_RXC_GPIO1_IO25                                =  IOMUX_PAD(0x02F4, 0x008C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_RD0_ENET1_RGMII_RD0                           =  IOMUX_PAD(0x02F8, 0x0090, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_RD0_GPIO1_IO26                                =  IOMUX_PAD(0x02F8, 0x0090, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_RD1_ENET1_RGMII_RD1                           =  IOMUX_PAD(0x02FC, 0x0094, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_RD1_GPIO1_IO27                                =  IOMUX_PAD(0x02FC, 0x0094, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_RD2_ENET1_RGMII_RD2                           =  IOMUX_PAD(0x0300, 0x0098, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_RD2_GPIO1_IO28                                =  IOMUX_PAD(0x0300, 0x0098, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ENET_RD3_ENET1_RGMII_RD3                           =  IOMUX_PAD(0x0304, 0x009C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ENET_RD3_GPIO1_IO29                                =  IOMUX_PAD(0x0304, 0x009C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD1_CLK_USDHC1_CLK                                 =  IOMUX_PAD(0x0308, 0x00A0, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD1_CLK_GPIO2_IO0                                  =  IOMUX_PAD(0x0308, 0x00A0, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD1_CMD_USDHC1_CMD                                 =  IOMUX_PAD(0x030C, 0x00A4, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD1_CMD_GPIO2_IO1                                  =  IOMUX_PAD(0x030C, 0x00A4, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD1_DATA0_USDHC1_DATA0                             =  IOMUX_PAD(0x0310, 0x00A8, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD1_DATA0_GPIO2_IO2                                =  IOMUX_PAD(0x0310, 0x00A8, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD1_DATA1_USDHC1_DATA1                             =  IOMUX_PAD(0x0314, 0x00AC, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD1_DATA1_GPIO2_IO3                                =  IOMUX_PAD(0x0314, 0x00AC, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD1_DATA2_USDHC1_DATA2                             =  IOMUX_PAD(0x0318, 0x00B0, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD1_DATA2_GPIO2_IO4                                =  IOMUX_PAD(0x0318, 0x00B0, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD1_DATA3_USDHC1_DATA3                             =  IOMUX_PAD(0x031C, 0x00B4, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD1_DATA3_GPIO2_IO5                                =  IOMUX_PAD(0x031C, 0x00B4, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD1_DATA4_USDHC1_DATA4                             =  IOMUX_PAD(0x0320, 0x00B8, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD1_DATA4_GPIO2_IO6                                =  IOMUX_PAD(0x0320, 0x00B8, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD1_DATA5_USDHC1_DATA5                             =  IOMUX_PAD(0x0324, 0x00BC, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD1_DATA5_GPIO2_IO7                                =  IOMUX_PAD(0x0324, 0x00BC, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD1_DATA6_USDHC1_DATA6                             =  IOMUX_PAD(0x0328, 0x00C0, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD1_DATA6_GPIO2_IO8                                =  IOMUX_PAD(0x0328, 0x00C0, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD1_DATA7_USDHC1_DATA7                             =  IOMUX_PAD(0x032C, 0x00C4, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD1_DATA7_GPIO2_IO9                                =  IOMUX_PAD(0x032C, 0x00C4, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD1_RESET_B_USDHC1_RESET_B                         =  IOMUX_PAD(0x0330, 0x00C8, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD1_RESET_B_GPIO2_IO10                             =  IOMUX_PAD(0x0330, 0x00C8, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD1_STROBE_USDHC1_STROBE                           =  IOMUX_PAD(0x0334, 0x00CC, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD1_STROBE_GPIO2_IO11                              =  IOMUX_PAD(0x0334, 0x00CC, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD2_CD_B_USDHC2_CD_B                               =  IOMUX_PAD(0x0338, 0x00D0, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_CD_B_GPIO2_IO12                                =  IOMUX_PAD(0x0338, 0x00D0, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SD2_CLK_USDHC2_CLK                                 =  IOMUX_PAD(0x033C, 0x00D4, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_CLK_GPIO2_IO13                                 =  IOMUX_PAD(0x033C, 0x00D4, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_CLK_CCM_OBSERVE0                               =  IOMUX_PAD(0x033C, 0x00D4, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SD2_CMD_USDHC2_CMD                                 =  IOMUX_PAD(0x0340, 0x00D8, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_CMD_GPIO2_IO14                                 =  IOMUX_PAD(0x0340, 0x00D8, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_CMD_CCM_OBSERVE1                               =  IOMUX_PAD(0x0340, 0x00D8, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SD2_DATA0_USDHC2_DATA0                             =  IOMUX_PAD(0x0344, 0x00DC, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_DATA0_GPIO2_IO15                               =  IOMUX_PAD(0x0344, 0x00DC, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_DATA0_CCM_OBSERVE2                             =  IOMUX_PAD(0x0344, 0x00DC, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SD2_DATA1_USDHC2_DATA1                             =  IOMUX_PAD(0x0348, 0x00E0, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_DATA1_GPIO2_IO16                               =  IOMUX_PAD(0x0348, 0x00E0, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_DATA1_CCM_WAIT                                 =  IOMUX_PAD(0x0348, 0x00E0, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SD2_DATA2_USDHC2_DATA2                             =  IOMUX_PAD(0x034C, 0x00E4, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_DATA2_GPIO2_IO17                               =  IOMUX_PAD(0x034C, 0x00E4, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_DATA2_CCM_STOP                                 =  IOMUX_PAD(0x034C, 0x00E4, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SD2_DATA3_USDHC2_DATA3                             =  IOMUX_PAD(0x0350, 0x00E8, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_DATA3_GPIO2_IO18                               =  IOMUX_PAD(0x0350, 0x00E8, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_DATA3_SRC_EARLY_RESET                          =  IOMUX_PAD(0x0350, 0x00E8, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SD2_RESET_B_USDHC2_RESET_B                         =  IOMUX_PAD(0x0354, 0x00EC, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_RESET_B_GPIO2_IO19                             =  IOMUX_PAD(0x0354, 0x00EC, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_RESET_B_SRC_SYSTEM_RESET                       =  IOMUX_PAD(0x0354, 0x00EC, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SD2_WP_USDHC2_WP                                   =  IOMUX_PAD(0x0358, 0x00F0, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SD2_WP_GPIO2_IO20                                  =  IOMUX_PAD(0x0358, 0x00F0, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_ALE_RAWNAND_ALE                               =  IOMUX_PAD(0x035C, 0x00F4, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_ALE_QSPI_A_SCLK                               =  IOMUX_PAD(0x035C, 0x00F4, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_ALE_GPIO3_IO0                                 =  IOMUX_PAD(0x035C, 0x00F4, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_CE0_B_RAWNAND_CE0_B                           =  IOMUX_PAD(0x0360, 0x00F8, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CE0_B_QSPI_A_SS0_B                            =  IOMUX_PAD(0x0360, 0x00F8, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CE0_B_GPIO3_IO1                               =  IOMUX_PAD(0x0360, 0x00F8, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_CE1_B_RAWNAND_CE1_B                           =  IOMUX_PAD(0x0364, 0x00FC, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CE1_B_QSPI_A_SS1_B                            =  IOMUX_PAD(0x0364, 0x00FC, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CE1_B_USDHC3_STROBE                           =  IOMUX_PAD(0x0364, 0x00FC, 2, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CE1_B_GPIO3_IO2                               =  IOMUX_PAD(0x0364, 0x00FC, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_CE2_B_RAWNAND_CE2_B                           =  IOMUX_PAD(0x0368, 0x0100, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CE2_B_QSPI_B_SS0_B                            =  IOMUX_PAD(0x0368, 0x0100, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CE2_B_USDHC3_DATA5                            =  IOMUX_PAD(0x0368, 0x0100, 2, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CE2_B_GPIO3_IO3                               =  IOMUX_PAD(0x0368, 0x0100, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_CE3_B_RAWNAND_CE3_B                           =  IOMUX_PAD(0x036C, 0x0104, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CE3_B_QSPI_B_SS1_B                            =  IOMUX_PAD(0x036C, 0x0104, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CE3_B_USDHC3_DATA6                            =  IOMUX_PAD(0x036C, 0x0104, 2, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CE3_B_GPIO3_IO4                               =  IOMUX_PAD(0x036C, 0x0104, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_CLE_RAWNAND_CLE                               =  IOMUX_PAD(0x0370, 0x0108, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CLE_QSPI_B_SCLK                               =  IOMUX_PAD(0x0370, 0x0108, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CLE_USDHC3_DATA7                              =  IOMUX_PAD(0x0370, 0x0108, 2, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_CLE_GPIO3_IO5                                 =  IOMUX_PAD(0x0370, 0x0108, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_DATA00_RAWNAND_DATA00                         =  IOMUX_PAD(0x0374, 0x010C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA00_QSPI_A_DATA0                           =  IOMUX_PAD(0x0374, 0x010C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA00_GPIO3_IO6                              =  IOMUX_PAD(0x0374, 0x010C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_DATA01_RAWNAND_DATA01                         =  IOMUX_PAD(0x0378, 0x0110, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA01_QSPI_A_DATA1                           =  IOMUX_PAD(0x0378, 0x0110, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA01_GPIO3_IO7                              =  IOMUX_PAD(0x0378, 0x0110, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_DATA02_RAWNAND_DATA02                         =  IOMUX_PAD(0x037C, 0x0114, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA02_QSPI_A_DATA2                           =  IOMUX_PAD(0x037C, 0x0114, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA02_USDHC3_CD_B                            =  IOMUX_PAD(0x037C, 0x0114, 2, 0x0544, 0, 0),
	IMX8MM_PAD_NAND_DATA02_GPIO3_IO8                              =  IOMUX_PAD(0x037C, 0x0114, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_DATA03_RAWNAND_DATA03                         =  IOMUX_PAD(0x0380, 0x0118, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA03_QSPI_A_DATA3                           =  IOMUX_PAD(0x0380, 0x0118, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA03_USDHC3_WP                              =  IOMUX_PAD(0x0380, 0x0118, 2, 0x0548, 0, 0),
	IMX8MM_PAD_NAND_DATA03_GPIO3_IO9                              =  IOMUX_PAD(0x0380, 0x0118, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_DATA04_RAWNAND_DATA04                         =  IOMUX_PAD(0x0384, 0x011C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA04_QSPI_B_DATA0                           =  IOMUX_PAD(0x0384, 0x011C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA04_USDHC3_DATA0                           =  IOMUX_PAD(0x0384, 0x011C, 2, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA04_GPIO3_IO10                             =  IOMUX_PAD(0x0384, 0x011C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_DATA05_RAWNAND_DATA05                         =  IOMUX_PAD(0x0388, 0x0120, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA05_QSPI_B_DATA1                           =  IOMUX_PAD(0x0388, 0x0120, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA05_USDHC3_DATA1                           =  IOMUX_PAD(0x0388, 0x0120, 2, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA05_GPIO3_IO11                             =  IOMUX_PAD(0x0388, 0x0120, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_DATA06_RAWNAND_DATA06                         =  IOMUX_PAD(0x038C, 0x0124, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA06_QSPI_B_DATA2                           =  IOMUX_PAD(0x038C, 0x0124, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA06_USDHC3_DATA2                           =  IOMUX_PAD(0x038C, 0x0124, 2, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA06_GPIO3_IO12                             =  IOMUX_PAD(0x038C, 0x0124, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_DATA07_RAWNAND_DATA07                         =  IOMUX_PAD(0x0390, 0x0128, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA07_QSPI_B_DATA3                           =  IOMUX_PAD(0x0390, 0x0128, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA07_USDHC3_DATA3                           =  IOMUX_PAD(0x0390, 0x0128, 2, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DATA07_GPIO3_IO13                             =  IOMUX_PAD(0x0390, 0x0128, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_DQS_RAWNAND_DQS                               =  IOMUX_PAD(0x0394, 0x012C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DQS_QSPI_A_DQS                                =  IOMUX_PAD(0x0394, 0x012C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_DQS_GPIO3_IO14                                =  IOMUX_PAD(0x0394, 0x012C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_RE_B_RAWNAND_RE_B                             =  IOMUX_PAD(0x0398, 0x0130, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_RE_B_QSPI_B_DQS                               =  IOMUX_PAD(0x0398, 0x0130, 1, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_RE_B_USDHC3_DATA4                             =  IOMUX_PAD(0x0398, 0x0130, 2, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_RE_B_GPIO3_IO15                               =  IOMUX_PAD(0x0398, 0x0130, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_READY_B_RAWNAND_READY_B                       =  IOMUX_PAD(0x039C, 0x0134, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_READY_B_USDHC3_RESET_B                        =  IOMUX_PAD(0x039C, 0x0134, 2, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_READY_B_GPIO3_IO16                            =  IOMUX_PAD(0x039C, 0x0134, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_WE_B_RAWNAND_WE_B                             =  IOMUX_PAD(0x03A0, 0x0138, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_WE_B_USDHC3_CLK                               =  IOMUX_PAD(0x03A0, 0x0138, 2 | IOMUX_CONFIG_SION, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_WE_B_GPIO3_IO17                               =  IOMUX_PAD(0x03A0, 0x0138, 5, 0x0000, 0, 0),

	IMX8MM_PAD_NAND_WP_B_RAWNAND_WP_B                             =  IOMUX_PAD(0x03A4, 0x013C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_WP_B_USDHC3_CMD                               =  IOMUX_PAD(0x03A4, 0x013C, 2, 0x0000, 0, 0),
	IMX8MM_PAD_NAND_WP_B_GPIO3_IO18                               =  IOMUX_PAD(0x03A4, 0x013C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI5_RXFS_SAI5_RX_SYNC                             =  IOMUX_PAD(0x03A8, 0x0140, 0, 0x04E4, 0, 0),
	IMX8MM_PAD_SAI5_RXFS_SAI1_TX_DATA0                            =  IOMUX_PAD(0x03A8, 0x0140, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI5_RXFS_GPIO3_IO19                               =  IOMUX_PAD(0x03A8, 0x0140, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI5_RXC_SAI5_RX_BCLK                              =  IOMUX_PAD(0x03AC, 0x0144, 0, 0x04D0, 0, 0),
	IMX8MM_PAD_SAI5_RXC_SAI1_TX_DATA1                             =  IOMUX_PAD(0x03AC, 0x0144, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI5_RXC_PDM_CLK                                   =  IOMUX_PAD(0x03AC, 0x0144, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI5_RXC_GPIO3_IO20                                =  IOMUX_PAD(0x03AC, 0x0144, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI5_RXD0_SAI5_RX_DATA0                            =  IOMUX_PAD(0x03B0, 0x0148, 0, 0x04D4, 0, 0),
	IMX8MM_PAD_SAI5_RXD0_SAI1_TX_DATA2                            =  IOMUX_PAD(0x03B0, 0x0148, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI5_RXD0_PDM_BIT_STREAM0                          =  IOMUX_PAD(0x03B0, 0x0148, 4, 0x0534, 0, 0),
	IMX8MM_PAD_SAI5_RXD0_GPIO3_IO21                               =  IOMUX_PAD(0x03B0, 0x0148, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI5_RXD1_SAI5_RX_DATA1                            =  IOMUX_PAD(0x03B4, 0x014C, 0, 0x04D8, 0, 0),
	IMX8MM_PAD_SAI5_RXD1_SAI1_TX_DATA3                            =  IOMUX_PAD(0x03B4, 0x014C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI5_RXD1_SAI1_TX_SYNC                             =  IOMUX_PAD(0x03B4, 0x014C, 2, 0x04CC, 0, 0),
	IMX8MM_PAD_SAI5_RXD1_SAI5_TX_SYNC                             =  IOMUX_PAD(0x03B4, 0x014C, 3, 0x04EC, 0, 0),
	IMX8MM_PAD_SAI5_RXD1_PDM_BIT_STREAM1                          =  IOMUX_PAD(0x03B4, 0x014C, 4, 0x0538, 0, 0),
	IMX8MM_PAD_SAI5_RXD1_GPIO3_IO22                               =  IOMUX_PAD(0x03B4, 0x014C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI5_RXD2_SAI5_RX_DATA2                            =  IOMUX_PAD(0x03B8, 0x0150, 0, 0x04DC, 0, 0),
	IMX8MM_PAD_SAI5_RXD2_SAI1_TX_DATA4                            =  IOMUX_PAD(0x03B8, 0x0150, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI5_RXD2_SAI1_TX_SYNC                             =  IOMUX_PAD(0x03B8, 0x0150, 2, 0x04CC, 1, 0),
	IMX8MM_PAD_SAI5_RXD2_SAI5_TX_BCLK                             =  IOMUX_PAD(0x03B8, 0x0150, 3, 0x04E8, 0, 0),
	IMX8MM_PAD_SAI5_RXD2_PDM_BIT_STREAM2                          =  IOMUX_PAD(0x03B8, 0x0150, 4, 0x053C, 0, 0),
	IMX8MM_PAD_SAI5_RXD2_GPIO3_IO23                               =  IOMUX_PAD(0x03B8, 0x0150, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI5_RXD3_SAI5_RX_DATA3                            =  IOMUX_PAD(0x03BC, 0x0154, 0, 0x04E0, 0, 0),
	IMX8MM_PAD_SAI5_RXD3_SAI1_TX_DATA5                            =  IOMUX_PAD(0x03BC, 0x0154, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI5_RXD3_SAI1_TX_SYNC                             =  IOMUX_PAD(0x03BC, 0x0154, 2, 0x04CC, 2, 0),
	IMX8MM_PAD_SAI5_RXD3_SAI5_TX_DATA0                            =  IOMUX_PAD(0x03BC, 0x0154, 3, 0x0000, 0, 0),
	IMX8MM_PAD_SAI5_RXD3_PDM_BIT_STREAM3                          =  IOMUX_PAD(0x03BC, 0x0154, 4, 0x0540, 0, 0),
	IMX8MM_PAD_SAI5_RXD3_GPIO3_IO24                               =  IOMUX_PAD(0x03BC, 0x0154, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI5_MCLK_SAI5_MCLK                                =  IOMUX_PAD(0x03C0, 0x0158, 0, 0x052C, 0, 0),
	IMX8MM_PAD_SAI5_MCLK_SAI1_TX_BCLK                             =  IOMUX_PAD(0x03C0, 0x0158, 1, 0x04C8, 0, 0),
	IMX8MM_PAD_SAI5_MCLK_GPIO3_IO25                               =  IOMUX_PAD(0x03C0, 0x0158, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI5_MCLK_SRC_TESTER_ACK                           =  IOMUX_PAD(0x03C0, 0x0158, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_RXFS_SAI1_RX_SYNC                             =  IOMUX_PAD(0x03C4, 0x015C, 0, 0x04C4, 0, 0),
	IMX8MM_PAD_SAI1_RXFS_SAI5_RX_SYNC                             =  IOMUX_PAD(0x03C4, 0x015C, 1, 0x04E4, 1, 0),
	IMX8MM_PAD_SAI1_RXFS_ARM_PLATFORM_TRACE_CLK                   =  IOMUX_PAD(0x03C4, 0x015C, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXFS_GPIO4_IO0                                =  IOMUX_PAD(0x03C4, 0x015C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_RXC_SAI1_RX_BCLK                              =  IOMUX_PAD(0x03C8, 0x0160, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXC_SAI5_RX_BCLK                              =  IOMUX_PAD(0x03C8, 0x0160, 1, 0x04D0, 1, 0),
	IMX8MM_PAD_SAI1_RXC_ARM_PLATFORM_TRACE_CTL                    =  IOMUX_PAD(0x03C8, 0x0160, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXC_GPIO4_IO1                                 =  IOMUX_PAD(0x03C8, 0x0160, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_RXD0_SAI1_RX_DATA0                            =  IOMUX_PAD(0x03CC, 0x0164, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD0_SAI5_RX_DATA0                            =  IOMUX_PAD(0x03CC, 0x0164, 1, 0x04D4, 1, 0),
	IMX8MM_PAD_SAI1_RXD0_SAI1_TX_DATA1                            =  IOMUX_PAD(0x03CC, 0x0164, 2, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD0_PDM_BIT_STREAM0                          =  IOMUX_PAD(0x03CC, 0x0164, 3, 0x0534, 1, 0),
	IMX8MM_PAD_SAI1_RXD0_ARM_PLATFORM_TRACE0                      =  IOMUX_PAD(0x03CC, 0x0164, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD0_GPIO4_IO2                                =  IOMUX_PAD(0x03CC, 0x0164, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD0_SRC_BOOT_CFG0                            =  IOMUX_PAD(0x03CC, 0x0164, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_RXD1_SAI1_RX_DATA1                            =  IOMUX_PAD(0x03D0, 0x0168, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD1_SAI5_RX_DATA1                            =  IOMUX_PAD(0x03D0, 0x0168, 1, 0x04D8, 1, 0),
	IMX8MM_PAD_SAI1_RXD1_PDM_BIT_STREAM1                          =  IOMUX_PAD(0x03D0, 0x0168, 3, 0x0538, 1, 0),
	IMX8MM_PAD_SAI1_RXD1_ARM_PLATFORM_TRACE1                      =  IOMUX_PAD(0x03D0, 0x0168, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD1_GPIO4_IO3                                =  IOMUX_PAD(0x03D0, 0x0168, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD1_SRC_BOOT_CFG1                            =  IOMUX_PAD(0x03D0, 0x0168, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_RXD2_SAI1_RX_DATA2                            =  IOMUX_PAD(0x03D4, 0x016C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD2_SAI5_RX_DATA2                            =  IOMUX_PAD(0x03D4, 0x016C, 1, 0x04DC, 1, 0),
	IMX8MM_PAD_SAI1_RXD2_PDM_BIT_STREAM2                          =  IOMUX_PAD(0x03D4, 0x016C, 3, 0x053C, 1, 0),
	IMX8MM_PAD_SAI1_RXD2_ARM_PLATFORM_TRACE2                      =  IOMUX_PAD(0x03D4, 0x016C, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD2_GPIO4_IO4                                =  IOMUX_PAD(0x03D4, 0x016C, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD2_SRC_BOOT_CFG2                            =  IOMUX_PAD(0x03D4, 0x016C, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_RXD3_SAI1_RX_DATA3                            =  IOMUX_PAD(0x03D8, 0x0170, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD3_SAI5_RX_DATA3                            =  IOMUX_PAD(0x03D8, 0x0170, 1, 0x04E0, 1, 0),
	IMX8MM_PAD_SAI1_RXD3_PDM_BIT_STREAM3                          =  IOMUX_PAD(0x03D8, 0x0170, 3, 0x0540, 1, 0),
	IMX8MM_PAD_SAI1_RXD3_ARM_PLATFORM_TRACE3                      =  IOMUX_PAD(0x03D8, 0x0170, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD3_GPIO4_IO5                                =  IOMUX_PAD(0x03D8, 0x0170, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD3_SRC_BOOT_CFG3                            =  IOMUX_PAD(0x03D8, 0x0170, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_RXD4_SAI1_RX_DATA4                            =  IOMUX_PAD(0x03DC, 0x0174, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD4_SAI6_TX_BCLK                             =  IOMUX_PAD(0x03DC, 0x0174, 1, 0x051C, 0, 0),
	IMX8MM_PAD_SAI1_RXD4_SAI6_RX_BCLK                             =  IOMUX_PAD(0x03DC, 0x0174, 2, 0x0510, 0, 0),
	IMX8MM_PAD_SAI1_RXD4_ARM_PLATFORM_TRACE4                      =  IOMUX_PAD(0x03DC, 0x0174, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD4_GPIO4_IO6                                =  IOMUX_PAD(0x03DC, 0x0174, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD4_SRC_BOOT_CFG4                            =  IOMUX_PAD(0x03DC, 0x0174, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_RXD5_SAI1_RX_DATA5                            =  IOMUX_PAD(0x03E0, 0x0178, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD5_SAI6_TX_DATA0                            =  IOMUX_PAD(0x03E0, 0x0178, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD5_SAI6_RX_DATA0                            =  IOMUX_PAD(0x03E0, 0x0178, 2, 0x0514, 0, 0),
	IMX8MM_PAD_SAI1_RXD5_SAI1_RX_SYNC                             =  IOMUX_PAD(0x03E0, 0x0178, 3, 0x04C4, 1, 0),
	IMX8MM_PAD_SAI1_RXD5_ARM_PLATFORM_TRACE5                      =  IOMUX_PAD(0x03E0, 0x0178, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD5_GPIO4_IO7                                =  IOMUX_PAD(0x03E0, 0x0178, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD5_SRC_BOOT_CFG5                            =  IOMUX_PAD(0x03E0, 0x0178, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_RXD6_SAI1_RX_DATA6                            =  IOMUX_PAD(0x03E4, 0x017C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD6_SAI6_TX_SYNC                             =  IOMUX_PAD(0x03E4, 0x017C, 1, 0x0520, 0, 0),
	IMX8MM_PAD_SAI1_RXD6_SAI6_RX_SYNC                             =  IOMUX_PAD(0x03E4, 0x017C, 2, 0x0518, 0, 0),
	IMX8MM_PAD_SAI1_RXD6_ARM_PLATFORM_TRACE6                      =  IOMUX_PAD(0x03E4, 0x017C, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD6_GPIO4_IO8                                =  IOMUX_PAD(0x03E4, 0x017C, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD6_SRC_BOOT_CFG6                            =  IOMUX_PAD(0x03E4, 0x017C, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_RXD7_SAI1_RX_DATA7                            =  IOMUX_PAD(0x03E8, 0x0180, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD7_SAI6_MCLK                                =  IOMUX_PAD(0x03E8, 0x0180, 1, 0x0530, 0, 0),
	IMX8MM_PAD_SAI1_RXD7_SAI1_TX_SYNC                             =  IOMUX_PAD(0x03E8, 0x0180, 2, 0x04CC, 4, 0),
	IMX8MM_PAD_SAI1_RXD7_SAI1_TX_DATA4                            =  IOMUX_PAD(0x03E8, 0x0180, 3, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD7_ARM_PLATFORM_TRACE7                      =  IOMUX_PAD(0x03E8, 0x0180, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD7_GPIO4_IO9                                =  IOMUX_PAD(0x03E8, 0x0180, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_RXD7_SRC_BOOT_CFG7                            =  IOMUX_PAD(0x03E8, 0x0180, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_TXFS_SAI1_TX_SYNC                             =  IOMUX_PAD(0x03EC, 0x0184, 0, 0x04CC, 3, 0),
	IMX8MM_PAD_SAI1_TXFS_SAI5_TX_SYNC                             =  IOMUX_PAD(0x03EC, 0x0184, 1, 0x04EC, 1, 0),
	IMX8MM_PAD_SAI1_TXFS_ARM_PLATFORM_EVENTO                      =  IOMUX_PAD(0x03EC, 0x0184, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXFS_GPIO4_IO10                               =  IOMUX_PAD(0x03EC, 0x0184, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_TXC_SAI1_TX_BCLK                              =  IOMUX_PAD(0x03F0, 0x0188, 0, 0x04C8, 1, 0),
	IMX8MM_PAD_SAI1_TXC_SAI5_TX_BCLK                              =  IOMUX_PAD(0x03F0, 0x0188, 1, 0x04E8, 1, 0),
	IMX8MM_PAD_SAI1_TXC_ARM_PLATFORM_EVENTI                       =  IOMUX_PAD(0x03F0, 0x0188, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXC_GPIO4_IO11                                =  IOMUX_PAD(0x03F0, 0x0188, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_TXD0_SAI1_TX_DATA0                            =  IOMUX_PAD(0x03F4, 0x018C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD0_SAI5_TX_DATA0                            =  IOMUX_PAD(0x03F4, 0x018C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD0_ARM_PLATFORM_TRACE8                      =  IOMUX_PAD(0x03F4, 0x018C, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD0_GPIO4_IO12                               =  IOMUX_PAD(0x03F4, 0x018C, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD0_SRC_BOOT_CFG8                            =  IOMUX_PAD(0x03F4, 0x018C, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_TXD1_SAI1_TX_DATA1                            =  IOMUX_PAD(0x03F8, 0x0190, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD1_SAI5_TX_DATA1                            =  IOMUX_PAD(0x03F8, 0x0190, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD1_ARM_PLATFORM_TRACE9                      =  IOMUX_PAD(0x03F8, 0x0190, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD1_GPIO4_IO13                               =  IOMUX_PAD(0x03F8, 0x0190, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD1_SRC_BOOT_CFG9                            =  IOMUX_PAD(0x03F8, 0x0190, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_TXD2_SAI1_TX_DATA2                            =  IOMUX_PAD(0x03FC, 0x0194, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD2_SAI5_TX_DATA2                            =  IOMUX_PAD(0x03FC, 0x0194, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD2_ARM_PLATFORM_TRACE10                     =  IOMUX_PAD(0x03FC, 0x0194, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD2_GPIO4_IO14                               =  IOMUX_PAD(0x03FC, 0x0194, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD2_SRC_BOOT_CFG10                           =  IOMUX_PAD(0x03FC, 0x0194, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_TXD3_SAI1_TX_DATA3                            =  IOMUX_PAD(0x0400, 0x0198, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD3_SAI5_TX_DATA3                            =  IOMUX_PAD(0x0400, 0x0198, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD3_ARM_PLATFORM_TRACE11                     =  IOMUX_PAD(0x0400, 0x0198, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD3_GPIO4_IO15                               =  IOMUX_PAD(0x0400, 0x0198, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD3_SRC_BOOT_CFG11                           =  IOMUX_PAD(0x0400, 0x0198, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_TXD4_SAI1_TX_DATA4                            =  IOMUX_PAD(0x0404, 0x019C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD4_SAI6_RX_BCLK                             =  IOMUX_PAD(0x0404, 0x019C, 1, 0x0510, 1, 0),
	IMX8MM_PAD_SAI1_TXD4_SAI6_TX_BCLK                             =  IOMUX_PAD(0x0404, 0x019C, 2, 0x051C, 1, 0),
	IMX8MM_PAD_SAI1_TXD4_ARM_PLATFORM_TRACE12                     =  IOMUX_PAD(0x0404, 0x019C, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD4_GPIO4_IO16                               =  IOMUX_PAD(0x0404, 0x019C, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD4_SRC_BOOT_CFG12                           =  IOMUX_PAD(0x0404, 0x019C, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_TXD5_SAI1_TX_DATA5                            =  IOMUX_PAD(0x0408, 0x01A0, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD5_SAI6_RX_DATA0                            =  IOMUX_PAD(0x0408, 0x01A0, 1, 0x0514, 1, 0),
	IMX8MM_PAD_SAI1_TXD5_SAI6_TX_DATA0                            =  IOMUX_PAD(0x0408, 0x01A0, 2, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD5_ARM_PLATFORM_TRACE13                     =  IOMUX_PAD(0x0408, 0x01A0, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD5_GPIO4_IO17                               =  IOMUX_PAD(0x0408, 0x01A0, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD5_SRC_BOOT_CFG13                           =  IOMUX_PAD(0x0408, 0x01A0, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_TXD6_SAI1_TX_DATA6                            =  IOMUX_PAD(0x040C, 0x01A4, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD6_SAI6_RX_SYNC                             =  IOMUX_PAD(0x040C, 0x01A4, 1, 0x0518, 1, 0),
	IMX8MM_PAD_SAI1_TXD6_SAI6_TX_SYNC                             =  IOMUX_PAD(0x040C, 0x01A4, 2, 0x0520, 1, 0),
	IMX8MM_PAD_SAI1_TXD6_ARM_PLATFORM_TRACE14                     =  IOMUX_PAD(0x040C, 0x01A4, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD6_GPIO4_IO18                               =  IOMUX_PAD(0x040C, 0x01A4, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD6_SRC_BOOT_CFG14                           =  IOMUX_PAD(0x040C, 0x01A4, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_TXD7_SAI1_TX_DATA7                            =  IOMUX_PAD(0x0410, 0x01A8, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD7_SAI6_MCLK                                =  IOMUX_PAD(0x0410, 0x01A8, 1, 0x0530, 1, 0),
	IMX8MM_PAD_SAI1_TXD7_PDM_CLK                                  =  IOMUX_PAD(0x0410, 0x01A8, 3, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD7_ARM_PLATFORM_TRACE15                     =  IOMUX_PAD(0x0410, 0x01A8, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD7_GPIO4_IO19                               =  IOMUX_PAD(0x0410, 0x01A8, 5, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_TXD7_SRC_BOOT_CFG15                           =  IOMUX_PAD(0x0410, 0x01A8, 6, 0x0000, 0, 0),

	IMX8MM_PAD_SAI1_MCLK_SAI1_MCLK                                =  IOMUX_PAD(0x0414, 0x01AC, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_MCLK_SAI5_MCLK                                =  IOMUX_PAD(0x0414, 0x01AC, 1, 0x052C, 1, 0),
	IMX8MM_PAD_SAI1_MCLK_SAI1_TX_BCLK                             =  IOMUX_PAD(0x0414, 0x01AC, 2, 0x04C8, 2, 0),
	IMX8MM_PAD_SAI1_MCLK_PDM_CLK                                  =  IOMUX_PAD(0x0414, 0x01AC, 3, 0x0000, 0, 0),
	IMX8MM_PAD_SAI1_MCLK_GPIO4_IO20                               =  IOMUX_PAD(0x0414, 0x01AC, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI2_RXFS_SAI2_RX_SYNC                             =  IOMUX_PAD(0x0418, 0x01B0, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_RXFS_SAI5_TX_SYNC                             =  IOMUX_PAD(0x0418, 0x01B0, 1, 0x04EC, 2, 0),
	IMX8MM_PAD_SAI2_RXFS_SAI5_TX_DATA1                            =  IOMUX_PAD(0x0418, 0x01B0, 2, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_RXFS_SAI2_RX_DATA1                            =  IOMUX_PAD(0x0418, 0x01B0, 3, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_RXFS_UART1_TX                                 =  IOMUX_PAD(0x0418, 0x01B0, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_RXFS_UART1_RX                                 =  IOMUX_PAD(0x0418, 0x01B0, 4, 0x04F4, 2, 0),
	IMX8MM_PAD_SAI2_RXFS_GPIO4_IO21                               =  IOMUX_PAD(0x0418, 0x01B0, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI2_RXC_SAI2_RX_BCLK                              =  IOMUX_PAD(0x041C, 0x01B4, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_RXC_SAI5_TX_BCLK                              =  IOMUX_PAD(0x041C, 0x01B4, 1, 0x04E8, 2, 0),
	IMX8MM_PAD_SAI2_RXC_UART1_RX                                  =  IOMUX_PAD(0x041C, 0x01B4, 4, 0x04F4, 3, 0),
	IMX8MM_PAD_SAI2_RXC_UART1_TX                                  =  IOMUX_PAD(0x041C, 0x01B4, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_RXC_GPIO4_IO22                                =  IOMUX_PAD(0x041C, 0x01B4, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI2_RXD0_SAI2_RX_DATA0                            =  IOMUX_PAD(0x0420, 0x01B8, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_RXD0_SAI5_TX_DATA0                            =  IOMUX_PAD(0x0420, 0x01B8, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_RXD0_UART1_RTS_B                              =  IOMUX_PAD(0x0420, 0x01B8, 4, 0x04F0, 2, 0),
	IMX8MM_PAD_SAI2_RXD0_UART1_CTS_B                              =  IOMUX_PAD(0x0420, 0x01B8, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_RXD0_GPIO4_IO23                               =  IOMUX_PAD(0x0420, 0x01B8, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI2_TXFS_SAI2_TX_SYNC                             =  IOMUX_PAD(0x0424, 0x01BC, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_TXFS_SAI5_TX_DATA1                            =  IOMUX_PAD(0x0424, 0x01BC, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_TXFS_SAI2_TX_DATA1                            =  IOMUX_PAD(0x0424, 0x01BC, 3, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_TXFS_UART1_CTS_B                              =  IOMUX_PAD(0x0424, 0x01BC, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_TXFS_UART1_RTS_B                              =  IOMUX_PAD(0x0424, 0x01BC, 4, 0x04F0, 3, 0),
	IMX8MM_PAD_SAI2_TXFS_GPIO4_IO24                               =  IOMUX_PAD(0x0424, 0x01BC, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI2_TXC_SAI2_TX_BCLK                              =  IOMUX_PAD(0x0428, 0x01C0, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_TXC_SAI5_TX_DATA2                             =  IOMUX_PAD(0x0428, 0x01C0, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_TXC_GPIO4_IO25                                =  IOMUX_PAD(0x0428, 0x01C0, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI2_TXD0_SAI2_TX_DATA0                            =  IOMUX_PAD(0x042C, 0x01C4, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_TXD0_SAI5_TX_DATA3                            =  IOMUX_PAD(0x042C, 0x01C4, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_TXD0_GPIO4_IO26                               =  IOMUX_PAD(0x042C, 0x01C4, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI2_MCLK_SAI2_MCLK                                =  IOMUX_PAD(0x0430, 0x01C8, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI2_MCLK_SAI5_MCLK                                =  IOMUX_PAD(0x0430, 0x01C8, 1, 0x052C, 2, 0),
	IMX8MM_PAD_SAI2_MCLK_GPIO4_IO27                               =  IOMUX_PAD(0x0430, 0x01C8, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI3_RXFS_SAI3_RX_SYNC                             =  IOMUX_PAD(0x0434, 0x01CC, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_RXFS_GPT1_CAPTURE1                            =  IOMUX_PAD(0x0434, 0x01CC, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_RXFS_SAI5_RX_SYNC                             =  IOMUX_PAD(0x0434, 0x01CC, 2, 0x04E4, 2, 0),
	IMX8MM_PAD_SAI3_RXFS_SAI3_RX_DATA1                            =  IOMUX_PAD(0x0434, 0x01CC, 3, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_RXFS_GPIO4_IO28                               =  IOMUX_PAD(0x0434, 0x01CC, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI3_RXC_SAI3_RX_BCLK                              =  IOMUX_PAD(0x0438, 0x01D0, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_RXC_GPT1_CLK                                  =  IOMUX_PAD(0x0438, 0x01D0, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_RXC_SAI5_RX_BCLK                              =  IOMUX_PAD(0x0438, 0x01D0, 2, 0x04D0, 2, 0),
	IMX8MM_PAD_SAI3_RXC_UART2_CTS_B                               =  IOMUX_PAD(0x0438, 0x01D0, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_RXC_UART2_RTS_B                               =  IOMUX_PAD(0x0438, 0x01D0, 4, 0x04F8, 2, 0),
	IMX8MM_PAD_SAI3_RXC_GPIO4_IO29                                =  IOMUX_PAD(0x0438, 0x01D0, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI3_RXD_SAI3_RX_DATA0                             =  IOMUX_PAD(0x043C, 0x01D4, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_RXD_GPT1_COMPARE1                             =  IOMUX_PAD(0x043C, 0x01D4, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_RXD_SAI5_RX_DATA0                             =  IOMUX_PAD(0x043C, 0x01D4, 2, 0x04D4, 2, 0),
	IMX8MM_PAD_SAI3_RXD_UART2_RTS_B                               =  IOMUX_PAD(0x043C, 0x01D4, 4, 0x04F8, 3, 0),
	IMX8MM_PAD_SAI3_RXD_UART2_CTS_B                               =  IOMUX_PAD(0x043C, 0x01D4, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_RXD_GPIO4_IO30                                =  IOMUX_PAD(0x043C, 0x01D4, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI3_TXFS_SAI3_TX_SYNC                             =  IOMUX_PAD(0x0440, 0x01D8, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_TXFS_GPT1_CAPTURE2                            =  IOMUX_PAD(0x0440, 0x01D8, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_TXFS_SAI5_RX_DATA1                            =  IOMUX_PAD(0x0440, 0x01D8, 2, 0x04D8, 2, 0),
	IMX8MM_PAD_SAI3_TXFS_SAI3_TX_DATA1                            =  IOMUX_PAD(0x0440, 0x01D8, 3, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_TXFS_UART2_RX                                 =  IOMUX_PAD(0x0440, 0x01D8, 4, 0x04FC, 2, 0),
	IMX8MM_PAD_SAI3_TXFS_UART2_TX                                 =  IOMUX_PAD(0x0440, 0x01D8, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_TXFS_GPIO4_IO31                               =  IOMUX_PAD(0x0440, 0x01D8, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI3_TXC_SAI3_TX_BCLK                              =  IOMUX_PAD(0x0444, 0x01DC, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_TXC_GPT1_COMPARE2                             =  IOMUX_PAD(0x0444, 0x01DC, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_TXC_SAI5_RX_DATA2                             =  IOMUX_PAD(0x0444, 0x01DC, 2, 0x04DC, 2, 0),
	IMX8MM_PAD_SAI3_TXC_UART2_TX                                  =  IOMUX_PAD(0x0444, 0x01DC, 4, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_TXC_UART2_RX                                  =  IOMUX_PAD(0x0444, 0x01DC, 4, 0x04FC, 3, 0),
	IMX8MM_PAD_SAI3_TXC_GPIO5_IO0                                 =  IOMUX_PAD(0x0444, 0x01DC, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI3_TXD_SAI3_TX_DATA0                             =  IOMUX_PAD(0x0448, 0x01E0, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_TXD_GPT1_COMPARE3                             =  IOMUX_PAD(0x0448, 0x01E0, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_TXD_SAI5_RX_DATA3                             =  IOMUX_PAD(0x0448, 0x01E0, 2, 0x04E0, 2, 0),
	IMX8MM_PAD_SAI3_TXD_GPIO5_IO1                                 =  IOMUX_PAD(0x0448, 0x01E0, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SAI3_MCLK_SAI3_MCLK                                =  IOMUX_PAD(0x044C, 0x01E4, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_MCLK_PWM4_OUT                                 =  IOMUX_PAD(0x044C, 0x01E4, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SAI3_MCLK_SAI5_MCLK                                =  IOMUX_PAD(0x044C, 0x01E4, 2, 0x052C, 3, 0),
	IMX8MM_PAD_SAI3_MCLK_GPIO5_IO2                                =  IOMUX_PAD(0x044C, 0x01E4, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SPDIF_TX_SPDIF1_OUT                                =  IOMUX_PAD(0x0450, 0x01E8, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SPDIF_TX_PWM3_OUT                                  =  IOMUX_PAD(0x0450, 0x01E8, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SPDIF_TX_GPIO5_IO3                                 =  IOMUX_PAD(0x0450, 0x01E8, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SPDIF_RX_SPDIF1_IN                                 =  IOMUX_PAD(0x0454, 0x01EC, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SPDIF_RX_PWM2_OUT                                  =  IOMUX_PAD(0x0454, 0x01EC, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SPDIF_RX_GPIO5_IO4                                 =  IOMUX_PAD(0x0454, 0x01EC, 5, 0x0000, 0, 0),

	IMX8MM_PAD_SPDIF_EXT_CLK_SPDIF1_EXT_CLK                       =  IOMUX_PAD(0x0458, 0x01F0, 0, 0x0000, 0, 0),
	IMX8MM_PAD_SPDIF_EXT_CLK_PWM1_OUT                             =  IOMUX_PAD(0x0458, 0x01F0, 1, 0x0000, 0, 0),
	IMX8MM_PAD_SPDIF_EXT_CLK_GPIO5_IO5                            =  IOMUX_PAD(0x0458, 0x01F0, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ECSPI1_SCLK_ECSPI1_SCLK                            =  IOMUX_PAD(0x045C, 0x01F4, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI1_SCLK_UART3_RX                               =  IOMUX_PAD(0x045C, 0x01F4, 1, 0x0504, 0, 0),
	IMX8MM_PAD_ECSPI1_SCLK_UART3_TX                               =  IOMUX_PAD(0x045C, 0x01F4, 1, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI1_SCLK_GPIO5_IO6                              =  IOMUX_PAD(0x045C, 0x01F4, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ECSPI1_MOSI_ECSPI1_MOSI                            =  IOMUX_PAD(0x0460, 0x01F8, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI1_MOSI_UART3_TX                               =  IOMUX_PAD(0x0460, 0x01F8, 1, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI1_MOSI_UART3_RX                               =  IOMUX_PAD(0x0460, 0x01F8, 1, 0x0504, 1, 0),
	IMX8MM_PAD_ECSPI1_MOSI_GPIO5_IO7                              =  IOMUX_PAD(0x0460, 0x01F8, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ECSPI1_MISO_ECSPI1_MISO                            =  IOMUX_PAD(0x0464, 0x01FC, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI1_MISO_UART3_CTS_B                            =  IOMUX_PAD(0x0464, 0x01FC, 1, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI1_MISO_UART3_RTS_B                            =  IOMUX_PAD(0x0464, 0x01FC, 1, 0x0500, 0, 0),
	IMX8MM_PAD_ECSPI1_MISO_GPIO5_IO8                              =  IOMUX_PAD(0x0464, 0x01FC, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ECSPI1_SS0_ECSPI1_SS0                              =  IOMUX_PAD(0x0468, 0x0200, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI1_SS0_UART3_RTS_B                             =  IOMUX_PAD(0x0468, 0x0200, 1, 0x0500, 1, 0),
	IMX8MM_PAD_ECSPI1_SS0_UART3_CTS_B                             =  IOMUX_PAD(0x0468, 0x0200, 1, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI1_SS0_GPIO5_IO9                               =  IOMUX_PAD(0x0468, 0x0200, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ECSPI2_SCLK_ECSPI2_SCLK                            =  IOMUX_PAD(0x046C, 0x0204, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI2_SCLK_UART4_RX                               =  IOMUX_PAD(0x046C, 0x0204, 1, 0x050C, 0, 0),
	IMX8MM_PAD_ECSPI2_SCLK_UART4_TX                               =  IOMUX_PAD(0x046C, 0x0204, 1, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI2_SCLK_GPIO5_IO10                             =  IOMUX_PAD(0x046C, 0x0204, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ECSPI2_MOSI_ECSPI2_MOSI                            =  IOMUX_PAD(0x0470, 0x0208, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI2_MOSI_UART4_TX                               =  IOMUX_PAD(0x0470, 0x0208, 1, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI2_MOSI_UART4_RX                               =  IOMUX_PAD(0x0470, 0x0208, 1, 0x050C, 1, 0),
	IMX8MM_PAD_ECSPI2_MOSI_GPIO5_IO11                             =  IOMUX_PAD(0x0470, 0x0208, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ECSPI2_MISO_ECSPI2_MISO                            =  IOMUX_PAD(0x0474, 0x020C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI2_MISO_UART4_CTS_B                            =  IOMUX_PAD(0x0474, 0x020C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI2_MISO_UART4_RTS_B                            =  IOMUX_PAD(0x0474, 0x020C, 1, 0x0508, 0, 0),
	IMX8MM_PAD_ECSPI2_MISO_GPIO5_IO12                             =  IOMUX_PAD(0x0474, 0x020C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_ECSPI2_SS0_ECSPI2_SS0                              =  IOMUX_PAD(0x0478, 0x0210, 0, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI2_SS0_UART4_RTS_B                             =  IOMUX_PAD(0x0478, 0x0210, 1, 0x0508, 1, 0),
	IMX8MM_PAD_ECSPI2_SS0_UART4_CTS_B                             =  IOMUX_PAD(0x0478, 0x0210, 1, 0x0000, 0, 0),
	IMX8MM_PAD_ECSPI2_SS0_GPIO5_IO13                              =  IOMUX_PAD(0x0478, 0x0210, 5, 0x0000, 0, 0),

	IMX8MM_PAD_I2C1_SCL_I2C1_SCL                                  =  IOMUX_PAD(0x047C, 0x0214, 0 | IOMUX_CONFIG_SION, 0x0000, 0, 0),
	IMX8MM_PAD_I2C1_SCL_ENET1_MDC                                 =  IOMUX_PAD(0x047C, 0x0214, 1, 0x0000, 0, 0),
	IMX8MM_PAD_I2C1_SCL_GPIO5_IO14                                =  IOMUX_PAD(0x047C, 0x0214, 5, 0x0000, 0, 0),

	IMX8MM_PAD_I2C1_SDA_I2C1_SDA                                  =  IOMUX_PAD(0x0480, 0x0218, 0 | IOMUX_CONFIG_SION, 0x0000, 0, 0),
	IMX8MM_PAD_I2C1_SDA_ENET1_MDIO                                =  IOMUX_PAD(0x0480, 0x0218, 1, 0x04C0, 2, 0),
	IMX8MM_PAD_I2C1_SDA_GPIO5_IO15                                =  IOMUX_PAD(0x0480, 0x0218, 5, 0x0000, 0, 0),

	IMX8MM_PAD_I2C2_SCL_I2C2_SCL                                  =  IOMUX_PAD(0x0484, 0x021C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_I2C2_SCL_ENET1_1588_EVENT1_IN                      =  IOMUX_PAD(0x0484, 0x021C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_I2C2_SCL_USDHC3_CD_B                               =  IOMUX_PAD(0x0484, 0x021C, 2, 0x0544, 1, 0),
	IMX8MM_PAD_I2C2_SCL_GPIO5_IO16                                =  IOMUX_PAD(0x0484, 0x021C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_I2C2_SDA_I2C2_SDA                                  =  IOMUX_PAD(0x0488, 0x0220, 0, 0x0000, 0, 0),
	IMX8MM_PAD_I2C2_SDA_ENET1_1588_EVENT1_OUT                     =  IOMUX_PAD(0x0488, 0x0220, 1, 0x0000, 0, 0),
	IMX8MM_PAD_I2C2_SDA_USDHC3_WP                                 =  IOMUX_PAD(0x0488, 0x0220, 2, 0x0548, 1, 0),
	IMX8MM_PAD_I2C2_SDA_GPIO5_IO17                                =  IOMUX_PAD(0x0488, 0x0220, 5, 0x0000, 0, 0),

	IMX8MM_PAD_I2C3_SCL_I2C3_SCL                                  =  IOMUX_PAD(0x048C, 0x0224, 0, 0x0000, 0, 0),
	IMX8MM_PAD_I2C3_SCL_PWM4_OUT                                  =  IOMUX_PAD(0x048C, 0x0224, 1, 0x0000, 0, 0),
	IMX8MM_PAD_I2C3_SCL_GPT2_CLK                                  =  IOMUX_PAD(0x048C, 0x0224, 2, 0x0000, 0, 0),
	IMX8MM_PAD_I2C3_SCL_GPIO5_IO18                                =  IOMUX_PAD(0x048C, 0x0224, 5, 0x0000, 0, 0),

	IMX8MM_PAD_I2C3_SDA_I2C3_SDA                                  =  IOMUX_PAD(0x0490, 0x0228, 0, 0x0000, 0, 0),
	IMX8MM_PAD_I2C3_SDA_PWM3_OUT                                  =  IOMUX_PAD(0x0490, 0x0228, 1, 0x0000, 0, 0),
	IMX8MM_PAD_I2C3_SDA_GPT3_CLK                                  =  IOMUX_PAD(0x0490, 0x0228, 2, 0x0000, 0, 0),
	IMX8MM_PAD_I2C3_SDA_GPIO5_IO19                                =  IOMUX_PAD(0x0490, 0x0228, 5, 0x0000, 0, 0),

	IMX8MM_PAD_I2C4_SCL_I2C4_SCL                                  =  IOMUX_PAD(0x0494, 0x022C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_I2C4_SCL_PWM2_OUT                                  =  IOMUX_PAD(0x0494, 0x022C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_I2C4_SCL_PCIE1_CLKREQ_B                            =  IOMUX_PAD(0x0494, 0x022C, 2, 0x0524, 0, 0),
	IMX8MM_PAD_I2C4_SCL_GPIO5_IO20                                =  IOMUX_PAD(0x0494, 0x022C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_I2C4_SDA_I2C4_SDA                                  =  IOMUX_PAD(0x0498, 0x0230, 0, 0x0000, 0, 0),
	IMX8MM_PAD_I2C4_SDA_PWM1_OUT                                  =  IOMUX_PAD(0x0498, 0x0230, 1, 0x0000, 0, 0),
	IMX8MM_PAD_I2C4_SDA_GPIO5_IO21                                =  IOMUX_PAD(0x0498, 0x0230, 5, 0x0000, 0, 0),

	IMX8MM_PAD_UART1_RXD_UART1_RX                                 =  IOMUX_PAD(0x049C, 0x0234, 0, 0x04F4, 0, 0),
	IMX8MM_PAD_UART1_RXD_UART1_TX                                 =  IOMUX_PAD(0x049C, 0x0234, 0, 0x0000, 0, 0),
	IMX8MM_PAD_UART1_RXD_ECSPI3_SCLK                              =  IOMUX_PAD(0x049C, 0x0234, 1, 0x0000, 0, 0),
	IMX8MM_PAD_UART1_RXD_GPIO5_IO22                               =  IOMUX_PAD(0x049C, 0x0234, 5, 0x0000, 0, 0),

	IMX8MM_PAD_UART1_TXD_UART1_TX                                 =  IOMUX_PAD(0x04A0, 0x0238, 0, 0x0000, 0, 0),
	IMX8MM_PAD_UART1_TXD_UART1_RX                                 =  IOMUX_PAD(0x04A0, 0x0238, 0, 0x04F4, 1, 0),
	IMX8MM_PAD_UART1_TXD_ECSPI3_MOSI                              =  IOMUX_PAD(0x04A0, 0x0238, 1, 0x0000, 0, 0),
	IMX8MM_PAD_UART1_TXD_GPIO5_IO23                               =  IOMUX_PAD(0x04A0, 0x0238, 5, 0x0000, 0, 0),

	IMX8MM_PAD_UART2_RXD_UART2_RX                                 =  IOMUX_PAD(0x04A4, 0x023C, 0, 0x04FC, 0, 0),
	IMX8MM_PAD_UART2_RXD_UART2_TX                                 =  IOMUX_PAD(0x04A4, 0x023C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_UART2_RXD_ECSPI3_MISO                              =  IOMUX_PAD(0x04A4, 0x023C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_UART2_RXD_GPIO5_IO24                               =  IOMUX_PAD(0x04A4, 0x023C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_UART2_TXD_UART2_TX                                 =  IOMUX_PAD(0x04A8, 0x0240, 0, 0x0000, 0, 0),
	IMX8MM_PAD_UART2_TXD_UART2_RX                                 =  IOMUX_PAD(0x04A8, 0x0240, 0, 0x04FC, 1, 0),
	IMX8MM_PAD_UART2_TXD_ECSPI3_SS0                               =  IOMUX_PAD(0x04A8, 0x0240, 1, 0x0000, 0, 0),
	IMX8MM_PAD_UART2_TXD_GPIO5_IO25                               =  IOMUX_PAD(0x04A8, 0x0240, 5, 0x0000, 0, 0),

	IMX8MM_PAD_UART3_RXD_UART3_RX                                 =  IOMUX_PAD(0x04AC, 0x0244, 0, 0x0504, 2, 0),
	IMX8MM_PAD_UART3_RXD_UART3_TX                                 =  IOMUX_PAD(0x04AC, 0x0244, 0, 0x0000, 0, 0),
	IMX8MM_PAD_UART3_RXD_UART1_CTS_B                              =  IOMUX_PAD(0x04AC, 0x0244, 1, 0x0000, 0, 0),
	IMX8MM_PAD_UART3_RXD_UART1_RTS_B                              =  IOMUX_PAD(0x04AC, 0x0244, 1, 0x04F0, 0, 0),
	IMX8MM_PAD_UART3_RXD_USDHC3_RESET_B                           =  IOMUX_PAD(0x04AC, 0x0244, 2, 0x0000, 0, 0),
	IMX8MM_PAD_UART3_RXD_GPIO5_IO26                               =  IOMUX_PAD(0x04AC, 0x0244, 5, 0x0000, 0, 0),

	IMX8MM_PAD_UART3_TXD_UART3_TX                                 =  IOMUX_PAD(0x04B0, 0x0248, 0, 0x0000, 0, 0),
	IMX8MM_PAD_UART3_TXD_UART3_RX                                 =  IOMUX_PAD(0x04B0, 0x0248, 0, 0x0504, 3, 0),
	IMX8MM_PAD_UART3_TXD_UART1_RTS_B                              =  IOMUX_PAD(0x04B0, 0x0248, 1, 0x04F0, 1, 0),
	IMX8MM_PAD_UART3_TXD_UART1_CTS_B                              =  IOMUX_PAD(0x04B0, 0x0248, 1, 0x0000, 0, 0),
	IMX8MM_PAD_UART3_TXD_USDHC3_VSELECT                           =  IOMUX_PAD(0x04B0, 0x0248, 2, 0x0000, 0, 0),
	IMX8MM_PAD_UART3_TXD_GPIO5_IO27                               =  IOMUX_PAD(0x04B0, 0x0248, 5, 0x0000, 0, 0),

	IMX8MM_PAD_UART4_RXD_UART4_RX                                 =  IOMUX_PAD(0x04B4, 0x024C, 0, 0x050C, 2, 0),
	IMX8MM_PAD_UART4_RXD_UART4_TX                                 =  IOMUX_PAD(0x04B4, 0x024C, 0, 0x0000, 0, 0),
	IMX8MM_PAD_UART4_RXD_UART2_CTS_B                              =  IOMUX_PAD(0x04B4, 0x024C, 1, 0x0000, 0, 0),
	IMX8MM_PAD_UART4_RXD_UART2_RTS_B                              =  IOMUX_PAD(0x04B4, 0x024C, 1, 0x04F8, 0, 0),
	IMX8MM_PAD_UART4_RXD_PCIE1_CLKREQ_B                           =  IOMUX_PAD(0x04B4, 0x024C, 2, 0x0524, 1, 0),
	IMX8MM_PAD_UART4_RXD_GPIO5_IO28                               =  IOMUX_PAD(0x04B4, 0x024C, 5, 0x0000, 0, 0),

	IMX8MM_PAD_UART4_TXD_UART4_TX                                 =  IOMUX_PAD(0x04B8, 0x0250, 0, 0x0000, 0, 0),
	IMX8MM_PAD_UART4_TXD_UART4_RX                                 =  IOMUX_PAD(0x04B8, 0x0250, 0, 0x050C, 3, 0),
	IMX8MM_PAD_UART4_TXD_UART2_RTS_B                              =  IOMUX_PAD(0x04B8, 0x0250, 1, 0x04F8, 1, 0),
	IMX8MM_PAD_UART4_TXD_UART2_CTS_B                              =  IOMUX_PAD(0x04B8, 0x0250, 1, 0x0000, 0, 0),
	IMX8MM_PAD_UART4_TXD_GPIO5_IO29                               =  IOMUX_PAD(0x04B8, 0x0250, 5, 0x0000, 0, 0),
};
#endif
