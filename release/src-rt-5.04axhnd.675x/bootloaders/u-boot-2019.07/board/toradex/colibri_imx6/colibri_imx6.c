// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
 * Copyright (C) 2014-2019, Toradex AG
 * copied from nitrogen6x
 */

#include <common.h>
#include <dm.h>

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
#include <cpu.h>
#include <dm/platform_data/serial_mxc.h>
#include <environment.h>
#include <fsl_esdhc.h>
#include <imx_thermal.h>
#include <micrel.h>
#include <miiphy.h>
#include <netdev.h>
#include <cpu.h>

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

#define NO_PULLUP	(					\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_SRE_SLOW)

#define WEAK_PULLDOWN	(PAD_CTL_PUS_100K_DOWN |		\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_SLOW)

#define OUTPUT_RGB (PAD_CTL_SPEED_MED|PAD_CTL_DSE_60ohm|PAD_CTL_SRE_FAST)

int dram_init(void)
{
	/* use the DDR controllers configured size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    (ulong)imx_ddr_size());

	return 0;
}

/* Colibri UARTA */
iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_CSI0_DAT10__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT11__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#if defined(CONFIG_FSL_ESDHC) && defined(CONFIG_SPL_BUILD)
/* Colibri MMC */
iomux_v3_cfg_t const usdhc1_pads[] = {
	MX6_PAD_SD1_CLK__SD1_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_CMD__SD1_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT0__SD1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT1__SD1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT2__SD1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT3__SD1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NANDF_D5__GPIO2_IO05 | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
#	define GPIO_MMC_CD IMX_GPIO_NR(2, 5)
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
	MX6_PAD_SD3_RST__SD3_RESET  | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};
#endif /* CONFIG_FSL_ESDHC & CONFIG_SPL_BUILD */

iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_RXD0__ENET_RX_DATA0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_RXD1__ENET_RX_DATA1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_RX_ER__ENET_RX_ER		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_TX_EN__ENET_TX_EN		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_TXD0__ENET_TX_DATA0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_TXD1__ENET_TX_DATA1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_CRS_DV__ENET_RX_EN		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_GPIO_16__ENET_REF_CLK		| MUX_PAD_CTRL(ENET_PAD_CTRL),
};

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));
}

/* mux auxiliary pins to GPIO, so they can be used from the U-Boot cmdline */
iomux_v3_cfg_t const gpio_pads[] = {
	/* ADDRESS[17:18] [25] used as GPIO */
	MX6_PAD_KEY_ROW2__GPIO4_IO11	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_KEY_COL2__GPIO4_IO10	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_D1__GPIO2_IO01	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	/* ADDRESS[19:24] used as GPIO */
	MX6_PAD_DISP0_DAT23__GPIO5_IO17 | MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_DISP0_DAT22__GPIO5_IO16 | MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_DISP0_DAT21__GPIO5_IO15 | MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_DISP0_DAT20__GPIO5_IO14 | MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_DISP0_DAT19__GPIO5_IO13 | MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_DISP0_DAT18__GPIO5_IO12 | MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	/* DATA[16:29] [31]	 used as GPIO */
	MX6_PAD_EIM_LBA__GPIO2_IO27	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_EIM_BCLK__GPIO6_IO31	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_CS3__GPIO6_IO16	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_CS1__GPIO6_IO14	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_RB0__GPIO6_IO10	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_ALE__GPIO6_IO08	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_WP_B__GPIO6_IO09	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_CS0__GPIO6_IO11	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_CLE__GPIO6_IO07	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_GPIO_19__GPIO4_IO05	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_CSI0_MCLK__GPIO5_IO19	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_CSI0_PIXCLK__GPIO5_IO18 | MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_GPIO_4__GPIO1_IO04	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_GPIO_5__GPIO1_IO05	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_GPIO_2__GPIO1_IO02	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	/* DQM[0:3]	 used as GPIO */
	MX6_PAD_EIM_EB0__GPIO2_IO28	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_EIM_EB1__GPIO2_IO29	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_SD2_DAT2__GPIO1_IO13	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_D0__GPIO2_IO00	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	/* RDY	used as GPIO */
	MX6_PAD_EIM_WAIT__GPIO5_IO00	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	/* ADDRESS[16] DATA[30]	 used as GPIO */
	MX6_PAD_KEY_ROW4__GPIO4_IO15	| MUX_PAD_CTRL(WEAK_PULLDOWN) |
					  MUX_MODE_SION,
	MX6_PAD_KEY_COL4__GPIO4_IO14	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	/* CSI pins used as GPIO */
	MX6_PAD_EIM_A24__GPIO5_IO04	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_SD2_CMD__GPIO1_IO11	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_CS2__GPIO6_IO15	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_EIM_D18__GPIO3_IO18	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_EIM_A19__GPIO2_IO19	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_EIM_D29__GPIO3_IO29	| MUX_PAD_CTRL(WEAK_PULLDOWN) |
					  MUX_MODE_SION,
	MX6_PAD_EIM_A23__GPIO6_IO06	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_EIM_A20__GPIO2_IO18	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_EIM_A17__GPIO2_IO21	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_EIM_A18__GPIO2_IO20	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_EIM_EB3__GPIO2_IO31	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_EIM_D17__GPIO3_IO17	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_SD2_DAT0__GPIO1_IO15	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	/* GPIO */
	MX6_PAD_EIM_D26__GPIO3_IO26	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_EIM_D27__GPIO3_IO27	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_D6__GPIO2_IO06	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_D3__GPIO2_IO03	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_ENET_REF_CLK__GPIO1_IO23 | MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_DI0_PIN4__GPIO4_IO20	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_SD4_DAT3__GPIO2_IO11	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_NANDF_D4__GPIO2_IO04	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_SD4_DAT0__GPIO2_IO08	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_GPIO_7__GPIO1_IO07	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	MX6_PAD_GPIO_8__GPIO1_IO08	| MUX_PAD_CTRL(WEAK_PULLUP) |
					  MUX_MODE_SION,
	/* USBH_OC */
	MX6_PAD_EIM_D30__GPIO3_IO30	| MUX_PAD_CTRL(WEAK_PULLUP),
	/* USBC_ID */
	MX6_PAD_NANDF_D2__GPIO2_IO02	| MUX_PAD_CTRL(WEAK_PULLUP),
	/* USBC_DET */
	MX6_PAD_GPIO_17__GPIO7_IO12	| MUX_PAD_CTRL(WEAK_PULLUP),
};

static void setup_iomux_gpio(void)
{
	imx_iomux_v3_setup_multiple_pads(gpio_pads, ARRAY_SIZE(gpio_pads));
}

iomux_v3_cfg_t const usb_pads[] = {
	/* USBH_PEN */
	MX6_PAD_EIM_D31__GPIO3_IO31 | MUX_PAD_CTRL(NO_PAD_CTRL) | MUX_MODE_SION,
#	define GPIO_USBH_EN IMX_GPIO_NR(3, 31)
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
	setbits_le32((u32 *)(UART3_BASE + UFCR), UFCR_DCEDTE);

	clrbits_le32((u32 *)(UART1_BASE + UCR3), UCR3_DCD | UCR3_RI);
	clrbits_le32((u32 *)(UART2_BASE + UCR3), UCR3_DCD | UCR3_RI);
	clrbits_le32((u32 *)(UART3_BASE + UCR3), UCR3_DCD | UCR3_RI);
}

static void setup_iomux_uart(void)
{
	setup_dtemode_uart();
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

#ifdef CONFIG_USB_EHCI_MX6
int board_ehci_hcd_init(int port)
{
	imx_iomux_v3_setup_multiple_pads(usb_pads, ARRAY_SIZE(usb_pads));
	return 0;
}
#endif

#if defined(CONFIG_FSL_ESDHC) && defined(CONFIG_SPL_BUILD)
/* use the following sequence: eMMC, MMC */
struct fsl_esdhc_cfg usdhc_cfg[CONFIG_SYS_FSL_USDHC_NUM] = {
	{USDHC3_BASE_ADDR},
	{USDHC1_BASE_ADDR},
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
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	uint32_t base = IMX_FEC_BASE;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int ret;

	/* provide the PHY clock from the i.MX 6 */
	ret = enable_fec_anatop_clock(0, ENET_50MHZ);
	if (ret)
		return ret;

	/* set gpr1[ENET_CLK_SEL] */
	setbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);

	setup_iomux_enet();

#ifdef CONFIG_FEC_MXC
	bus = fec_get_miibus(base, -1);
	if (!bus)
		return 0;

	/* scan PHY 1..7 */
	phydev = phy_find_by_mask(bus, 0xff, PHY_INTERFACE_MODE_RMII);
	if (!phydev) {
		free(bus);
		puts("no PHY found\n");
		return 0;
	}

	phy_reset(phydev);
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
	/* Backlight On */
	MX6_PAD_EIM_D26__GPIO3_IO26 | MUX_PAD_CTRL(NO_PAD_CTRL) | MUX_MODE_SION,
#define RGB_BACKLIGHT_GP IMX_GPIO_NR(3, 26)
	/* Backlight PWM, used as GPIO in U-Boot */
	MX6_PAD_EIM_A22__GPIO2_IO16  | MUX_PAD_CTRL(NO_PULLUP),
	MX6_PAD_SD4_DAT1__GPIO2_IO09 | MUX_PAD_CTRL(NO_PAD_CTRL) |
				       MUX_MODE_SION,
#define RGB_BACKLIGHTPWM_GP IMX_GPIO_NR(2, 9)
};

static iomux_v3_cfg_t const rgb_pads[] = {
	MX6_PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DI0_PIN15__IPU1_DI0_PIN15 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DI0_PIN2__IPU1_DI0_PIN02 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DI0_PIN3__IPU1_DI0_PIN03 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT0__IPU1_DISP0_DATA00 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT1__IPU1_DISP0_DATA01 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT2__IPU1_DISP0_DATA02 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT3__IPU1_DISP0_DATA03 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT4__IPU1_DISP0_DATA04 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT5__IPU1_DISP0_DATA05 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT6__IPU1_DISP0_DATA06 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT7__IPU1_DISP0_DATA07 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT8__IPU1_DISP0_DATA08 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT9__IPU1_DISP0_DATA09 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT10__IPU1_DISP0_DATA10 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT11__IPU1_DISP0_DATA11 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT12__IPU1_DISP0_DATA12 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT13__IPU1_DISP0_DATA13 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT14__IPU1_DISP0_DATA14 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT15__IPU1_DISP0_DATA15 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT16__IPU1_DISP0_DATA16 | MUX_PAD_CTRL(OUTPUT_RGB),
	MX6_PAD_DISP0_DAT17__IPU1_DISP0_DATA17 | MUX_PAD_CTRL(OUTPUT_RGB),
};

static void do_enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

static void enable_rgb(struct display_info_t const *dev)
{
	imx_iomux_v3_setup_multiple_pads(
		rgb_pads,
		ARRAY_SIZE(rgb_pads));
	gpio_direction_output(RGB_BACKLIGHT_GP, 1);
	gpio_direction_output(RGB_BACKLIGHTPWM_GP, 0);
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
	.pixfmt	= IPU_PIX_FMT_RGB666,
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
	.pixfmt	= IPU_PIX_FMT_RGB666,
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
	gpio_request(RGB_BACKLIGHTPWM_GP, "PWM<A>");
	gpio_request(RGB_BACKLIGHT_GP, "BL_ON");
	gpio_direction_output(RGB_BACKLIGHTPWM_GP, 0);
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
	setup_iomux_uart();

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
#endif

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
	printf("Model: Toradex Colibri iMX6 %s %sMB%s\n",
	       is_cpu_type(MXC_CPU_MX6DL) ? "DualLite" : "Solo",
	       (gd->ram_size == 0x20000000) ? "512" : "256", it);
	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	u32 cma_size;

	ft_common_board_setup(blob, bd);

	cma_size = env_get_ulong("cma-size", 10, 320 * 1024 * 1024);
	cma_size = min((u32)(gd->ram_size >> 1), cma_size);

	fdt_setprop_u32(blob,
			fdt_path_offset(blob, "/reserved-memory/linux,cma"),
			"size",
			cma_size);
	return 0;
}
#endif

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	{"mmc",	MAKE_CFGVAL(0x40, 0x20, 0x00, 0x00)},
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
#include "asm/arch/mx6dl-ddr.h"
#include "asm/arch/iomux.h"
#include "asm/arch/crm_regs.h"

static int mx6s_dcd_table[] = {
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
/* TODO: check what the RALAT field does */
MX6_MMDC_P0_MDMISC, 0x00081740,

/*
 * MDSCR	con_req
 */
MX6_MMDC_P0_MDSCR, 0x00008000,


/* 800mhz_2x64mx16.cfg */

MX6_MMDC_P0_MDPDC, 0x0002002D,
MX6_MMDC_P0_MDCFG0, 0x2C305503,
MX6_MMDC_P0_MDCFG1, 0xB66D8D63,
MX6_MMDC_P0_MDCFG2, 0x01FF00DB,
MX6_MMDC_P0_MDRWD, 0x000026D2,
MX6_MMDC_P0_MDOR, 0x00301023,
MX6_MMDC_P0_MDOTC, 0x00333030,
MX6_MMDC_P0_MDPDC, 0x0002556D,
/* CS0 End: 7MSB of ((0x10000000, + 512M) -1) >> 25 */
MX6_MMDC_P0_MDASP, 0x00000017,
/* DDR3 DATA BUS SIZE: 64BIT */
/* MX6_MMDC_P0_MDCTL, 0x821A0000, */
/* DDR3 DATA BUS SIZE: 32BIT */
MX6_MMDC_P0_MDCTL, 0x82190000,

/* Write commands to DDR */
/* Load Mode Registers */
/* TODO Use Auto Self-Refresh mode (Extended Temperature)*/
/* MX6_MMDC_P0_MDSCR, 0x04408032, */
MX6_MMDC_P0_MDSCR, 0x04008032,
MX6_MMDC_P0_MDSCR, 0x00008033,
MX6_MMDC_P0_MDSCR, 0x00048031,
MX6_MMDC_P0_MDSCR, 0x13208030,
/* ZQ calibration */
MX6_MMDC_P0_MDSCR, 0x04008040,

MX6_MMDC_P0_MPZQHWCTRL, 0xA1390003,
MX6_MMDC_P1_MPZQHWCTRL, 0xA1390003,
MX6_MMDC_P0_MDREF, 0x00005800,

MX6_MMDC_P0_MPODTCTRL, 0x00000000,
MX6_MMDC_P1_MPODTCTRL, 0x00000000,

MX6_MMDC_P0_MPDGCTRL0, 0x42360232,
MX6_MMDC_P0_MPDGCTRL1, 0x021F022A,
MX6_MMDC_P1_MPDGCTRL0, 0x421E0224,
MX6_MMDC_P1_MPDGCTRL1, 0x02110218,

MX6_MMDC_P0_MPRDDLCTL, 0x41434344,
MX6_MMDC_P1_MPRDDLCTL, 0x4345423E,
MX6_MMDC_P0_MPWRDLCTL, 0x39383339,
MX6_MMDC_P1_MPWRDLCTL, 0x3E363930,

MX6_MMDC_P0_MPWLDECTRL0, 0x00340039,
MX6_MMDC_P0_MPWLDECTRL1, 0x002C002D,
MX6_MMDC_P1_MPWLDECTRL0, 0x00120019,
MX6_MMDC_P1_MPWLDECTRL1, 0x0012002D,

MX6_MMDC_P0_MPMUR0, 0x00000800,
MX6_MMDC_P1_MPMUR0, 0x00000800,
MX6_MMDC_P0_MDSCR, 0x00000000,
MX6_MMDC_P0_MAPSR, 0x00011006,
};

static int mx6dl_dcd_table[] = {
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
/* TODO: check what the RALAT field does */
MX6_MMDC_P0_MDMISC, 0x00081740,

/*
 * MDSCR	con_req
 */
MX6_MMDC_P0_MDSCR, 0x00008000,


/* 800mhz_2x64mx16.cfg */

MX6_MMDC_P0_MDPDC, 0x0002002D,
MX6_MMDC_P0_MDCFG0, 0x2C305503,
MX6_MMDC_P0_MDCFG1, 0xB66D8D63,
MX6_MMDC_P0_MDCFG2, 0x01FF00DB,
MX6_MMDC_P0_MDRWD, 0x000026D2,
MX6_MMDC_P0_MDOR, 0x00301023,
MX6_MMDC_P0_MDOTC, 0x00333030,
MX6_MMDC_P0_MDPDC, 0x0002556D,
/* CS0 End: 7MSB of ((0x10000000, + 512M) -1) >> 25 */
MX6_MMDC_P0_MDASP, 0x00000017,
/* DDR3 DATA BUS SIZE: 64BIT */
MX6_MMDC_P0_MDCTL, 0x821A0000,
/* DDR3 DATA BUS SIZE: 32BIT */
/* MX6_MMDC_P0_MDCTL, 0x82190000, */

/* Write commands to DDR */
/* Load Mode Registers */
/* TODO Use Auto Self-Refresh mode (Extended Temperature)*/
/* MX6_MMDC_P0_MDSCR, 0x04408032, */
MX6_MMDC_P0_MDSCR, 0x04008032,
MX6_MMDC_P0_MDSCR, 0x00008033,
MX6_MMDC_P0_MDSCR, 0x00048031,
MX6_MMDC_P0_MDSCR, 0x13208030,
/* ZQ calibration */
MX6_MMDC_P0_MDSCR, 0x04008040,

MX6_MMDC_P0_MPZQHWCTRL, 0xA1390003,
MX6_MMDC_P1_MPZQHWCTRL, 0xA1390003,
MX6_MMDC_P0_MDREF, 0x00005800,

MX6_MMDC_P0_MPODTCTRL, 0x00000000,
MX6_MMDC_P1_MPODTCTRL, 0x00000000,

MX6_MMDC_P0_MPDGCTRL0, 0x42360232,
MX6_MMDC_P0_MPDGCTRL1, 0x021F022A,
MX6_MMDC_P1_MPDGCTRL0, 0x421E0224,
MX6_MMDC_P1_MPDGCTRL1, 0x02110218,

MX6_MMDC_P0_MPRDDLCTL, 0x41434344,
MX6_MMDC_P1_MPRDDLCTL, 0x4345423E,
MX6_MMDC_P0_MPWRDLCTL, 0x39383339,
MX6_MMDC_P1_MPWRDLCTL, 0x3E363930,

MX6_MMDC_P0_MPWLDECTRL0, 0x00340039,
MX6_MMDC_P0_MPWLDECTRL1, 0x002C002D,
MX6_MMDC_P1_MPWLDECTRL0, 0x00120019,
MX6_MMDC_P1_MPWLDECTRL1, 0x0012002D,

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
		if (is_cpu_type(MXC_CPU_MX6DL)) {
			puts("Commercial temperature grade DDR3 timings, 64bit bus width.\n");
			ddr_init(mx6dl_dcd_table, ARRAY_SIZE(mx6dl_dcd_table));
		} else {
			puts("Commercial temperature grade DDR3 timings, 32bit bus width.\n");
			ddr_init(mx6s_dcd_table, ARRAY_SIZE(mx6s_dcd_table));
		}
		break;
	case TEMP_INDUSTRIAL:
	case TEMP_AUTOMOTIVE:
	default:
		if (is_cpu_type(MXC_CPU_MX6DL)) {
			puts("Industrial temperature grade DDR3 timings, 64bit bus width.\n");
			ddr_init(mx6dl_dcd_table, ARRAY_SIZE(mx6dl_dcd_table));
		} else {
			puts("Industrial temperature grade DDR3 timings, 32bit bus width.\n");
			ddr_init(mx6s_dcd_table, ARRAY_SIZE(mx6s_dcd_table));
		}
		break;
	};
	udelay(100);
}

static iomux_v3_cfg_t const gpio_reset_pad[] = {
	MX6_PAD_RGMII_RD1__GPIO6_IO27 | MUX_PAD_CTRL(NO_PAD_CTRL) |
					MUX_MODE_SION
#define GPIO_NRESET IMX_GPIO_NR(6, 27)
};

#define IMX_RESET_CAUSE_POR 0x00011
static void nreset_out(void)
{
	int reset_cause = get_imx_reset_cause();

	if (reset_cause != IMX_RESET_CAUSE_POR) {
		imx_iomux_v3_setup_multiple_pads(gpio_reset_pad,
						 ARRAY_SIZE(gpio_reset_pad));
		gpio_direction_output(GPIO_NRESET, 1);
		udelay(100);
		gpio_direction_output(GPIO_NRESET, 0);
	}
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

	/* Make sure we use dte mode */
	setup_dtemode_uart();

	/* DDR initialization */
	spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* Assert nReset_Out */
	nreset_out();

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
