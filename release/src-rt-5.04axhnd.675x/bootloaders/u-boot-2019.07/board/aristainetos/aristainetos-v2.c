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
#include <micrel.h>
#include <spi.h>
#include <video.h>
#include <../drivers/video/imx/ipu.h>
#if defined(CONFIG_VIDEO_BMP_LOGO)
	#include <bmp_logo.h>
#endif

#define USDHC2_PAD_CTRL (PAD_CTL_SPEED_LOW |			\
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#if (CONFIG_SYS_BOARD_VERSION == 2)
	/* 4.3 display controller */
	#define ECSPI1_CS0		IMX_GPIO_NR(4, 9)
	#define ECSPI4_CS0		IMX_GPIO_NR(3, 29)
#elif (CONFIG_SYS_BOARD_VERSION == 3)
	#define ECSPI1_CS0		IMX_GPIO_NR(2, 30)   /* NOR flash */
	/* 4.3 display controller */
	#define ECSPI1_CS1		IMX_GPIO_NR(4, 10)
#endif

#define SOFT_RESET_GPIO		IMX_GPIO_NR(7, 13)
#define SD2_DRIVER_ENABLE	IMX_GPIO_NR(7, 8)

struct i2c_pads_info i2c_pad_info3 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO_5__I2C3_SCL | PC,
		.gpio_mode = MX6_PAD_GPIO_5__GPIO1_IO05 | PC,
		.gp = IMX_GPIO_NR(1, 5)
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO_6__I2C3_SDA | PC,
		.gpio_mode = MX6_PAD_GPIO_6__GPIO1_IO06 | PC,
		.gp = IMX_GPIO_NR(1, 6)
	}
};

struct i2c_pads_info i2c_pad_info4 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO_7__I2C4_SCL | PC,
		.gpio_mode = MX6_PAD_GPIO_7__GPIO1_IO07 | PC,
		.gp = IMX_GPIO_NR(1, 7)
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO_8__I2C4_SDA | PC,
		.gpio_mode = MX6_PAD_GPIO_8__GPIO1_IO08 | PC,
		.gp = IMX_GPIO_NR(1, 8)
	}
};

iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_CSI0_DAT10__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT11__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D19__UART1_CTS_B    | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D20__UART1_RTS_B    | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const uart2_pads[] = {
	MX6_PAD_EIM_D26__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D27__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const uart3_pads[] = {
	MX6_PAD_EIM_D24__UART3_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D25__UART3_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D31__UART3_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D23__UART3_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const uart4_pads[] = {
	MX6_PAD_KEY_COL0__UART4_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_KEY_ROW0__UART4_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const gpio_pads[] = {
	/* LED enable*/
	MX6_PAD_ENET_CRS_DV__GPIO1_IO25 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LED yellow */
	MX6_PAD_NANDF_CS3__GPIO6_IO16 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LED red */
#if (CONFIG_SYS_BOARD_VERSION == 2)
	MX6_PAD_EIM_EB0__GPIO2_IO28 | MUX_PAD_CTRL(NO_PAD_CTRL),
#elif (CONFIG_SYS_BOARD_VERSION == 3)
	MX6_PAD_EIM_WAIT__GPIO5_IO00 | MUX_PAD_CTRL(NO_PAD_CTRL),
#endif
	/* LED green */
	MX6_PAD_EIM_A24__GPIO5_IO04 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LED blue */
	MX6_PAD_EIM_EB1__GPIO2_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* spi flash WP protect */
	MX6_PAD_SD4_DAT7__GPIO2_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* spi CS 0 */
	MX6_PAD_EIM_D29__GPIO3_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* spi bus #2 SS driver enable */
	MX6_PAD_EIM_A23__GPIO6_IO06 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* RST_LOC# PHY reset input (has pull-down!)*/
	MX6_PAD_GPIO_18__GPIO7_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* SD 2 level shifter output enable */
	MX6_PAD_SD3_RST__GPIO7_IO08 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* SD1 card detect input */
	MX6_PAD_ENET_RXD0__GPIO1_IO27 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* SD1 write protect input */
	MX6_PAD_DI0_PIN4__GPIO4_IO20 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* SD2 card detect input */
	MX6_PAD_GPIO_19__GPIO4_IO05 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* SD2 write protect input */
	MX6_PAD_SD4_DAT2__GPIO2_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* Touchscreen IRQ */
	MX6_PAD_SD4_DAT1__GPIO2_IO09 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const misc_pads[] = {
	/* USB_OTG_ID = GPIO1_24*/
	MX6_PAD_ENET_RX_ER__USB_OTG_ID		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* H1 Power enable = GPIO1_0*/
	MX6_PAD_GPIO_0__USB_H1_PWR		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* OTG Power enable = GPIO4_15*/
	MX6_PAD_KEY_ROW4__USB_OTG_PWR		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__RGMII_TXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__RGMII_TD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__RGMII_TD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__RGMII_TD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__RGMII_TD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RXC__RGMII_RXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD0__RGMII_RD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD1__RGMII_RD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD2__RGMII_RD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD3__RGMII_RD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
};

static iomux_v3_cfg_t const backlight_pads[] = {
	/* backlight PWM brightness control */
	MX6_PAD_GPIO_9__PWM1_OUT | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* backlight enable */
	MX6_PAD_EIM_BCLK__GPIO6_IO31 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LCD power enable */
	MX6_PAD_NANDF_CS2__GPIO6_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const ecspi1_pads[] = {
	MX6_PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
#if (CONFIG_SYS_BOARD_VERSION == 2)
	MX6_PAD_KEY_ROW1__GPIO4_IO09 | MUX_PAD_CTRL(SPI_PAD_CTRL),
#elif (CONFIG_SYS_BOARD_VERSION == 3)
	MX6_PAD_EIM_EB2__GPIO2_IO30  | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_KEY_COL2__GPIO4_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
#endif
};

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));
}

#if (CONFIG_SYS_BOARD_VERSION == 2)
iomux_v3_cfg_t const ecspi4_pads[] = {
	MX6_PAD_EIM_D21__ECSPI4_SCLK | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_D22__ECSPI4_MISO | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_D28__ECSPI4_MOSI | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_A25__GPIO5_IO02  | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_D29__GPIO3_IO29  | MUX_PAD_CTRL(NO_PAD_CTRL),
};
#endif

static iomux_v3_cfg_t const display_pads[] = {
	MX6_PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK | MUX_PAD_CTRL(DISP_PAD_CTRL),
	MX6_PAD_DI0_PIN15__IPU1_DI0_PIN15,
	MX6_PAD_DI0_PIN2__IPU1_DI0_PIN02,
	MX6_PAD_DI0_PIN3__IPU1_DI0_PIN03,
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
	if (bus == CONFIG_SF_DEFAULT_BUS && cs == CONFIG_SF_DEFAULT_CS)
#if (CONFIG_SYS_BOARD_VERSION == 2)
		return IMX_GPIO_NR(5, 2);

	if (bus == 0 && cs == 0)
		return IMX_GPIO_NR(4, 9);
#elif (CONFIG_SYS_BOARD_VERSION == 3)
		return ECSPI1_CS0;

	if (bus == 0 && cs == 1)
		return ECSPI1_CS1;
#endif
	return -1;
}

static void setup_spi(void)
{
	int i;

	imx_iomux_v3_setup_multiple_pads(ecspi1_pads, ARRAY_SIZE(ecspi1_pads));

#if (CONFIG_SYS_BOARD_VERSION == 2)
	imx_iomux_v3_setup_multiple_pads(ecspi4_pads, ARRAY_SIZE(ecspi4_pads));
#endif

	for (i = 0; i < 4; i++)
		enable_spi_clk(true, i);

	gpio_direction_output(ECSPI1_CS0, 1);
#if (CONFIG_SYS_BOARD_VERSION == 2)
	gpio_direction_output(ECSPI4_CS1, 0);
	/* set cs0 to high (second device on spi bus #4) */
	gpio_direction_output(ECSPI4_CS0, 1);
#elif (CONFIG_SYS_BOARD_VERSION == 3)
	gpio_direction_output(ECSPI1_CS1, 1);
#endif
}

static void setup_iomux_uart(void)
{
	switch (CONFIG_MXC_UART_BASE) {
	case UART1_BASE:
		imx_iomux_v3_setup_multiple_pads(uart1_pads,
						 ARRAY_SIZE(uart1_pads));
		break;
	case UART2_BASE:
		imx_iomux_v3_setup_multiple_pads(uart2_pads,
						 ARRAY_SIZE(uart2_pads));
		break;
	case UART3_BASE:
		imx_iomux_v3_setup_multiple_pads(uart3_pads,
						 ARRAY_SIZE(uart3_pads));
		break;
	case UART4_BASE:
		imx_iomux_v3_setup_multiple_pads(uart4_pads,
						 ARRAY_SIZE(uart4_pads));
		break;
	}
}

int board_phy_config(struct phy_device *phydev)
{
	/* control data pad skew - devaddr = 0x02, register = 0x04 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* rx data pad skew - devaddr = 0x02, register = 0x05 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* tx data pad skew - devaddr = 0x02, register = 0x06 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* gtx and rx clock pad skew - devaddr = 0x02, register = 0x08 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_CLOCK_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x03FF);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	setup_iomux_enet();
	return cpu_eth_init(bis);
}

static int rotate_logo_one(unsigned char *out, unsigned char *in)
{
	int   i, j;

	for (i = 0; i < BMP_LOGO_WIDTH; i++)
		for (j = 0; j < BMP_LOGO_HEIGHT; j++)
			out[j * BMP_LOGO_WIDTH + BMP_LOGO_HEIGHT - 1 - i] =
			in[i * BMP_LOGO_WIDTH + j];
	return 0;
}

/*
 * Rotate the BMP_LOGO (only)
 * Will only work, if the logo is square, as
 * BMP_LOGO_HEIGHT and BMP_LOGO_WIDTH are defines, not variables
 */
void rotate_logo(int rotations)
{
	unsigned char out_logo[BMP_LOGO_WIDTH * BMP_LOGO_HEIGHT];
	unsigned char *in_logo;
	int   i, j;

	if (BMP_LOGO_WIDTH != BMP_LOGO_HEIGHT)
		return;

	in_logo = bmp_logo_bitmap;

	/* one 90 degree rotation */
	if (rotations == 1  ||  rotations == 2  ||  rotations == 3)
		rotate_logo_one(out_logo, in_logo);

	/* second 90 degree rotation */
	if (rotations == 2  ||  rotations == 3)
		rotate_logo_one(in_logo, out_logo);

	/* third 90 degree rotation */
	if (rotations == 3)
		rotate_logo_one(out_logo, in_logo);

	/* copy result back to original array */
	if (rotations == 1  ||  rotations == 3)
		for (i = 0; i < BMP_LOGO_WIDTH; i++)
			for (j = 0; j < BMP_LOGO_HEIGHT; j++)
				in_logo[i * BMP_LOGO_WIDTH + j] =
				out_logo[i * BMP_LOGO_WIDTH + j];
}

static void enable_display_power(void)
{
	imx_iomux_v3_setup_multiple_pads(backlight_pads,
					 ARRAY_SIZE(backlight_pads));

	/* backlight enable */
	gpio_direction_output(IMX_GPIO_NR(6, 31), 1);
	/* LCD power enable */
	gpio_direction_output(IMX_GPIO_NR(6, 15), 1);

	/* enable backlight PWM 1 */
	if (pwm_init(0, 0, 0))
		goto error;
	/* duty cycle 500ns, period: 3000ns */
	if (pwm_config(0, 50000, 300000))
		goto error;
	if (pwm_enable(0))
		goto error;
	return;

error:
	puts("error init pwm for backlight\n");
	return;
}

static void enable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;
	s32 timeout = 100000;

	/* set PLL5 clock */
	reg = readl(&ccm->analog_pll_video);
	reg |= BM_ANADIG_PLL_VIDEO_POWERDOWN;
	writel(reg, &ccm->analog_pll_video);

	/* set PLL5 to 232720000Hz */
	reg &= ~BM_ANADIG_PLL_VIDEO_DIV_SELECT;
	reg |= BF_ANADIG_PLL_VIDEO_DIV_SELECT(0x26);
	reg &= ~BM_ANADIG_PLL_VIDEO_POST_DIV_SELECT;
	reg |= BF_ANADIG_PLL_VIDEO_POST_DIV_SELECT(0);
	writel(reg, &ccm->analog_pll_video);

	writel(BF_ANADIG_PLL_VIDEO_NUM_A(0xC0238),
	       &ccm->analog_pll_video_num);
	writel(BF_ANADIG_PLL_VIDEO_DENOM_B(0xF4240),
	       &ccm->analog_pll_video_denom);

	reg &= ~BM_ANADIG_PLL_VIDEO_POWERDOWN;
	writel(reg, &ccm->analog_pll_video);

	while (timeout--)
		if (readl(&ccm->analog_pll_video) & BM_ANADIG_PLL_VIDEO_LOCK)
			break;
	if (timeout < 0)
		printf("Warning: video pll lock timeout!\n");

	reg = readl(&ccm->analog_pll_video);
	reg |= BM_ANADIG_PLL_VIDEO_ENABLE;
	reg &= ~BM_ANADIG_PLL_VIDEO_BYPASS;
	writel(reg, &ccm->analog_pll_video);

	/* set LDB0, LDB1 clk select to 000/000 (PLL5 clock) */
	reg = readl(&ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 | MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (0 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
		| (0 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &ccm->cs2cdr);

	reg = readl(&ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV;
	writel(reg, &ccm->cscmr2);

	reg = readl(&ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	writel(reg, &ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	      | IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH
	      | IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_HIGH
	      | IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	      | IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT
	      | IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED
	      | IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~IOMUXC_GPR3_LVDS0_MUX_CTL_MASK)
	       | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
		  << IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);

	return;
}

static void enable_spi_display(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;
	s32 timeout = 100000;

#if defined(CONFIG_VIDEO_BMP_LOGO)
	rotate_logo(3);  /* portrait display in landscape mode */
#endif

	/*
	 * set ldb clock to 28341000 Hz calculated through the formula:
	 * (XRES + LEFT_M + RIGHT_M + HSYNC_LEN) *
	 * (YRES + UPPER_M + LOWER_M + VSYNC_LEN) * REFRESH)
	 * see:
	 * https://community.freescale.com/thread/308170
	 */
	ipu_set_ldb_clock(28341000);

	reg = readl(&ccm->cs2cdr);

	/* select pll 5 clock */
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		| MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	writel(reg, &ccm->cs2cdr);

	/* set PLL5 to 197994996Hz */
	reg &= ~BM_ANADIG_PLL_VIDEO_DIV_SELECT;
	reg |= BF_ANADIG_PLL_VIDEO_DIV_SELECT(0x21);
	reg &= ~BM_ANADIG_PLL_VIDEO_POST_DIV_SELECT;
	reg |= BF_ANADIG_PLL_VIDEO_POST_DIV_SELECT(0);
	writel(reg, &ccm->analog_pll_video);

	writel(BF_ANADIG_PLL_VIDEO_NUM_A(0xfbf4),
	       &ccm->analog_pll_video_num);
	writel(BF_ANADIG_PLL_VIDEO_DENOM_B(0xf4240),
	       &ccm->analog_pll_video_denom);

	reg &= ~BM_ANADIG_PLL_VIDEO_POWERDOWN;
	writel(reg, &ccm->analog_pll_video);

	while (timeout--)
		if (readl(&ccm->analog_pll_video) & BM_ANADIG_PLL_VIDEO_LOCK)
			break;
	if (timeout < 0)
		printf("Warning: video pll lock timeout!\n");

	reg = readl(&ccm->analog_pll_video);
	reg |= BM_ANADIG_PLL_VIDEO_ENABLE;
	reg &= ~BM_ANADIG_PLL_VIDEO_BYPASS;
	writel(reg, &ccm->analog_pll_video);

	/* set LDB0, LDB1 clk select to 000/000 (PLL5 clock) */
	reg = readl(&ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 | MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (0 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
		| (0 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &ccm->cs2cdr);

	reg = readl(&ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV;
	writel(reg, &ccm->cscmr2);

	reg = readl(&ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	reg &= ~MXC_CCM_CHSCCDR_IPU1_DI0_PODF_MASK;
	reg |= (2 << MXC_CCM_CHSCCDR_IPU1_DI0_PODF_OFFSET);
	reg &= ~MXC_CCM_CHSCCDR_IPU1_DI0_PRE_CLK_SEL_MASK;
	reg |= (2 << MXC_CCM_CHSCCDR_IPU1_DI0_PRE_CLK_SEL_OFFSET);
	writel(reg, &ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	      | IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH
	      | IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_HIGH
	      | IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	      | IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT
	      | IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED
	      | IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~IOMUXC_GPR3_LVDS0_MUX_CTL_MASK)
	       | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
		  << IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);

	imx_iomux_v3_setup_multiple_pads(
		display_pads,
		 ARRAY_SIZE(display_pads));

	return;
}
static void setup_display(void)
{
	enable_ipu_clock();
	enable_display_power();
}

static void setup_iomux_gpio(void)
{
	imx_iomux_v3_setup_multiple_pads(gpio_pads, ARRAY_SIZE(gpio_pads));
}

static void set_gpr_register(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	writel(IOMUXC_GPR1_APP_CLK_REQ_N | IOMUXC_GPR1_PCIE_RDY_L23 |
	       IOMUXC_GPR1_EXC_MON_SLVE |
	       (2 << IOMUXC_GPR1_ADDRS0_OFFSET) |
	       IOMUXC_GPR1_ACT_CS0,
	       &iomuxc_regs->gpr[1]);
	writel(0x0, &iomuxc_regs->gpr[8]);
	writel(IOMUXC_GPR12_ARMP_IPG_CLK_EN | IOMUXC_GPR12_ARMP_AHB_CLK_EN |
	       IOMUXC_GPR12_ARMP_ATB_CLK_EN | IOMUXC_GPR12_ARMP_APB_CLK_EN,
	       &iomuxc_regs->gpr[12]);
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_gpio();

	gpio_direction_output(SOFT_RESET_GPIO, 1);
	gpio_direction_output(SD2_DRIVER_ENABLE, 1);
	setup_display();
	set_gpr_register();
	return 0;
}

static void setup_i2c4(void)
{
	setup_i2c(3, CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE,
		  &i2c_pad_info4);
}

static void setup_board_gpio(void)
{
	/* enable all LEDs */
	gpio_request(IMX_GPIO_NR(2, 13), "LED ena"); /* 25 */
	gpio_direction_output(IMX_GPIO_NR(1, 25), 0);

	/* switch off Status LEDs */
#if (CONFIG_SYS_BOARD_VERSION == 2)
	gpio_request(IMX_GPIO_NR(6, 16), "LED yellow"); /* 176 */
	gpio_direction_output(IMX_GPIO_NR(6, 16), 1);
	gpio_request(IMX_GPIO_NR(2, 28), "LED red"); /* 60 */
	gpio_direction_output(IMX_GPIO_NR(2, 28), 1);
	gpio_request(IMX_GPIO_NR(5, 4), "LED green"); /* 132 */
	gpio_direction_output(IMX_GPIO_NR(5, 4), 1);
	gpio_request(IMX_GPIO_NR(2, 29), "LED blue"); /* 61 */
	gpio_direction_output(IMX_GPIO_NR(2, 29), 1);
#elif (CONFIG_SYS_BOARD_VERSION == 3)
	gpio_request(IMX_GPIO_NR(6, 16), "LED yellow"); /* 176 */
	gpio_direction_output(IMX_GPIO_NR(6, 16), 0);
	gpio_request(IMX_GPIO_NR(5, 0), "LED red"); /* 128 */
	gpio_direction_output(IMX_GPIO_NR(5, 0), 0);
	gpio_request(IMX_GPIO_NR(5, 4), "LED green"); /* 132 */
	gpio_direction_output(IMX_GPIO_NR(5, 4), 0);
	gpio_request(IMX_GPIO_NR(2, 29), "LED blue"); /* 61 */
	gpio_direction_output(IMX_GPIO_NR(2, 29), 0);
#endif
}

static void setup_board_spi(void)
{
	/* enable spi bus #2 SS drivers (and spi bus #4 SS1 for rev2b) */
	gpio_direction_output(IMX_GPIO_NR(6, 6), 1);
}

int board_late_init(void)
{
	char *my_bootdelay;
	char bootmode = 0;
	char const *panel = env_get("panel");

	/*
	 * Check the boot-source. If booting from NOR Flash,
	 * disable bootdelay
	 */
	gpio_request(IMX_GPIO_NR(7, 6), "bootsel0");
	gpio_direction_input(IMX_GPIO_NR(7, 6));
	gpio_request(IMX_GPIO_NR(7, 7), "bootsel1");
	gpio_direction_input(IMX_GPIO_NR(7, 7));
	gpio_request(IMX_GPIO_NR(7, 1), "bootsel2");
	gpio_direction_input(IMX_GPIO_NR(7, 1));
	bootmode |= (gpio_get_value(IMX_GPIO_NR(7, 6)) ? 1 : 0) << 0;
	bootmode |= (gpio_get_value(IMX_GPIO_NR(7, 7)) ? 1 : 0) << 1;
	bootmode |= (gpio_get_value(IMX_GPIO_NR(7, 1)) ? 1 : 0) << 2;

	if (bootmode == 7) {
		my_bootdelay = env_get("nor_bootdelay");
		if (my_bootdelay != NULL)
			env_set("bootdelay", my_bootdelay);
		else
			env_set("bootdelay", "-2");
	}

	/* if we have the lg panel, we can initialze it now */
	if (panel)
		if (!strcmp(panel, displays[1].mode.name))
			lg4573_spi_startup(CONFIG_LG4573_BUS,
					   CONFIG_LG4573_CS,
					   10000000, SPI_MODE_0);

	return 0;
}
