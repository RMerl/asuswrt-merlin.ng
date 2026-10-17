// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * Copyright (C) 2013, 2014 TQ-Systems (ported SabreSD to TQMa6x)
 * Author: Markus Niebel <markus.niebel@tq-group.com>
 *
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 */

#include <init.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>

#include <common.h>
#include <fsl_esdhc_imx.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <i2c.h>
#include <miiphy.h>
#include <mmc.h>
#include <netdev.h>

#include "tqma6_bb.h"

/* UART */
#define UART4_PAD_CTRL (			\
		PAD_CTL_HYS |			\
		PAD_CTL_PUS_100K_UP |		\
		PAD_CTL_PUE |			\
		PAD_CTL_PKE |			\
		PAD_CTL_SPEED_MED |		\
		PAD_CTL_DSE_40ohm |		\
		PAD_CTL_SRE_SLOW		\
		)

static iomux_v3_cfg_t const uart4_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT17__UART4_CTS_B, UART4_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT16__UART4_RTS_B, UART4_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT13__UART4_RX_DATA, UART4_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT12__UART4_TX_DATA, UART4_PAD_CTRL),
};

static void setup_iomuxc_uart4(void)
{
	imx_iomux_v3_setup_multiple_pads(uart4_pads, ARRAY_SIZE(uart4_pads));
}

/* MMC */
#define USDHC2_PAD_CTRL (			\
		PAD_CTL_HYS |			\
		PAD_CTL_PUS_47K_UP |		\
		PAD_CTL_SPEED_LOW |		\
		PAD_CTL_DSE_80ohm |		\
		PAD_CTL_SRE_FAST		\
		)

#define USDHC2_CLK_PAD_CTRL (			\
		PAD_CTL_HYS |			\
		PAD_CTL_PUS_47K_UP |		\
		PAD_CTL_SPEED_LOW |		\
		PAD_CTL_DSE_40ohm |		\
		PAD_CTL_SRE_FAST		\
		)

static iomux_v3_cfg_t const usdhc2_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_SD2_CLK__SD2_CLK, USDHC2_CLK_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_CMD__SD2_CMD, USDHC2_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT0__SD2_DATA0, USDHC2_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT1__SD2_DATA1, USDHC2_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT2__SD2_DATA2, USDHC2_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT3__SD2_DATA3, USDHC2_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_GPIO_4__GPIO1_IO04, USDHC2_PAD_CTRL), /* CD */
	NEW_PAD_CTRL(MX6_PAD_GPIO_2__SD2_WP, USDHC2_PAD_CTRL), /* WP */
};

#define USDHC2_CD_GPIO IMX_GPIO_NR(1, 4)
#define USDHC2_WP_GPIO IMX_GPIO_NR(1, 2)

static struct fsl_esdhc_cfg usdhc2_cfg = {
	.esdhc_base = USDHC2_BASE_ADDR,
	.max_bus_width = 4,
};

int tqma6_bb_board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC2_BASE_ADDR)
		ret = !gpio_get_value(USDHC2_CD_GPIO);

	return ret;
}

int tqma6_bb_board_mmc_getwp(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC2_BASE_ADDR)
		ret = gpio_get_value(USDHC2_WP_GPIO);

	return ret;
}

int tqma6_bb_board_mmc_init(struct bd_info *bis)
{
	int ret;

	imx_iomux_v3_setup_multiple_pads(usdhc2_pads, ARRAY_SIZE(usdhc2_pads));

	ret = gpio_request(USDHC2_CD_GPIO, "mmc-cd");
	if (!ret)
		gpio_direction_input(USDHC2_CD_GPIO);
	ret = gpio_request(USDHC2_WP_GPIO, "mmc-wp");
	if (!ret)
		gpio_direction_input(USDHC2_WP_GPIO);

	usdhc2_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	if(fsl_esdhc_initialize(bis, &usdhc2_cfg))
		puts("WARNING: failed to initialize SD\n");

	return 0;
}

/* Ethernet */
#define ENET_PAD_CTRL (				\
		PAD_CTL_HYS |			\
		PAD_CTL_PUS_100K_UP |		\
		PAD_CTL_PUE |			\
		PAD_CTL_PKE |			\
		PAD_CTL_SPEED_MED |		\
		PAD_CTL_DSE_40ohm |		\
		PAD_CTL_SRE_SLOW		\
		)

static iomux_v3_cfg_t const enet_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_ENET_MDC__ENET_MDC, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_MDIO__ENET_MDIO, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_GPIO_16__ENET_REF_CLK, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_RXD0__ENET_RX_DATA0, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_RXD1__ENET_RX_DATA1, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_CRS_DV__ENET_RX_EN, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_RX_ER__ENET_RX_ER, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_TXD0__ENET_TX_DATA0, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_TXD1__ENET_TX_DATA1, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_TX_EN__ENET_TX_EN, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_GPIO_19__ENET_TX_ER, ENET_PAD_CTRL),

	/* ENET1 reset */
	NEW_PAD_CTRL(MX6_PAD_GPIO_8__GPIO1_IO08, ENET_PAD_CTRL),
	/* ENET1 interrupt */
	NEW_PAD_CTRL(MX6_PAD_GPIO_9__GPIO1_IO09, ENET_PAD_CTRL),
};

#define ENET_PHY_RESET_GPIO IMX_GPIO_NR(1, 8)

static void setup_iomuxc_enet(void)
{
	int ret;

	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));

	/* Reset LAN8720 PHY */
	ret = gpio_request(ENET_PHY_RESET_GPIO, "phy-reset");
	if (!ret)
		gpio_direction_output(ENET_PHY_RESET_GPIO , 0);
	udelay(25000);
	gpio_set_value(ENET_PHY_RESET_GPIO, 1);
}

int board_eth_init(struct bd_info *bis)
{
	return cpu_eth_init(bis);
}

/* GPIO */
#define GPIO_PAD_CTRL (				\
		PAD_CTL_HYS |			\
		PAD_CTL_PUS_100K_UP |		\
		PAD_CTL_PUE |			\
		PAD_CTL_SPEED_MED |		\
		PAD_CTL_DSE_40ohm |		\
		PAD_CTL_SRE_SLOW		\
		)

#define GPIO_OD_PAD_CTRL (			\
		PAD_CTL_HYS |			\
		PAD_CTL_PUS_100K_UP |		\
		PAD_CTL_PUE |			\
		PAD_CTL_ODE |			\
		PAD_CTL_SPEED_MED |		\
		PAD_CTL_DSE_40ohm |		\
		PAD_CTL_SRE_SLOW		\
		)

static iomux_v3_cfg_t const gpio_pads[] = {
	/* USB_H_PWR */
	NEW_PAD_CTRL(MX6_PAD_GPIO_0__GPIO1_IO00, GPIO_PAD_CTRL),
	/* USB_OTG_PWR */
	NEW_PAD_CTRL(MX6_PAD_EIM_D22__GPIO3_IO22, GPIO_PAD_CTRL),
	/* PCIE_RST */
	NEW_PAD_CTRL(MX6_PAD_NANDF_CLE__GPIO6_IO07, GPIO_OD_PAD_CTRL),
	/* UART1_PWRON */
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT14__GPIO5_IO08, GPIO_PAD_CTRL),
	/* UART2_PWRON */
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT16__GPIO5_IO10, GPIO_PAD_CTRL),
	/* UART3_PWRON */
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT18__GPIO5_IO12, GPIO_PAD_CTRL),
};

#define GPIO_USB_H_PWR		IMX_GPIO_NR(1, 0)
#define GPIO_USB_OTG_PWR	IMX_GPIO_NR(3, 22)
#define GPIO_PCIE_RST		IMX_GPIO_NR(6, 7)
#define GPIO_UART1_PWRON	IMX_GPIO_NR(5, 8)
#define GPIO_UART2_PWRON	IMX_GPIO_NR(5, 10)
#define GPIO_UART3_PWRON	IMX_GPIO_NR(5, 12)

static void gpio_init(void)
{
	int ret;

	imx_iomux_v3_setup_multiple_pads(gpio_pads, ARRAY_SIZE(gpio_pads));

	ret = gpio_request(GPIO_USB_H_PWR, "usb-h-pwr");
	if (!ret)
		gpio_direction_output(GPIO_USB_H_PWR, 1);
	ret = gpio_request(GPIO_USB_OTG_PWR, "usb-otg-pwr");
	if (!ret)
		gpio_direction_output(GPIO_USB_OTG_PWR, 1);
	ret = gpio_request(GPIO_PCIE_RST, "pcie-reset");
	if (!ret)
		gpio_direction_output(GPIO_PCIE_RST, 1);
	ret = gpio_request(GPIO_UART1_PWRON, "uart1-pwr");
	if (!ret)
		gpio_direction_output(GPIO_UART1_PWRON, 0);
	ret = gpio_request(GPIO_UART2_PWRON, "uart2-pwr");
	if (!ret)
		gpio_direction_output(GPIO_UART2_PWRON, 0);
	ret = gpio_request(GPIO_UART3_PWRON, "uart3-pwr");
	if (!ret)
		gpio_direction_output(GPIO_UART3_PWRON, 0);
}

void tqma6_iomuxc_spi(void)
{
	/* No SPI on this baseboard */
}

int tqma6_bb_board_early_init_f(void)
{
	setup_iomuxc_uart4();

	return 0;
}

int tqma6_bb_board_init(void)
{
	setup_iomuxc_enet();

	gpio_init();

	/* Turn the UART-couplers on one-after-another */
	gpio_set_value(GPIO_UART1_PWRON, 1);
	mdelay(10);
	gpio_set_value(GPIO_UART2_PWRON, 1);
	mdelay(10);
	gpio_set_value(GPIO_UART3_PWRON, 1);

	return 0;
}

int tqma6_bb_board_late_init(void)
{
	return 0;
}

const char *tqma6_bb_get_boardname(void)
{
	return "WRU-IV";
}

static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"sd2",	 MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	/* 8 bit bus width */
	{"emmc", MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	{ NULL, 0 },
};

int misc_init_r(void)
{
	add_board_boot_modes(board_boot_modes);

	return 0;
}

#define WRU4_USB_H1_PWR		IMX_GPIO_NR(1, 0)
#define WRU4_USB_OTG_PWR	IMX_GPIO_NR(3, 22)

int board_ehci_hcd_init(int port)
{
	int ret;

	ret = gpio_request(WRU4_USB_H1_PWR, "usb-h1-pwr");
	if (!ret)
		gpio_direction_output(WRU4_USB_H1_PWR, 1);

	ret = gpio_request(WRU4_USB_OTG_PWR, "usb-OTG-pwr");
	if (!ret)
		gpio_direction_output(WRU4_USB_OTG_PWR, 1);

	return 0;
}

int board_ehci_power(int port, int on)
{
	if (port)
		gpio_set_value(WRU4_USB_OTG_PWR, on);
	else
		gpio_set_value(WRU4_USB_H1_PWR, on);

	return 0;
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
void tqma6_bb_ft_board_setup(void *blob, struct bd_info *bd)
{
	/* TBD */
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
