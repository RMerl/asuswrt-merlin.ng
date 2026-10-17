// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 * Copyright (C) 2018, Boundary Devices <info@boundarydevices.com>
 */

#include <common.h>
#include <env.h>
#include <init.h>
#include <malloc.h>
#include <errno.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/clock.h>
#include <spl.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <power/pmic.h>

DECLARE_GLOBAL_DATA_PTR;


#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)

static iomux_v3_cfg_t const wdog_pads[] = {
	IMX8MQ_PAD_GPIO1_IO02__WDOG1_WDOG_B | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};


int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));
	set_wdog_reset(wdog);

	return 0;
}

#ifdef CONFIG_FEC_MXC

#define PHY_RESET	IMX_GPIO_NR(1, 9)
#define PHY_RX_CTL	IMX_GPIO_NR(1, 24)
#define PHY_RXC		IMX_GPIO_NR(1, 25)
#define PHY_RD0		IMX_GPIO_NR(1, 26)
#define PHY_RD1		IMX_GPIO_NR(1, 27)
#define PHY_RD2		IMX_GPIO_NR(1, 28)
#define PHY_RD3		IMX_GPIO_NR(1, 29)

#define STRAP_AR8035	(0x28) // 0010 1000

static const iomux_v3_cfg_t enet_ar8035_gpio_pads[] = {
	IMX8MQ_PAD_GPIO1_IO09__GPIO1_IO9 | MUX_PAD_CTRL(PAD_CTL_DSE6),
	IMX8MQ_PAD_ENET_RD0__GPIO1_IO26 | MUX_PAD_CTRL(0x91),
	IMX8MQ_PAD_ENET_RD1__GPIO1_IO27 | MUX_PAD_CTRL(0x91),
	IMX8MQ_PAD_ENET_RD2__GPIO1_IO28 | MUX_PAD_CTRL(0x91),
	IMX8MQ_PAD_ENET_RD3__GPIO1_IO29 | MUX_PAD_CTRL(0xd1),
	IMX8MQ_PAD_ENET_RX_CTL__GPIO1_IO24 | MUX_PAD_CTRL(0x91),
	/* 1.8V(1)/1.5V select(0) */
	IMX8MQ_PAD_ENET_RXC__GPIO1_IO25 | MUX_PAD_CTRL(0xd1),
};

static const iomux_v3_cfg_t enet_ar8035_pads[] = {
	IMX8MQ_PAD_ENET_RD0__ENET_RGMII_RD0 | MUX_PAD_CTRL(0x91),
	IMX8MQ_PAD_ENET_RD1__ENET_RGMII_RD1 | MUX_PAD_CTRL(0x91),
	IMX8MQ_PAD_ENET_RD2__ENET_RGMII_RD2 | MUX_PAD_CTRL(0x91),
	IMX8MQ_PAD_ENET_RD3__ENET_RGMII_RD3 | MUX_PAD_CTRL(0x91),
	IMX8MQ_PAD_ENET_RX_CTL__ENET_RGMII_RX_CTL | MUX_PAD_CTRL(0x91),
	IMX8MQ_PAD_ENET_RXC__ENET_RGMII_RXC | MUX_PAD_CTRL(0x91),
};

static void setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Pull PHY into reset */
	gpio_request(PHY_RESET, "fec_rst");
	gpio_direction_output(PHY_RESET, 0);

	/* Configure ethernet pins value as GPIOs */
	gpio_request(PHY_RD0, "fec_rd0");
	gpio_direction_output(PHY_RD0, 0);
	gpio_request(PHY_RD1, "fec_rd1");
	gpio_direction_output(PHY_RD1, 0);
	gpio_request(PHY_RD2, "fec_rd2");
	gpio_direction_output(PHY_RD2, 0);
	gpio_request(PHY_RD3, "fec_rd3");
	gpio_direction_output(PHY_RD3, 1);
	gpio_request(PHY_RX_CTL, "fec_rx_ctl");
	gpio_direction_output(PHY_RX_CTL, 0);
	gpio_request(PHY_RXC, "fec_rxc");
	gpio_direction_output(PHY_RXC, 1);

	/* Set ethernet pins to GPIO to bootstrap PHY */
	imx_iomux_v3_setup_multiple_pads(enet_ar8035_gpio_pads,
	    ARRAY_SIZE(enet_ar8035_gpio_pads));

	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(&gpr->gpr[1], BIT(13) | BIT(17), 0);
	/* Enable RGMII TX clk output */
	setbits_le32(&gpr->gpr[1], BIT(22));
	set_clk_enet(ENET_125MHZ);

	/* 1 ms minimum reset pulse for ar8035 */
	mdelay(1);

	/* Release PHY from reset */
	gpio_set_value(PHY_RESET, 1);

	/* strap hold time for AR8035, 5 fails, 6 works, so 12 should be safe */
	udelay(12);

	/* Change ethernet pins back to normal function */
	imx_iomux_v3_setup_multiple_pads(enet_ar8035_pads,
	    ARRAY_SIZE(enet_ar8035_pads));
}
#endif

#define USB1_HUB_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)
#define USB1_HUB_RESET		IMX_GPIO_NR(1, 14)

static void setup_usb(void)
{
	imx_iomux_v3_setup_pad(IMX8MQ_PAD_GPIO1_IO14__GPIO1_IO14 |
	    MUX_PAD_CTRL(USB1_HUB_PAD_CTRL));
	gpio_request(USB1_HUB_RESET, "usb1_rst");
	gpio_direction_output(USB1_HUB_RESET, 0);
	mdelay(10);
	gpio_set_value(USB1_HUB_RESET, 1);
}

int board_init(void)
{
#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

	setup_usb();

#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_DWC3)
	init_usb_clk();
#endif

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "Reform2");
	env_set("board_rev", "iMX8MQ");
#endif

	return 0;
}
