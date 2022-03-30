/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K2G EVM: Pinmux configuration
 *
 * (C) Copyright 2015
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mux-k2g.h>
#include <asm/arch/hardware.h>
#include "board.h"

struct pin_cfg k2g_generic_pin_cfg[] = {
	/* UART0 */
	{ 115,  MODE(0) },	/* SOC_UART0_RXD */
	{ 116,  MODE(0) },	/* SOC_UART0_TXD */

	/* I2C 0 */
	{ 223,  MODE(0) },	/* SOC_I2C0_SCL */
	{ 224,  MODE(0) },	/* SOC_I2C0_SDA */

	/* I2C 1 */
	{ 225,  MODE(0) },	/* SOC_I2C1_SCL */
	{ 226,  MODE(0) },	/* SOC_I2C1_SDA */
	{ MAX_PIN_N, }
};

struct pin_cfg k2g_evm_pin_cfg[] = {
	/* GPMC */
	{ 0,	MODE(0) },	/* GPMCAD0 */
	{ 1,	MODE(0) },	/* GPMCAD1 */
	{ 2,	MODE(0) },	/* GPMCAD2 */
	{ 3,	MODE(0) },	/* GPMCAD3 */
	{ 4,	MODE(0) },	/* GPMCAD4 */
	{ 5,	MODE(0) },	/* GPMCAD5 */
	{ 6,	MODE(0) },	/* GPMCAD6 */
	{ 7,	MODE(0) },	/* GPMCAD7 */
	{ 8,	MODE(0) },	/* GPMCAD8 */
	{ 9,	MODE(0) },	/* GPMCAD9 */
	{ 10,	MODE(0) },	/* GPMCAD10 */
	{ 11,	MODE(0) },	/* GPMCAD11 */
	{ 12,	MODE(0) },	/* GPMCAD12 */
	{ 13,	MODE(0) },	/* GPMCAD13 */
	{ 14,	MODE(0) },	/* GPMCAD14 */
	{ 15,	MODE(0) },	/* GPMCAD15 */
	{ 17,	MODE(0) },	/* GPMCADVNALE */
	{ 18,	MODE(0) },	/* GPMCOENREN */
	{ 19,	MODE(0) },	/* GPMCWEN */
	{ 20,	MODE(0) },	/* GPMCBE0NCLE */
	{ 22,	MODE(0) },	/* GPMCWAIT0 */
	{ 24,	MODE(0) },	/* GPMCWPN */
	{ 26,	MODE(0) },	/* GPMCCSN0 */

	/* GPIOs */
	{ 16,	MODE(3) | PIN_IEN },	/* GPIO0_16 - PRSNT1# */
	{ 21,	MODE(3) | PIN_IEN },	/* GPIO0_21 - DC_BRD_DET */
	{ 82,	MODE(3) | PIN_IEN },	/* GPIO0_82 - TPS_INT1 */
	{ 83,	MODE(3) },		/* GPIO0_83 - TPS_SLEEP */
	{ 84,	MODE(3) },		/* GPIO0_84 - SEL_HDMIn_GPIO */
	{ 87,	MODE(3) },		/* GPIO0_87 - SD_LP2996A */
	{ 106,	MODE(3) | PIN_IEN},	/* GPIO0_100 - SOC_INT */
	{ 201,	MODE(3) | PIN_IEN},	/* GPIO1_26 - GPIO_EXP_INT */
	{ 202,	MODE(3) },		/* GPIO1_27 - SEL_LCDn_GPIO */
	{ 203,	MODE(3) | PIN_IEN},	/* GPIO1_28 - SOC_MLB_GPIO2 */
	{ 204,	MODE(3) | PIN_IEN},	/* GPIO1_29 - SOC_PCIE_WAKEn */
	{ 205,	MODE(3) | PIN_IEN},	/* GPIO1_30 - BMC_INT1 */
	{ 206,	MODE(3) | PIN_IEN},	/* GPIO1_31 - HDMI_INTn*/
	{ 207,	MODE(3) | PIN_IEN},	/* GPIO1_32 - CS2000_AUX_OUT */
	{ 208,	MODE(3) | PIN_IEN},	/* GPIO1_33 - TEMP_INT */
	{ 209,	MODE(3) | PIN_IEN},	/* GPIO1_34 - WLAN_IRQ */
	{ 216,	MODE(3) },		/* GPIO1_41 - FLASH_HOLD */
	{ 217,	MODE(3) | PIN_IEN},	/* GPIO1_42 - TOUCH_INTn */

	/* MLB */
	{ 23,	MODE(2) },	/* SOC_MLBCLK */
	{ 25,	MODE(2) },	/* SOC_MLBSIG */
	{ 27,	MODE(2) },	/* SOC_MLBDAT */

	/* DSS */
	{ 30,	MODE(0) },	/* SOC_DSSDATA23 */
	{ 31,	MODE(0) },	/* SOC_DSSDATA22 */
	{ 32,	MODE(0) },	/* SOC_DSSDATA21 */
	{ 33,	MODE(0) },	/* SOC_DSSDATA20 */
	{ 34,	MODE(0) },	/* SOC_DSSDATA19 */
	{ 35,	MODE(0) },	/* SOC_DSSDATA18 */
	{ 36,	MODE(0) },	/* SOC_DSSDATA17 */
	{ 37,	MODE(0) },	/* SOC_DSSDATA16 */
	{ 38,	MODE(0) },	/* SOC_DSSDATA15 */
	{ 39,	MODE(0) },	/* SOC_DSSDATA14 */
	{ 40,	MODE(0) },	/* SOC_DSSDATA13 */
	{ 41,	MODE(0) },	/* SOC_DSSDATA12 */
	{ 42,	MODE(0) },	/* SOC_DSSDATA11 */
	{ 43,	MODE(0) },	/* SOC_DSSDATA10 */
	{ 44,	MODE(0) },	/* SOC_DSSDATA9 */
	{ 45,	MODE(0) },	/* SOC_DSSDATA8 */
	{ 46,	MODE(0) },	/* SOC_DSSDATA7 */
	{ 47,	MODE(0) },	/* SOC_DSSDATA6 */
	{ 48,	MODE(0) },	/* SOC_DSSDATA5 */
	{ 49,	MODE(0) },	/* SOC_DSSDATA4 */
	{ 50,	MODE(0) },	/* SOC_DSSDATA3 */
	{ 51,	MODE(0) },	/* SOC_DSSDATA2 */
	{ 52,	MODE(0) },	/* SOC_DSSDATA1 */
	{ 53,	MODE(0) },	/* SOC_DSSDATA0 */
	{ 54,	MODE(0) },	/* SOC_DSSVSYNC */
	{ 55,	MODE(0) },	/* SOC_DSSHSYNC */
	{ 56,	MODE(0) },	/* SOC_DSSPCLK */
	{ 57,	MODE(0) },	/* SOC_DSS_DE */
	{ 58,	MODE(0) },	/* SOC_DSS_FID */
	{ 221,	MODE(4) },	/* PWM0 - SOC_BACKLIGHT_PWM */

	/* MMC1 */
	{ 59,	MODE(0) },	/* SOC_MMC1_DAT7 */
	{ 60,	MODE(0) },	/* SOC_MMC1_DAT6 */
	{ 61,	MODE(0) },	/* SOC_MMC1_DAT5 */
	{ 62,	MODE(0) },	/* SOC_MMC1_DAT4 */
	{ 63,	MODE(0) },	/* SOC_MMC1_DAT3 */
	{ 64,	MODE(0) },	/* SOC_MMC1_DAT2 */
	{ 65,	MODE(0) },	/* SOC_MMC1_DAT1 */
	{ 66,	MODE(0) },	/* SOC_MMC1_DAT0 */
	{ 67,	MODE(0) },	/* SOC_MMC1_CLK */
	{ 68,	MODE(0) },	/* SOC_MMC1_CMD */
	{ 69,	MODE(0) },	/* MMC1SDCD TP125 */
	{ 70,	MODE(0) },	/* SOC_MMC1_SDWP */
	{ 71,	MODE(0) },	/* MMC1POW TP124 */

		/* EMAC */
	{ 72,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_RXC */
	{ 77,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_RXD3 */
	{ 78,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_RXD2 */
	{ 79,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_RXD1 */
	{ 80,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_RXD0 */
	{ 81,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_RXCTL */
	{ 85,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_TXC */
	{ 91,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_TXD3 */
	{ 92,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_TXD2 */
	{ 93,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_TXD1 */
	{ 94,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_TXD0 */
	{ 95,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_TXCTL */

	/* MDIO */
	{ 98,	BUFFER_CLASS_B | PIN_PDIS | MODE(0) },	/* MDIO_DATA */
	{ 99,	BUFFER_CLASS_B | PIN_PDIS | MODE(0) },	/* MDIO_CLK */

	/* PWM */
	{ 73,	MODE(4) },	/* SOC_EHRPWM3A */
	{ 74,	MODE(4) },	/* SOC_EHRPWM3B */
	{ 75,	MODE(4) },	/* SOC_EHRPWM3_SYNCI */
	{ 76,	MODE(4) },	/* SOC_EHRPWM3_SYNCO */
	{ 96,	MODE(4) },	/* SOC_EHRPWM_TRIPZONE_INPUT3 */
	{ 198,	MODE(4) },	/* SOC_EHRPWM_TRIPZONE_INPUT4 */
	{ 199,	MODE(4) },	/* SOC_EHRPWM4A */
	{ 200,	MODE(4) },	/* SOC_EHRPWM4B */
	{ 218,	MODE(4) },	/* SOC_EHRPWM_TRIPZONE_INPUT5 */
	{ 219,	MODE(4) },	/* SOC_EHRPWM5A */
	{ 220,	MODE(4) },	/* SOC_EHRPWM5B */
	{ 222,	MODE(4) },	/* SOC_ECAP1_IN_PWM1_OUT */

	/* SPI3 */
	{ 86,	MODE(1) },	/* SOC_SPI3_SCS0 */
	{ 88,	MODE(1) },	/* SOC_SPI3_CLK */
	{ 89,	MODE(1) },	/* SOC_SPI3_MISO */
	{ 90,	MODE(1) },	/* SOC_SPI3_MOSI */

	/* CLK */
	{ 97,	MODE(0) },	/* SMD - TP132 */

	/* SPI0 */
	{ 100,	MODE(0) },	/* SOC_SPI0_SCS0 */
	{ 101,	MODE(0) },	/* SOC_SPI0_SCS1 */
	{ 102,	MODE(0) },	/* SOC_SPI0_CLK */
	{ 103,	MODE(0) },	/* SOC_SPI0_MISO */
	{ 104,	MODE(0) },	/* SOC_SPI0_MOSI */

	/* SPI1 NORFLASH */
	{ 105,	MODE(0) },	/* SOC_SPI1_SCS0 */
	{ 107,	MODE(0) },	/* SOC_SPI1_CLK */
	{ 108,	MODE(0) },	/* SOC_SPI1_MISO */
	{ 109,	MODE(0) },	/* SOC_SPI1_MOSI */

	/* SPI2 */
	{ 110,	MODE(0) },	/* SOC_SPI2_SCS0 */
	{ 111,	MODE(1) },	/* SOC_HOUT */
	{ 112,	MODE(0) },	/* SOC_SPI2_CLK */
	{ 113,	MODE(0) },	/* SOC_SPI2_MISO */
	{ 114,	MODE(0) },	/* SOC_SPI2_MOSI */

	/* UART0 */
	{ 115,	MODE(0) },	/* SOC_UART0_RXD */
	{ 116,	MODE(0) },	/* SOC_UART0_TXD */
	{ 117,	MODE(0) },	/* SOC_UART0_CTSn */
	{ 118,	MODE(0) },	/* SOC_UART0_RTSn */

	/* UART1 */
	{ 119,	MODE(0) },	/* SOC_UART1_RXD */
	{ 120,	MODE(0) },	/* SOC_UART1_TXD */
	{ 121,	MODE(0) },	/* SOC_UART1_CTSn */
	{ 122,	MODE(0) },	/* SOC_UART1_RTSn */

	/* UART2 */
	{ 123,	MODE(0) },	/* SOC_UART2_RXD */
	{ 124,	MODE(0) },	/* SOC_UART2_TXD */
	{ 125,	MODE(0) },	/* UART0_TXVR_EN */
	{ 126,	MODE(4) },	/* SOC_CPTS_TS_COMP */

	/* DCAN */
	{ 127,	MODE(0) },	/* SOC_DCAN0_TX */
	{ 128,	MODE(0) },	/* SOC_DCAN0_RX */
	{ 137,	MODE(1) },	/* SOC_DCAN1_TX */
	{ 138,	MODE(1) },	/* SOC_DCAN1_RX */

	/* QSPI */
	{ 129,	MODE(0) },	/* SOC_QSPI_CLK */
	{ 130,	MODE(0) },	/* SOC_QSPI_RTCLK */
	{ 131,	MODE(0) },	/* SOC_QSPI_D0 */
	{ 132,	MODE(0) },	/* SOC_QSPI_D1 */
	{ 133,	MODE(0) },	/* SOC_QSPI_D2 */
	{ 134,	MODE(0) },	/* SOC_QSPI_D3 */
	{ 135,	MODE(0) },	/* SOC_QSPI_CSN0 */
	{ 136,	MODE(1) },	/* DNI <-> WLAN_SLOW_CLK */

	/* MCASP2 */
	{ 139,	MODE(3) },	/* SOC_MCASP2AXR0 - (GPIO0_108)SOC_LED0 */
	{ 140,	MODE(4) },	/* SOC_MCASP2AXR1 */
	{ 141,	MODE(4) },	/* SOC_MCASP2AXR2 */
	{ 142,	MODE(4) },	/* SOC_MCASP2AXR3 */
	{ 143,	MODE(4) },	/* SOC_MCASP2AXR4 */
	{ 144,	MODE(4) },	/* SOC_MCASP2AXR5 */
	{ 145,	MODE(4) },	/* SOC_McASP2ACLKR */
	{ 146,	MODE(4) },	/* SOC_McASP2FSR */
	{ 147,	MODE(4) },	/* SOC_McASP2AHCLKR */
	{ 148,	MODE(3) },	/* GPIO0_117 - WLAN_TRANS_EN */
	{ 149,	MODE(4) },	/* SOC_McASP2FSX */
	{ 150,	MODE(4) },	/* SOC_McASP2AHCLKX */
	{ 151,	MODE(4) },	/* SOC_McASP2ACLKX */

	/* MCASP1 */
	{ 152,	MODE(4) },	/* SOC_MCASP1ACLKR */
	{ 153,	MODE(4) },	/* SOC_MCASP1FSR */
	{ 154,	MODE(4) },	/* SOC_MCASP1AHCLKR */
	{ 155,	MODE(4) },	/* SOC_MCASP1ACLKX */
	{ 156,	MODE(4) },	/* SOC_MCASP1FSX */
	{ 157,	MODE(4) },	/* SOC_MCASP1AHCLKX */
	{ 158,	MODE(4) },	/* SOC_MCASP1AMUTE */
	{ 159,	MODE(4) },	/* SOC_MCASP1AXR0 */
	{ 160,	MODE(4) },	/* SOC_MCASP1AXR1 */
	{ 161,	MODE(4) },	/* SOC_MCASP1AXR2 */
	{ 162,	MODE(4) },	/* SOC_MCASP1AXR3 */
	{ 163,	MODE(4) },	/* SOC_MCASP1AXR4 */
	{ 164,	MODE(4) },	/* SOC_MCASP1AXR5 */
	{ 165,	MODE(4) },	/* SOC_MCASP1AXR6 */
	{ 166,	MODE(4) },	/* SOC_MCASP1AXR7 */
	{ 167,	MODE(4) },	/* SOC_MCASP1AXR8 */
	{ 168,	MODE(4) },	/* SOC_MCASP1AXR9 */

	/* MCASP0 */
	{ 169,	MODE(4) },	/* SOC_MCASP0AMUTE */
	{ 170,	MODE(4) },	/* SOC_MCASP0ACLKR */
	{ 171,	MODE(4) },	/* SOC_MCASP0FSR */
	{ 172,	MODE(4) },	/* SOC_MCASP0AHCLKR */
	{ 173,	MODE(4) },	/* SOC_MCASP0ACLKX */
	{ 174,	MODE(4) },	/* SOC_MCASP0FSX */
	{ 175,	MODE(4) },	/* SOC_MCASP0AHCLKX */
	{ 176,	MODE(4) },	/* SOC_MCASP0AXR0 */
	{ 177,	MODE(4) },	/* SOC_MCASP0AXR1 */
	{ 178,	MODE(4) },	/* SOC_MCASP0AXR2 */
	{ 179,	MODE(4) },	/* SOC_MCASP0AXR3 */
	{ 180,	MODE(4) },	/* SOC_MCASP0AXR4 */
	{ 181,	MODE(4) },	/* SOC_MCASP0AXR5 */
	{ 182,	MODE(4) },	/* SOC_MCASP0AXR6 */
	{ 183,	MODE(4) },	/* SOC_MCASP0AXR7 */
	{ 184,	MODE(4) },	/* SOC_MCASP0AXR8 */
	{ 185,	MODE(4) },	/* SOC_MCASP0AXR9 */
	{ 186,	MODE(3) },	/* SOC_MCASP0AXR10 - (GPIO1_11)SOC_LED1 */
	{ 188,	MODE(4) },	/* SOC_MCASP0AXR12 */
	{ 189,	MODE(4) },	/* SOC_MCASP0AXR13 */
	{ 190,	MODE(4) },	/* SOC_MCASP0AXR14 */
	{ 191,	MODE(4) },	/* SOC_MCASP0AXR15 */

	/* MMC0 */
	{ 192,	MODE(2) },	/* SOC_MMC0_DAT3 */
	{ 193,	MODE(2) },	/* SOC_MMC0_DAT2 */
	{ 194,	MODE(2) },	/* SOC_MMC0_DAT1 */
	{ 195,	MODE(2) },	/* SOC_MMC0_DAT0 */
	{ 196,	MODE(2) },	/* SOC_MMC0_CLK */
	{ 197,	MODE(2) },	/* SOC_MMC0_CMD */
	{ 187,	MODE(2) },	/* SOC_MMC0_SDCD */

	/* McBSP */
	{ 28,	MODE(2) | PIN_IEN },	/* SOC_TIMI1 */
	{ 29,	MODE(2) },		/* SOC_TIMO1 */
	{ 210,	MODE(2) },	/* SOC_MCBSPDR */
	{ 211,	MODE(2) },	/* SOC_MCBSPDX */
	{ 212,	MODE(2) },	/* SOC_MCBSPFSX */
	{ 213,	MODE(2) },	/* SOC_MCBSPCLKX */
	{ 214,	MODE(2) },	/* SOC_MCBSPFSR */
	{ 215,	MODE(2) },	/* SOC_MCBSPCLKR */

	/* I2C */
	{ 223,	MODE(0) },	/* SOC_I2C0_SCL */
	{ 224,	MODE(0) },	/* SOC_I2C0_SDA */
	{ 225,	MODE(0) },	/* SOC_I2C1_SCL */
	{ 226,	MODE(0) },	/* SOC_I2C1_SDA */
	{ 227,	MODE(0) },	/* SOC_I2C2_SCL */
	{ 228,	MODE(0) },	/* SOC_I2C2_SDA */
	{ 229,	MODE(0) },	/* NMIz */
	{ 230,	MODE(0) },	/* LRESETz */
	{ 231,	MODE(0) },	/* LRESETNMIENz */

	{ 235,	MODE(0) },
	{ 236,	MODE(0) },
	{ 237,	MODE(0) },
	{ 238,	MODE(0) },
	{ 239,	MODE(0) },
	{ 240,	MODE(0) },
	{ 241,	MODE(0) },
	{ 242,	MODE(0) },
	{ 243,	MODE(0) },
	{ 244,	MODE(0) },

	{ 258,	MODE(0) },	/* USB0DRVVBUS */
	{ 259,	MODE(0) },	/* USB1DRVVBUS */
	{ MAX_PIN_N, }
};

struct pin_cfg k2g_ice_evm_pin_cfg[] = {
	/* MMC 1 */
	{ 63, MODE(0) | PIN_PTD },	/* MMC1_DAT3.MMC1_DAT3 */
	{ 64, MODE(0) | PIN_PTU },	/* MMC1_DAT2.MMC1_DAT2 */
	{ 65, MODE(0) | PIN_PTU },	/* MMC1_DAT1.MMC1_DAT1 */
	{ 66, MODE(0) | PIN_PTD },	/* MMC1_DAT0.MMC1_DAT0 */
	{ 67, MODE(0) | PIN_PTD },	/* MMC1_CLK.MMC1_CLK   */
	{ 68, MODE(0) | PIN_PTD },	/* MMC1_CMD.MMC1_CMD   */
	{ 69, MODE(3) | PIN_PTU },	/* MMC1_SDCD.GPIO0_69  */
	{ 70, MODE(0) | PIN_PTU },	/* MMC1_SDWP.MMC1_SDWP */
	{ 71, MODE(0) | PIN_PTD },	/* MMC1_POW.MMC1_POW   */

	/* I2C 0 */
	{ 223,  MODE(0) },		/* SOC_I2C0_SCL */
	{ 224,  MODE(0) },		/* SOC_I2C0_SDA */

	/* QSPI */
	{ 129,	MODE(0) },	/* SOC_QSPI_CLK */
	{ 130,	MODE(0) },	/* SOC_QSPI_RTCLK */
	{ 131,	MODE(0) },	/* SOC_QSPI_D0 */
	{ 132,	MODE(0) },	/* SOC_QSPI_D1 */
	{ 133,	MODE(0) },	/* SOC_QSPI_D2 */
	{ 134,	MODE(0) },	/* SOC_QSPI_D3 */
	{ 135,	MODE(0) },	/* SOC_QSPI_CSN0 */

	/* EMAC */
	{ 72,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_RXC */
	{ 77,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_RXD3 */
	{ 78,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_RXD2 */
	{ 79,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_RXD1 */
	{ 80,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_RXD0 */
	{ 81,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_RXCTL */
	{ 85,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_TXC */
	{ 91,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_TXD3 */
	{ 92,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_TXD2 */
	{ 93,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_TXD1 */
	{ 94,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_TXD0 */
	{ 95,	BUFFER_CLASS_D | PIN_PDIS | MODE(1) },	/* RGMII_TXCTL */

	/* MDIO */
	{ 98,	BUFFER_CLASS_B | PIN_PDIS | MODE(0) },	/* MDIO_DATA */
	{ 99,	BUFFER_CLASS_B | PIN_PDIS | MODE(0) },	/* MDIO_CLK */

	{ MAX_PIN_N, }
};

void k2g_mux_config(void)
{
	if (!board_ti_was_eeprom_read()) {
		configure_pin_mux(k2g_generic_pin_cfg);
	} else if (board_is_k2g_gp() || board_is_k2g_g1()) {
		configure_pin_mux(k2g_evm_pin_cfg);
	} else if (board_is_k2g_ice()) {
		configure_pin_mux(k2g_ice_evm_pin_cfg);
	} else {
		puts("Unknown board, cannot configure pinmux.");
		hang();
	}
}
