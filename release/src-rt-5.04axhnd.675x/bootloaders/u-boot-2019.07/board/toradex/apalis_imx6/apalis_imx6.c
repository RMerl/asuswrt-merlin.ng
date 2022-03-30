// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
 * Copyright (C) 2014-2019, Toradex AG
 * copied from nitrogen6x
 */

#include <common.h>
#include <dm.h>

#include <ahci.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/sys_proto.h>
#include <asm/bootm.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/sata.h>
#include <asm/mach-imx/video.h>
#include <dm/device-internal.h>
#include <dm/platform_data/serial_mxc.h>
#include <dwc_ahsata.h>
#include <environment.h>
#include <fsl_esdhc.h>
#include <imx_thermal.h>
#include <micrel.h>
#include <miiphy.h>
#include <netdev.h>

#include "../common/tdx-cfg-block.h"
#ifdef CONFIG_TDX_CMD_IMX_MFGR
#include "pf0100.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_EMMC_PAD_CTRL (PAD_CTL_PUS_47K_UP |		\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define WEAK_PULLUP	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_SRE_SLOW)

#define WEAK_PULLDOWN	(PAD_CTL_PUS_100K_DOWN |		\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_SLOW)

#define TRISTATE	(PAD_CTL_HYS | PAD_CTL_SPEED_MED)

#define OUTPUT_RGB (PAD_CTL_SPEED_MED|PAD_CTL_DSE_60ohm|PAD_CTL_SRE_FAST)

#define APALIS_IMX6_SATA_INIT_RETRIES	10

int dram_init(void)
{
	/* use the DDR controllers configured size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    (ulong)imx_ddr_size());

	return 0;
}

/* Apalis UART1 */
iomux_v3_cfg_t const uart1_pads_dce[] = {
	MX6_PAD_CSI0_DAT10__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT11__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};
iomux_v3_cfg_t const uart1_pads_dte[] = {
	MX6_PAD_CSI0_DAT10__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT11__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#if defined(CONFIG_FSL_ESDHC) && defined(CONFIG_SPL_BUILD)
/* Apalis MMC1 */
iomux_v3_cfg_t const usdhc1_pads[] = {
	MX6_PAD_SD1_CLK__SD1_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_CMD__SD1_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT0__SD1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT1__SD1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT2__SD1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT3__SD1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NANDF_D0__SD1_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NANDF_D1__SD1_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NANDF_D2__SD1_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NANDF_D3__SD1_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_DI0_PIN4__GPIO4_IO20   | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
#	define GPIO_MMC_CD IMX_GPIO_NR(4, 20)
};

/* Apalis SD1 */
iomux_v3_cfg_t const usdhc2_pads[] = {
	MX6_PAD_SD2_CLK__SD2_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_CMD__SD2_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT0__SD2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT1__SD2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT2__SD2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT3__SD2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NANDF_CS1__GPIO6_IO14  | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
#	define GPIO_SD_CD IMX_GPIO_NR(6, 14)
};

/* eMMC */
iomux_v3_cfg_t const usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__SD3_CLK    | MUX_PAD_CTRL(USDHC_EMMC_PAD_CTRL),
	MX6_PAD_SD3_CMD__SD3_CMD    | MUX_PAD_CTRL(USDHC_EMMC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_EMMC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_EMMC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_EMMC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_EMMC_PAD_CTRL),
	MX6_PAD_SD3_DAT4__SD3_DATA4 | MUX_PAD_CTRL(USDHC_EMMC_PAD_CTRL),
	MX6_PAD_SD3_DAT5__SD3_DATA5 | MUX_PAD_CTRL(USDHC_EMMC_PAD_CTRL),
	MX6_PAD_SD3_DAT6__SD3_DATA6 | MUX_PAD_CTRL(USDHC_EMMC_PAD_CTRL),
	MX6_PAD_SD3_DAT7__SD3_DATA7 | MUX_PAD_CTRL(USDHC_EMMC_PAD_CTRL),
	MX6_PAD_SD3_RST__GPIO7_IO08 | MUX_PAD_CTRL(WEAK_PULLUP) | MUX_MODE_SION,
};
#endif /* CONFIG_FSL_ESDHC & CONFIG_SPL_BUILD */

int mx6_rgmii_rework(struct phy_device *phydev)
{
	/* control data pad skew - devaddr = 0x02, register = 0x04 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* rx data pad skew - devaddr = 0x02, register = 0x05 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* tx data pad skew - devaddr = 0x02, register = 0x05 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* gtx and rx clock pad skew - devaddr = 0x02, register = 0x08 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_CLOCK_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x03FF);
	return 0;
}

iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__RGMII_TXC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__RGMII_TD0		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__RGMII_TD1		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__RGMII_TD2		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__RGMII_TD3		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RXC__RGMII_RXC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD0__RGMII_RD0		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD1__RGMII_RD1		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD2__RGMII_RD2		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD3__RGMII_RD3		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	/* KSZ9031 PHY Reset */
	MX6_PAD_ENET_CRS_DV__GPIO1_IO25		| MUX_PAD_CTRL(NO_PAD_CTRL) |
						  MUX_MODE_SION,
#	define GPIO_ENET_PHY_RESET IMX_GPIO_NR(1, 25)
};

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));
}

static int reset_enet_phy(struct mii_dev *bus)
{
	/* Reset KSZ9031 PHY */
	gpio_request(GPIO_ENET_PHY_RESET, "ETH_RESET#");
	gpio_direction_output(GPIO_ENET_PHY_RESET, 0);
	mdelay(10);
	gpio_set_value(GPIO_ENET_PHY_RESET, 1);

	return 0;
}

/* mux the Apalis GPIO pins, so they can be used from the U-Boot cmdline */
iomux_v3_cfg_t const gpio_pads[] = {
	/* Apalis GPIO1 - GPIO8 */
	MX6_PAD_NANDF_D4__GPIO2_IO04	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_D5__GPIO2_IO05	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_D6__GPIO2_IO06	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_D7__GPIO2_IO07	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_RB0__GPIO6_IO10	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_WP_B__GPIO6_IO09	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_GPIO_2__GPIO1_IO02	| MUX_PAD_CTRL(WEAK_PULLDOWN) |
					  MUX_MODE_SION,
	MX6_PAD_GPIO_6__GPIO1_IO06	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_GPIO_4__GPIO1_IO04	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
};

static void setup_iomux_gpio(void)
{
	imx_iomux_v3_setup_multiple_pads(gpio_pads, ARRAY_SIZE(gpio_pads));
}

iomux_v3_cfg_t const usb_pads[] = {
	/* USBH_EN */
	MX6_PAD_GPIO_0__GPIO1_IO00 | MUX_PAD_CTRL(NO_PAD_CTRL) | MUX_MODE_SION,
#	define GPIO_USBH_EN IMX_GPIO_NR(1, 0)
	/* USB_VBUS_DET */
	MX6_PAD_EIM_D28__GPIO3_IO28 | MUX_PAD_CTRL(NO_PAD_CTRL),
#	define GPIO_USB_VBUS_DET IMX_GPIO_NR(3, 28)
	/* USBO1_ID */
	MX6_PAD_ENET_RX_ER__USB_OTG_ID	| MUX_PAD_CTRL(WEAK_PULLUP),
	/* USBO1_EN */
	MX6_PAD_EIM_D22__GPIO3_IO22 | MUX_PAD_CTRL(NO_PAD_CTRL) | MUX_MODE_SION,
#	define GPIO_USBO_EN IMX_GPIO_NR(3, 22)
};

/*
 * UARTs are used in DTE mode, switch the mode on all UARTs before
 * any pinmuxing connects a (DCE) output to a transceiver output.
 */
#define UCR3		0x88	/* FIFO Control Register */
#define UCR3_RI		BIT(8)	/* RIDELT DTE mode */
#define UCR3_DCD	BIT(9)	/* DCDDELT DTE mode */
#define UFCR		0x90	/* FIFO Control Register */
#define UFCR_DCEDTE	BIT(6)	/* DCE=0 */

static void setup_dtemode_uart(void)
{
	setbits_le32((u32 *)(UART1_BASE + UFCR), UFCR_DCEDTE);
	setbits_le32((u32 *)(UART2_BASE + UFCR), UFCR_DCEDTE);
	setbits_le32((u32 *)(UART4_BASE + UFCR), UFCR_DCEDTE);
	setbits_le32((u32 *)(UART5_BASE + UFCR), UFCR_DCEDTE);

	clrbits_le32((u32 *)(UART1_BASE + UCR3), UCR3_DCD | UCR3_RI);
	clrbits_le32((u32 *)(UART2_BASE + UCR3), UCR3_DCD | UCR3_RI);
	clrbits_le32((u32 *)(UART4_BASE + UCR3), UCR3_DCD | UCR3_RI);
	clrbits_le32((u32 *)(UART5_BASE + UCR3), UCR3_DCD | UCR3_RI);
}
static void setup_dcemode_uart(void)
{
	clrbits_le32((u32 *)(UART1_BASE + UFCR), UFCR_DCEDTE);
	clrbits_le32((u32 *)(UART2_BASE + UFCR), UFCR_DCEDTE);
	clrbits_le32((u32 *)(UART4_BASE + UFCR), UFCR_DCEDTE);
	clrbits_le32((u32 *)(UART5_BASE + UFCR), UFCR_DCEDTE);
}

static void setup_iomux_dte_uart(void)
{
	setup_dtemode_uart();
	imx_iomux_v3_setup_multiple_pads(uart1_pads_dte,
					 ARRAY_SIZE(uart1_pads_dte));
}
static void setup_iomux_dce_uart(void)
{
	setup_dcemode_uart();
	imx_iomux_v3_setup_multiple_pads(uart1_pads_dce,
					 ARRAY_SIZE(uart1_pads_dce));
}

#ifdef CONFIG_USB_EHCI_MX6
int board_ehci_hcd_init(int port)
{
	imx_iomux_v3_setup_multiple_pads(usb_pads, ARRAY_SIZE(usb_pads));
	return 0;
}
#endif

#if defined(CONFIG_FSL_ESDHC) && defined(CONFIG_SPL_BUILD)
/* use the following sequence: eMMC, MMC1, SD1 */
struct fsl_esdhc_cfg usdhc_cfg[CONFIG_SYS_FSL_USDHC_NUM] = {
	{USDHC3_BASE_ADDR},
	{USDHC1_BASE_ADDR},
	{USDHC2_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = true; /* default: assume inserted */

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		gpio_request(GPIO_MMC_CD, "MMC_CD");
		gpio_direction_input(GPIO_MMC_CD);
		ret = !gpio_get_value(GPIO_MMC_CD);
		break;
	case USDHC2_BASE_ADDR:
		gpio_request(GPIO_MMC_CD, "SD_CD");
		gpio_direction_input(GPIO_SD_CD);
		ret = !gpio_get_value(GPIO_SD_CD);
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	unsigned reg = readl(&psrc->sbmr1) >> 11;
	/*
	 * Upon reading BOOT_CFG register the following map is done:
	 * Bit 11 and 12 of BOOT_CFG register can determine the current
	 * mmc port
	 * 0x1                  SD1
	 * 0x2                  SD2
	 * 0x3                  SD4
	 */

	switch (reg & 0x3) {
	case 0x0:
		imx_iomux_v3_setup_multiple_pads(
			usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
		usdhc_cfg[0].esdhc_base = USDHC1_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	case 0x1:
		imx_iomux_v3_setup_multiple_pads(
			usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
		usdhc_cfg[0].esdhc_base = USDHC2_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	case 0x2:
		imx_iomux_v3_setup_multiple_pads(
			usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
		usdhc_cfg[0].esdhc_base = USDHC3_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	default:
		puts("MMC boot device not available");
	}

	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
}
#endif /* CONFIG_FSL_ESDHC & CONFIG_SPL_BUILD */

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	uint32_t base = IMX_FEC_BASE;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int ret;

	setup_iomux_enet();

#ifdef CONFIG_FEC_MXC
	bus = fec_get_miibus(base, -1);
	if (!bus)
		return 0;

	bus->reset = reset_enet_phy;
	/* scan PHY 4,5,6,7 */
	phydev = phy_find_by_mask(bus, (0xf << 4), PHY_INTERFACE_MODE_RGMII);
	if (!phydev) {
		free(bus);
		puts("no PHY found\n");
		return 0;
	}

	printf("using PHY at %d\n", phydev->addr);
	ret = fec_probe(bis, -1, base, bus, phydev);
	if (ret) {
		printf("FEC MXC: %s:failed\n", __func__);
		free(phydev);
		free(bus);
	}
#endif /* CONFIG_FEC_MXC */

	return 0;
}

static iomux_v3_cfg_t const pwr_intb_pads[] = {
	/*
	 * the bootrom sets the iomux to vselect, potentially connecting
	 * two outputs. Set this back to GPIO
	 */
	MX6_PAD_GPIO_18__GPIO7_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL)
};

#if defined(CONFIG_VIDEO_IPUV3)

static iomux_v3_cfg_t const backlight_pads[] = {
	/* Backlight on RGB connector: J15 */
	MX6_PAD_EIM_DA13__GPIO3_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL) |
				       MUX_MODE_SION,
#define RGB_BACKLIGHT_GP IMX_GPIO_NR(3, 13)
	/* additional CPU pin on BKL_PWM, keep in tristate */
	MX6_PAD_EIM_DA14__GPIO3_IO14 | MUX_PAD_CTRL(TRISTATE),
	/* Backlight PWM, used as GPIO in U-Boot */
	MX6_PAD_SD4_DAT2__GPIO2_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL) |
				       MUX_MODE_SION,
#define RGB_BACKLIGHTPWM_GP IMX_GPIO_NR(2, 10)
	/* buffer output enable 0: buffer enabled */
	MX6_PAD_EIM_A25__GPIO5_IO02 | MUX_PAD_CTRL(WEAK_PULLUP) | MUX_MODE_SION,
#define RGB_BACKLIGHTPWM_OE IMX_GPIO_NR(5, 2)
	/* PSAVE# integrated VDAC */
	MX6_PAD_EIM_BCLK__GPIO6_IO31 | MUX_PAD_CTRL(NO_PAD_CTRL) |
				       MUX_MODE_SION,
#define VGA_PSAVE_NOT_GP IMX_GPIO_NR(6, 31)
};

static iomux_v3_cfg_t const rgb_pads[] = {
	MX6_PAD_EIM_A16__IPU1_DI1_DISP_CLK | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_DA10__IPU1_DI1_PIN15 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_DA11__IPU1_DI1_PIN02 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_DA12__IPU1_DI1_PIN03 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_DA9__IPU1_DISP1_DATA00 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_DA8__IPU1_DISP1_DATA01 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_DA7__IPU1_DISP1_DATA02 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_DA6__IPU1_DISP1_DATA03 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_DA5__IPU1_DISP1_DATA04 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_DA4__IPU1_DISP1_DATA05 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_DA3__IPU1_DISP1_DATA06 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_DA2__IPU1_DISP1_DATA07 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_DA1__IPU1_DISP1_DATA08 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_DA0__IPU1_DISP1_DATA09 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_EB1__IPU1_DISP1_DATA10 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_EB0__IPU1_DISP1_DATA11 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_A17__IPU1_DISP1_DATA12 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_A18__IPU1_DISP1_DATA13 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_A19__IPU1_DISP1_DATA14 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_A20__IPU1_DISP1_DATA15 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_A21__IPU1_DISP1_DATA16 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_A22__IPU1_DISP1_DATA17 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_A23__IPU1_DISP1_DATA18 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_A24__IPU1_DISP1_DATA19 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_D26__IPU1_DISP1_DATA22 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_D27__IPU1_DISP1_DATA23 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_D30__IPU1_DISP1_DATA21 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_EIM_D31__IPU1_DISP1_DATA20 | MUX_PAD_CTRL(OUTPUT_RGB),
};

static void do_enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

static void enable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)
				IOMUXC_BASE_ADDR;
	u32 reg = readl(&iomux->gpr[2]);
	reg |= IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT;
	writel(reg, &iomux->gpr[2]);
	gpio_direction_output(RGB_BACKLIGHT_GP, 1);
	gpio_direction_output(RGB_BACKLIGHTPWM_GP, 0);
	gpio_direction_output(RGB_BACKLIGHTPWM_OE, 0);
}

static void enable_rgb(struct display_info_t const *dev)
{
	imx_iomux_v3_setup_multiple_pads(
		rgb_pads,
		ARRAY_SIZE(rgb_pads));
	gpio_direction_output(RGB_BACKLIGHT_GP, 1);
	gpio_direction_output(RGB_BACKLIGHTPWM_GP, 0);
	gpio_direction_output(RGB_BACKLIGHTPWM_OE, 0);
}

static int detect_default(struct display_info_t const *dev)
{
	(void) dev;
	return 1;
}

struct display_info_t const displays[] = {{
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= detect_hdmi,
	.enable	= do_enable_hdmi,
	.mode	= {
		.name           = "HDMI",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= -1,
	.addr	= 0,
	.di	= 1,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= detect_default,
	.enable	= enable_rgb,
	.mode	= {
		.name           = "vga-rgb",
		.refresh        = 60,
		.xres           = 640,
		.yres           = 480,
		.pixclock       = 33000,
		.left_margin    = 48,
		.right_margin   = 16,
		.upper_margin   = 31,
		.lower_margin   = 11,
		.hsync_len      = 96,
		.vsync_len      = 2,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= -1,
	.addr	= 0,
	.di	= 1,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.enable	= enable_rgb,
	.mode	= {
		.name           = "wvga-rgb",
		.refresh        = 60,
		.xres           = 800,
		.yres           = 480,
		.pixclock       = 25000,
		.left_margin    = 40,
		.right_margin   = 88,
		.upper_margin   = 33,
		.lower_margin   = 10,
		.hsync_len      = 128,
		.vsync_len      = 2,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_LVDS666,
	.enable	= enable_lvds,
	.mode	= {
		.name           = "wsvga-lvds",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 600,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} } };
size_t display_count = ARRAY_SIZE(displays);

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	enable_ipu_clock();
	imx_setup_hdmi();
	/* Turn on LDB0,IPU,IPU DI0 clocks */
	reg = __raw_readl(&mxc_ccm->CCGR3);
	reg |= MXC_CCM_CCGR3_LDB_DI0_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	/* set LDB0, LDB1 clk select to 011/011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 |MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3<<MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
	      |(3<<MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<<MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	     |IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH
	     |IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
	     |IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
	     |IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
	     |IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	     |IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
	     |IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED
	     |IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~(IOMUXC_GPR3_LVDS0_MUX_CTL_MASK
			|IOMUXC_GPR3_HDMI_MUX_CTL_MASK))
	    | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
	       <<IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);

	/* backlight unconditionally on for now */
	imx_iomux_v3_setup_multiple_pads(backlight_pads,
					 ARRAY_SIZE(backlight_pads));
	/* use 0 for EDT 7", use 1 for LG fullHD panel */
	gpio_request(RGB_BACKLIGHTPWM_GP, "BKL1_PWM");
	gpio_request(RGB_BACKLIGHTPWM_OE, "BKL1_PWM_EN");
	gpio_request(RGB_BACKLIGHT_GP, "BKL1_ON");
	gpio_direction_output(RGB_BACKLIGHTPWM_GP, 0);
	gpio_direction_output(RGB_BACKLIGHTPWM_OE, 0);
	gpio_direction_output(RGB_BACKLIGHT_GP, 1);
}

/*
 * Backlight off before OS handover
 */
void board_preboot_os(void)
{
	gpio_direction_output(RGB_BACKLIGHTPWM_GP, 1);
	gpio_direction_output(RGB_BACKLIGHT_GP, 0);
}
#endif /* defined(CONFIG_VIDEO_IPUV3) */

int board_early_init_f(void)
{
	imx_iomux_v3_setup_multiple_pads(pwr_intb_pads,
					 ARRAY_SIZE(pwr_intb_pads));
#ifndef CONFIG_TDX_APALIS_IMX6_V1_0
	setup_iomux_dte_uart();
#else
	setup_iomux_dce_uart();
#endif
	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#if defined(CONFIG_VIDEO_IPUV3)
	setup_display();
#endif

#ifdef CONFIG_TDX_CMD_IMX_MFGR
	(void) pmic_init();
#endif

#ifdef CONFIG_SATA
	setup_sata();
#endif

	setup_iomux_gpio();

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#if defined(CONFIG_REVISION_TAG) && \
    defined(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)
	char env_str[256];
	u32 rev;

	rev = get_board_rev();
	snprintf(env_str, ARRAY_SIZE(env_str), "%.4x", rev);
	env_set("board_rev", env_str);

#ifndef CONFIG_TDX_APALIS_IMX6_V1_0
	if ((rev & 0xfff0) == 0x0100) {
		char *fdt_env;

		/* reconfigure the UART to DCE mode dynamically if on V1.0 HW */
		setup_iomux_dce_uart();

		/* if using the default device tree, use version for V1.0 HW */
		fdt_env = env_get("fdt_file");
		if ((fdt_env != NULL) && (strcmp(FDT_FILE, fdt_env) == 0)) {
			env_set("fdt_file", FDT_FILE_V1_0);
			printf("patching fdt_file to " FDT_FILE_V1_0 "\n");
#ifndef CONFIG_ENV_IS_NOWHERE
			env_save();
#endif
		}
	}
#endif /* CONFIG_TDX_APALIS_IMX6_V1_0 */
#endif /* CONFIG_REVISION_TAG */

#ifdef CONFIG_CMD_USB_SDP
	if (is_boot_from_usb()) {
		printf("Serial Downloader recovery mode, using sdp command\n");
		env_set("bootdelay", "0");
		env_set("bootcmd", "sdp 0");
	}
#endif /* CONFIG_CMD_USB_SDP */

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */

int checkboard(void)
{
	char it[] = " IT";
	int minc, maxc;

	switch (get_cpu_temp_grade(&minc, &maxc)) {
	case TEMP_AUTOMOTIVE:
	case TEMP_INDUSTRIAL:
		break;
	case TEMP_EXTCOMMERCIAL:
	default:
		it[0] = 0;
	};
	printf("Model: Toradex Apalis iMX6 %s %s%s\n",
	       is_cpu_type(MXC_CPU_MX6D) ? "Dual" : "Quad",
	       (gd->ram_size == 0x80000000) ? "2GB" :
	       (gd->ram_size == 0x40000000) ? "1GB" : "512MB", it);
	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	return ft_common_board_setup(blob, bd);
}
#endif

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4-bit bus width */
	{"mmc",	MAKE_CFGVAL(0x40, 0x20, 0x00, 0x00)},
	{"sd",	MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	{NULL,	0},
};
#endif

int misc_init_r(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif
	return 0;
}

#ifdef CONFIG_LDO_BYPASS_CHECK
/* TODO, use external pmic, for now always ldo_enable */
void ldo_mode_set(int ldo_bypass)
{
	return;
}
#endif

#ifdef CONFIG_SPL_BUILD
#include <spl.h>
#include <linux/libfdt.h>
#include "asm/arch/mx6q-ddr.h"
#include "asm/arch/iomux.h"
#include "asm/arch/crm_regs.h"

static int mx6_com_dcd_table[] = {
/* ddr-setup.cfg */
MX6_IOM_DRAM_SDQS0, 0x00000030,
MX6_IOM_DRAM_SDQS1, 0x00000030,
MX6_IOM_DRAM_SDQS2, 0x00000030,
MX6_IOM_DRAM_SDQS3, 0x00000030,
MX6_IOM_DRAM_SDQS4, 0x00000030,
MX6_IOM_DRAM_SDQS5, 0x00000030,
MX6_IOM_DRAM_SDQS6, 0x00000030,
MX6_IOM_DRAM_SDQS7, 0x00000030,

MX6_IOM_GRP_B0DS, 0x00000030,
MX6_IOM_GRP_B1DS, 0x00000030,
MX6_IOM_GRP_B2DS, 0x00000030,
MX6_IOM_GRP_B3DS, 0x00000030,
MX6_IOM_GRP_B4DS, 0x00000030,
MX6_IOM_GRP_B5DS, 0x00000030,
MX6_IOM_GRP_B6DS, 0x00000030,
MX6_IOM_GRP_B7DS, 0x00000030,
MX6_IOM_GRP_ADDDS, 0x00000030,
/* 40 Ohm drive strength for cs0/1,sdba2,cke0/1,sdwe */
MX6_IOM_GRP_CTLDS, 0x00000030,

MX6_IOM_DRAM_DQM0, 0x00020030,
MX6_IOM_DRAM_DQM1, 0x00020030,
MX6_IOM_DRAM_DQM2, 0x00020030,
MX6_IOM_DRAM_DQM3, 0x00020030,
MX6_IOM_DRAM_DQM4, 0x00020030,
MX6_IOM_DRAM_DQM5, 0x00020030,
MX6_IOM_DRAM_DQM6, 0x00020030,
MX6_IOM_DRAM_DQM7, 0x00020030,

MX6_IOM_DRAM_CAS, 0x00020030,
MX6_IOM_DRAM_RAS, 0x00020030,
MX6_IOM_DRAM_SDCLK_0, 0x00020030,
MX6_IOM_DRAM_SDCLK_1, 0x00020030,

MX6_IOM_DRAM_RESET, 0x00020030,
MX6_IOM_DRAM_SDCKE0, 0x00003000,
MX6_IOM_DRAM_SDCKE1, 0x00003000,

MX6_IOM_DRAM_SDODT0, 0x00003030,
MX6_IOM_DRAM_SDODT1, 0x00003030,

/* (differential input) */
MX6_IOM_DDRMODE_CTL, 0x00020000,
/* (differential input) */
MX6_IOM_GRP_DDRMODE, 0x00020000,
/* disable ddr pullups */
MX6_IOM_GRP_DDRPKE, 0x00000000,
MX6_IOM_DRAM_SDBA2, 0x00000000,
/* 40 Ohm drive strength for cs0/1,sdba2,cke0/1,sdwe */
MX6_IOM_GRP_DDR_TYPE, 0x000C0000,

/* Read data DQ Byte0-3 delay */
MX6_MMDC_P0_MPRDDQBY0DL, 0x33333333,
MX6_MMDC_P0_MPRDDQBY1DL, 0x33333333,
MX6_MMDC_P0_MPRDDQBY2DL, 0x33333333,
MX6_MMDC_P0_MPRDDQBY3DL, 0x33333333,
MX6_MMDC_P1_MPRDDQBY0DL, 0x33333333,
MX6_MMDC_P1_MPRDDQBY1DL, 0x33333333,
MX6_MMDC_P1_MPRDDQBY2DL, 0x33333333,
MX6_MMDC_P1_MPRDDQBY3DL, 0x33333333,

/*
 * MDMISC	mirroring	interleaved (row/bank/col)
 */
MX6_MMDC_P0_MDMISC, 0x00081740,

/*
 * MDSCR	con_req
 */
MX6_MMDC_P0_MDSCR, 0x00008000,

/* 1066mhz_4x128mx16.cfg */

MX6_MMDC_P0_MDPDC, 0x00020036,
MX6_MMDC_P0_MDCFG0, 0x555A7954,
MX6_MMDC_P0_MDCFG1, 0xDB328F64,
MX6_MMDC_P0_MDCFG2, 0x01FF00DB,
MX6_MMDC_P0_MDRWD, 0x000026D2,
MX6_MMDC_P0_MDOR, 0x005A1023,
MX6_MMDC_P0_MDOTC, 0x09555050,
MX6_MMDC_P0_MDPDC, 0x00025576,
MX6_MMDC_P0_MDASP, 0x00000027,
MX6_MMDC_P0_MDCTL, 0x831A0000,
MX6_MMDC_P0_MDSCR, 0x04088032,
MX6_MMDC_P0_MDSCR, 0x00008033,
MX6_MMDC_P0_MDSCR, 0x00428031,
MX6_MMDC_P0_MDSCR, 0x19308030,
MX6_MMDC_P0_MDSCR, 0x04008040,
MX6_MMDC_P0_MPZQHWCTRL, 0xA1390003,
MX6_MMDC_P1_MPZQHWCTRL, 0xA1390003,
MX6_MMDC_P0_MDREF, 0x00005800,
MX6_MMDC_P0_MPODTCTRL, 0x00000000,
MX6_MMDC_P1_MPODTCTRL, 0x00000000,

MX6_MMDC_P0_MPDGCTRL0, 0x432A0338,
MX6_MMDC_P0_MPDGCTRL1, 0x03260324,
MX6_MMDC_P1_MPDGCTRL0, 0x43340344,
MX6_MMDC_P1_MPDGCTRL1, 0x031E027C,

MX6_MMDC_P0_MPRDDLCTL, 0x33272D2E,
MX6_MMDC_P1_MPRDDLCTL, 0x2F312B37,

MX6_MMDC_P0_MPWRDLCTL, 0x3A35433C,
MX6_MMDC_P1_MPWRDLCTL, 0x4336453F,

MX6_MMDC_P0_MPWLDECTRL0, 0x0009000E,
MX6_MMDC_P0_MPWLDECTRL1, 0x0018000B,
MX6_MMDC_P1_MPWLDECTRL0, 0x00060015,
MX6_MMDC_P1_MPWLDECTRL1, 0x0006000E,

MX6_MMDC_P0_MPMUR0, 0x00000800,
MX6_MMDC_P1_MPMUR0, 0x00000800,
MX6_MMDC_P0_MDSCR, 0x00000000,
MX6_MMDC_P0_MAPSR, 0x00011006,
};

static int mx6_it_dcd_table[] = {
/* ddr-setup.cfg */
MX6_IOM_DRAM_SDQS0, 0x00000030,
MX6_IOM_DRAM_SDQS1, 0x00000030,
MX6_IOM_DRAM_SDQS2, 0x00000030,
MX6_IOM_DRAM_SDQS3, 0x00000030,
MX6_IOM_DRAM_SDQS4, 0x00000030,
MX6_IOM_DRAM_SDQS5, 0x00000030,
MX6_IOM_DRAM_SDQS6, 0x00000030,
MX6_IOM_DRAM_SDQS7, 0x00000030,

MX6_IOM_GRP_B0DS, 0x00000030,
MX6_IOM_GRP_B1DS, 0x00000030,
MX6_IOM_GRP_B2DS, 0x00000030,
MX6_IOM_GRP_B3DS, 0x00000030,
MX6_IOM_GRP_B4DS, 0x00000030,
MX6_IOM_GRP_B5DS, 0x00000030,
MX6_IOM_GRP_B6DS, 0x00000030,
MX6_IOM_GRP_B7DS, 0x00000030,
MX6_IOM_GRP_ADDDS, 0x00000030,
/* 40 Ohm drive strength for cs0/1,sdba2,cke0/1,sdwe */
MX6_IOM_GRP_CTLDS, 0x00000030,

MX6_IOM_DRAM_DQM0, 0x00020030,
MX6_IOM_DRAM_DQM1, 0x00020030,
MX6_IOM_DRAM_DQM2, 0x00020030,
MX6_IOM_DRAM_DQM3, 0x00020030,
MX6_IOM_DRAM_DQM4, 0x00020030,
MX6_IOM_DRAM_DQM5, 0x00020030,
MX6_IOM_DRAM_DQM6, 0x00020030,
MX6_IOM_DRAM_DQM7, 0x00020030,

MX6_IOM_DRAM_CAS, 0x00020030,
MX6_IOM_DRAM_RAS, 0x00020030,
MX6_IOM_DRAM_SDCLK_0, 0x00020030,
MX6_IOM_DRAM_SDCLK_1, 0x00020030,

MX6_IOM_DRAM_RESET, 0x00020030,
MX6_IOM_DRAM_SDCKE0, 0x00003000,
MX6_IOM_DRAM_SDCKE1, 0x00003000,

MX6_IOM_DRAM_SDODT0, 0x00003030,
MX6_IOM_DRAM_SDODT1, 0x00003030,

/* (differential input) */
MX6_IOM_DDRMODE_CTL, 0x00020000,
/* (differential input) */
MX6_IOM_GRP_DDRMODE, 0x00020000,
/* disable ddr pullups */
MX6_IOM_GRP_DDRPKE, 0x00000000,
MX6_IOM_DRAM_SDBA2, 0x00000000,
/* 40 Ohm drive strength for cs0/1,sdba2,cke0/1,sdwe */
MX6_IOM_GRP_DDR_TYPE, 0x000C0000,

/* Read data DQ Byte0-3 delay */
MX6_MMDC_P0_MPRDDQBY0DL, 0x33333333,
MX6_MMDC_P0_MPRDDQBY1DL, 0x33333333,
MX6_MMDC_P0_MPRDDQBY2DL, 0x33333333,
MX6_MMDC_P0_MPRDDQBY3DL, 0x33333333,
MX6_MMDC_P1_MPRDDQBY0DL, 0x33333333,
MX6_MMDC_P1_MPRDDQBY1DL, 0x33333333,
MX6_MMDC_P1_MPRDDQBY2DL, 0x33333333,
MX6_MMDC_P1_MPRDDQBY3DL, 0x33333333,

/*
 * MDMISC	mirroring	interleaved (row/bank/col)
 */
MX6_MMDC_P0_MDMISC, 0x00081740,

/*
 * MDSCR	con_req
 */
MX6_MMDC_P0_MDSCR, 0x00008000,

/* 1066mhz_4x256mx16.cfg */

MX6_MMDC_P0_MDPDC, 0x00020036,
MX6_MMDC_P0_MDCFG0, 0x898E78f5,
MX6_MMDC_P0_MDCFG1, 0xff328f64,
MX6_MMDC_P0_MDCFG2, 0x01FF00DB,
MX6_MMDC_P0_MDRWD, 0x000026D2,
MX6_MMDC_P0_MDOR, 0x008E1023,
MX6_MMDC_P0_MDOTC, 0x09444040,
MX6_MMDC_P0_MDPDC, 0x00025576,
MX6_MMDC_P0_MDASP, 0x00000047,
MX6_MMDC_P0_MDCTL, 0x841A0000,
MX6_MMDC_P0_MDSCR, 0x02888032,
MX6_MMDC_P0_MDSCR, 0x00008033,
MX6_MMDC_P0_MDSCR, 0x00048031,
MX6_MMDC_P0_MDSCR, 0x19408030,
MX6_MMDC_P0_MDSCR, 0x04008040,
MX6_MMDC_P0_MPZQHWCTRL, 0xA1390003,
MX6_MMDC_P1_MPZQHWCTRL, 0xA1390003,
MX6_MMDC_P0_MDREF, 0x00007800,
MX6_MMDC_P0_MPODTCTRL, 0x00022227,
MX6_MMDC_P1_MPODTCTRL, 0x00022227,

MX6_MMDC_P0_MPDGCTRL0, 0x03300338,
MX6_MMDC_P0_MPDGCTRL1, 0x03240324,
MX6_MMDC_P1_MPDGCTRL0, 0x03440350,
MX6_MMDC_P1_MPDGCTRL1, 0x032C0308,

MX6_MMDC_P0_MPRDDLCTL, 0x40363C3E,
MX6_MMDC_P1_MPRDDLCTL, 0x3C3E3C46,

MX6_MMDC_P0_MPWRDLCTL, 0x403E463E,
MX6_MMDC_P1_MPWRDLCTL, 0x4A384C46,

MX6_MMDC_P0_MPWLDECTRL0, 0x0009000E,
MX6_MMDC_P0_MPWLDECTRL1, 0x0018000B,
MX6_MMDC_P1_MPWLDECTRL0, 0x00060015,
MX6_MMDC_P1_MPWLDECTRL1, 0x0006000E,

MX6_MMDC_P0_MPMUR0, 0x00000800,
MX6_MMDC_P1_MPMUR0, 0x00000800,
MX6_MMDC_P0_MDSCR, 0x00000000,
MX6_MMDC_P0_MAPSR, 0x00011006,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFFFF3, &ccm->CCGR2);
	writel(0x3FF0300F, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000F3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);

/*
 * Setup CCM_CCOSR register as follows:
 *
 * cko1_en  = 1	   --> CKO1 enabled
 * cko1_div = 111  --> divide by 8
 * cko1_sel = 1011 --> ahb_clk_root
 *
 * This sets CKO1 at ahb_clk_root/8 = 132/8 = 16.5 MHz
 */
	writel(0x000000FB, &ccm->ccosr);
}

static void ddr_init(int *table, int size)
{
	int i;

	for (i = 0; i < size / 2 ; i++)
		writel(table[2 * i + 1], table[2 * i]);
}

static void spl_dram_init(void)
{
	int minc, maxc;

	switch (get_cpu_temp_grade(&minc, &maxc)) {
	case TEMP_COMMERCIAL:
	case TEMP_EXTCOMMERCIAL:
		puts("Commercial temperature grade DDR3 timings.\n");
		ddr_init(mx6_com_dcd_table, ARRAY_SIZE(mx6_com_dcd_table));
		break;
	case TEMP_INDUSTRIAL:
	case TEMP_AUTOMOTIVE:
	default:
		puts("Industrial temperature grade DDR3 timings.\n");
		ddr_init(mx6_it_dcd_table, ARRAY_SIZE(mx6_it_dcd_table));
		break;
	};
	udelay(100);
}

void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* iomux */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

#ifndef CONFIG_TDX_APALIS_IMX6_V1_0
	/* Make sure we use dte mode */
	setup_dtemode_uart();
#endif

	/* DDR initialization */
	spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}

void reset_cpu(ulong addr)
{
}

#endif /* CONFIG_SPL_BUILD */

static struct mxc_serial_platdata mxc_serial_plat = {
	.reg = (struct mxc_uart *)UART1_BASE,
	.use_dte = true,
};

U_BOOT_DEVICE(mxc_serial) = {
	.name = "serial_mxc",
	.platdata = &mxc_serial_plat,
};
