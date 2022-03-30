// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/video.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <pwm.h>

struct i2c_pads_info i2c_pad_info3 = {
	.scl = {
		.i2c_mode = MX6_PAD_EIM_D17__I2C3_SCL | PC,
		.gpio_mode = MX6_PAD_EIM_D17__GPIO3_IO17 | PC,
		.gp = IMX_GPIO_NR(3, 17)
	},
	.sda = {
		.i2c_mode = MX6_PAD_EIM_D18__I2C3_SDA | PC,
		.gpio_mode = MX6_PAD_EIM_D18__GPIO3_IO18 | PC,
		.gp = IMX_GPIO_NR(3, 18)
	}
};

iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_CSI0_DAT10__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT11__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const uart5_pads[] = {
	MX6_PAD_CSI0_DAT14__UART5_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT15__UART5_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const gpio_pads[] = {
	/* LED enable */
	MX6_PAD_SD4_DAT5__GPIO2_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* spi flash WP protect */
	MX6_PAD_SD4_DAT7__GPIO2_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* backlight enable */
	MX6_PAD_GPIO_2__GPIO1_IO02 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LED yellow */
	MX6_PAD_GPIO_3__GPIO1_IO03 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LED red */
	MX6_PAD_GPIO_4__GPIO1_IO04 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LED green */
	MX6_PAD_GPIO_5__GPIO1_IO05 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LED blue */
	MX6_PAD_GPIO_6__GPIO1_IO06 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* i2c4 scl */
	MX6_PAD_GPIO_7__GPIO1_IO07 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* i2c4 sda */
	MX6_PAD_GPIO_8__GPIO1_IO08 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* spi CS 1 */
	MX6_PAD_EIM_A25__GPIO5_IO02 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const misc_pads[] = {
	MX6_PAD_GPIO_1__USB_OTG_ID		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* OTG Power enable */
	MX6_PAD_EIM_D31__GPIO3_IO31		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_KEY_ROW4__GPIO4_IO15		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_GPIO_16__ENET_REF_CLK	| MUX_PAD_CTRL(0x4001b0a8),
	MX6_PAD_ENET_MDIO__ENET_MDIO	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_TXD0__ENET_TX_DATA0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_TXD1__ENET_TX_DATA1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_TX_EN__ENET_TX_EN	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_RX_ER__ENET_RX_ER	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_RXD0__ENET_RX_DATA0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_RXD1__ENET_RX_DATA1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_CRS_DV__ENET_RX_EN	| MUX_PAD_CTRL(ENET_PAD_CTRL),
};

static void setup_iomux_enet(void)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));

	/* set GPIO_16 as ENET_REF_CLK_OUT */
	setbits_le32(&iomux->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);
}

static iomux_v3_cfg_t const backlight_pads[] = {
	MX6_PAD_GPIO_9__PWM1_OUT | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD4_DAT1__PWM3_OUT | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_GPIO_2__GPIO1_IO02 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

iomux_v3_cfg_t const ecspi4_pads[] = {
	MX6_PAD_EIM_D21__ECSPI4_SCLK | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_D22__ECSPI4_MISO | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_D28__ECSPI4_MOSI | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_D20__GPIO3_IO20 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const display_pads[] = {
	MX6_PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK | MUX_PAD_CTRL(DISP_PAD_CTRL),
	MX6_PAD_DI0_PIN15__IPU1_DI0_PIN15,
	MX6_PAD_DI0_PIN2__IPU1_DI0_PIN02,
	MX6_PAD_DI0_PIN3__IPU1_DI0_PIN03,
	MX6_PAD_DI0_PIN4__GPIO4_IO20,
	MX6_PAD_DISP0_DAT0__IPU1_DISP0_DATA00,
	MX6_PAD_DISP0_DAT1__IPU1_DISP0_DATA01,
	MX6_PAD_DISP0_DAT2__IPU1_DISP0_DATA02,
	MX6_PAD_DISP0_DAT3__IPU1_DISP0_DATA03,
	MX6_PAD_DISP0_DAT4__IPU1_DISP0_DATA04,
	MX6_PAD_DISP0_DAT5__IPU1_DISP0_DATA05,
	MX6_PAD_DISP0_DAT6__IPU1_DISP0_DATA06,
	MX6_PAD_DISP0_DAT7__IPU1_DISP0_DATA07,
	MX6_PAD_DISP0_DAT8__IPU1_DISP0_DATA08,
	MX6_PAD_DISP0_DAT9__IPU1_DISP0_DATA09,
	MX6_PAD_DISP0_DAT10__IPU1_DISP0_DATA10,
	MX6_PAD_DISP0_DAT11__IPU1_DISP0_DATA11,
	MX6_PAD_DISP0_DAT12__IPU1_DISP0_DATA12,
	MX6_PAD_DISP0_DAT13__IPU1_DISP0_DATA13,
	MX6_PAD_DISP0_DAT14__IPU1_DISP0_DATA14,
	MX6_PAD_DISP0_DAT15__IPU1_DISP0_DATA15,
	MX6_PAD_DISP0_DAT16__IPU1_DISP0_DATA16,
	MX6_PAD_DISP0_DAT17__IPU1_DISP0_DATA17,
	MX6_PAD_DISP0_DAT18__IPU1_DISP0_DATA18,
	MX6_PAD_DISP0_DAT19__IPU1_DISP0_DATA19,
	MX6_PAD_DISP0_DAT20__IPU1_DISP0_DATA20,
	MX6_PAD_DISP0_DAT21__IPU1_DISP0_DATA21,
	MX6_PAD_DISP0_DAT22__IPU1_DISP0_DATA22,
	MX6_PAD_DISP0_DAT23__IPU1_DISP0_DATA23,
};

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == CONFIG_SF_DEFAULT_BUS && cs == CONFIG_SF_DEFAULT_CS)
		? (IMX_GPIO_NR(3, 20)) : -1;
}

static void setup_spi(void)
{
	int i;

	imx_iomux_v3_setup_multiple_pads(ecspi4_pads, ARRAY_SIZE(ecspi4_pads));
	for (i = 0; i < 3; i++)
		enable_spi_clk(true, i);

	/* set cs1 to high */
	gpio_direction_output(ECSPI4_CS1, 1);
}

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart5_pads, ARRAY_SIZE(uart5_pads));
}

int board_eth_init(bd_t *bis)
{
	struct iomuxc *iomuxc_regs =
				(struct iomuxc *)IOMUXC_BASE_ADDR;
	int ret;

	/* clear gpr1[14], gpr1[18:17] to select anatop clock */
	clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC_MASK, 0);

	ret = enable_fec_anatop_clock(0, ENET_50MHZ);
	if (ret)
		return ret;

	setup_iomux_enet();
	return cpu_eth_init(bis);
}

static void enable_lvds(struct display_info_t const *dev)
{
	imx_iomux_v3_setup_multiple_pads(
		display_pads,
		 ARRAY_SIZE(display_pads));
	imx_iomux_v3_setup_multiple_pads(
		backlight_pads,
		 ARRAY_SIZE(backlight_pads));

	/* enable backlight PWM 3 */
	if (pwm_init(2, 0, 0))
		goto error;
	/* duty cycle 500ns, period: 3000ns */
	if (pwm_config(2, 500, 3000))
		goto error;
	if (pwm_enable(2))
		goto error;
	return;

error:
	puts("error init pwm for backlight\n");
	return;
}

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;

	enable_ipu_clock();

	reg = readl(&mxc_ccm->cs2cdr);
	/* select pll 5 clock */
	reg &= MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK;
	reg &= MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK;
	writel(reg, &mxc_ccm->cs2cdr);

	imx_iomux_v3_setup_multiple_pads(backlight_pads,
					 ARRAY_SIZE(backlight_pads));
}

static void setup_iomux_gpio(void)
{
	imx_iomux_v3_setup_multiple_pads(gpio_pads, ARRAY_SIZE(gpio_pads));
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_gpio();

	setup_display();
	return 0;
}


static void setup_i2c4(void)
{
	/* i2c4 not used, set it to gpio input */
	gpio_request(IMX_GPIO_NR(1, 7), "i2c4_scl");
	gpio_direction_input(IMX_GPIO_NR(1, 7));
	gpio_request(IMX_GPIO_NR(1, 8), "i2c4_sda");
	gpio_direction_input(IMX_GPIO_NR(1, 8));
}

static void setup_board_gpio(void)
{
	/* enable LED */
	gpio_request(IMX_GPIO_NR(2, 13), "LED ena");
	gpio_direction_output(IMX_GPIO_NR(2, 13), 0);

	gpio_request(IMX_GPIO_NR(1, 3), "LED yellow");
	gpio_direction_output(IMX_GPIO_NR(1, 3), 1);
	gpio_request(IMX_GPIO_NR(1, 4), "LED red");
	gpio_direction_output(IMX_GPIO_NR(1, 4), 1);
	gpio_request(IMX_GPIO_NR(1, 5), "LED green");
	gpio_direction_output(IMX_GPIO_NR(1, 5), 1);
	gpio_request(IMX_GPIO_NR(1, 6), "LED blue");
	gpio_direction_output(IMX_GPIO_NR(1, 6), 1);
}

static void setup_board_spi(void)
{
}
