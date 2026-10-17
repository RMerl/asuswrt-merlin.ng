// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Eukréa Electromatique
 * Author: Eric Bénard <eric@eukrea.com>
 *         Fabio Estevam <fabio.estevam@freescale.com>
 *         Jon Nettleton <jon.nettleton@gmail.com>
 *
 * based on sabresd.c which is :
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * and on hummingboard.c which is :
 * Copyright (C) 2013 SolidRun ltd.
 * Copyright (C) 2013 Jon Nettleton <jon.nettleton@gmail.com>.
 */

#include <common.h>
#include <init.h>
#include <net.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/spi.h>
#include <asm/mach-imx/video.h>
#include <i2c.h>
#include <input.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CLK_CTRL (PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm | PAD_CTL_SRE_FAST |			\
	PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define ENET_PAD_CTRL_PD  (PAD_CTL_PUS_100K_DOWN |		\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define ENET_PAD_CTRL_CLK  ((PAD_CTL_PUS_100K_UP & ~PAD_CTL_PKE) | \
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED | \
		      PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

static int board_type = -1;
#define BOARD_IS_MARSBOARD	0
#define BOARD_IS_RIOTBOARD	1

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

static iomux_v3_cfg_t const uart2_pads[] = {
	MX6_PAD_EIM_D26__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D27__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
}

iomux_v3_cfg_t const enet_pads[] = {
	/* AR8035 PHY Reset */
	MX6_PAD_EIM_D31__GPIO3_IO31 | MUX_PAD_CTRL(ENET_PAD_CTRL_PD),
	/* AR8035 PHY Interrupt */
	MX6_PAD_ENET_TX_EN__GPIO1_IO28 | MUX_PAD_CTRL(ENET_PAD_CTRL),
};

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));

	/* Reset AR8035 PHY */
	gpio_direction_output(IMX_GPIO_NR(3, 31) , 0);
	mdelay(2);
	gpio_set_value(IMX_GPIO_NR(3, 31), 1);
}

int mx6_rgmii_rework(struct phy_device *phydev)
{
	/* from linux/arch/arm/mach-imx/mach-imx6q.c :
	 * Ar803x phy SmartEEE feature cause link status generates glitch,
	 * which cause ethernet link down/up issue, so disable SmartEEE
	 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x805d);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4003);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#ifdef CONFIG_MXC_SPI
iomux_v3_cfg_t const ecspi1_pads[] = {
	MX6_PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_EB2__GPIO2_IO30 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? (IMX_GPIO_NR(2, 30)) : -1;
}

static void setup_spi(void)
{
	imx_iomux_v3_setup_multiple_pads(ecspi1_pads, ARRAY_SIZE(ecspi1_pads));
}
#endif

struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_CSI0_DAT9__I2C1_SCL
				| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_CSI0_DAT9__GPIO5_IO27
				| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		.i2c_mode = MX6_PAD_CSI0_DAT8__I2C1_SDA
				| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_CSI0_DAT8__GPIO5_IO26
				| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 26)
	}
};

struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6_PAD_KEY_COL3__I2C2_SCL
				| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_KEY_COL3__GPIO4_IO12
				| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6_PAD_KEY_ROW3__I2C2_SDA
				| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_KEY_ROW3__GPIO4_IO13
				| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 13)
	}
};

struct i2c_pads_info i2c_pad_info3 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO_5__I2C3_SCL
				| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_GPIO_5__GPIO1_IO05
				| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 5)
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO_6__I2C3_SDA
				| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_GPIO_6__GPIO1_IO06
				| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 6)
	}
};

iomux_v3_cfg_t const tft_pads_riot[] = {
	/* LCD_PWR_EN */
	MX6_PAD_ENET_TXD1__GPIO1_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* TOUCH_INT */
	MX6_PAD_NANDF_CS1__GPIO6_IO14 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LED_PWR_EN */
	MX6_PAD_NANDF_CS2__GPIO6_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* BL LEVEL */
	MX6_PAD_SD1_CMD__GPIO1_IO18 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

iomux_v3_cfg_t const tft_pads_mars[] = {
	/* LCD_PWR_EN */
	MX6_PAD_ENET_TXD1__GPIO1_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* TOUCH_INT */
	MX6_PAD_NANDF_CS1__GPIO6_IO14 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LED_PWR_EN */
	MX6_PAD_NANDF_CS2__GPIO6_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* BL LEVEL (PWM4) */
	MX6_PAD_SD4_DAT2__GPIO2_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

#if defined(CONFIG_VIDEO_IPUV3)

static void enable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)
				IOMUXC_BASE_ADDR;
	setbits_le32(&iomux->gpr[2],
		     IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT);
	/* set backlight level to ON */
	if (board_type == BOARD_IS_RIOTBOARD)
		gpio_direction_output(IMX_GPIO_NR(1, 18) , 1);
	else if (board_type == BOARD_IS_MARSBOARD)
		gpio_direction_output(IMX_GPIO_NR(2, 10) , 1);
}

static void disable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* set backlight level to OFF */
	if (board_type == BOARD_IS_RIOTBOARD)
		gpio_direction_output(IMX_GPIO_NR(1, 18) , 0);
	else if (board_type == BOARD_IS_MARSBOARD)
		gpio_direction_output(IMX_GPIO_NR(2, 10) , 0);

	clrbits_le32(&iomux->gpr[2],
		     IOMUXC_GPR2_LVDS_CH0_MODE_MASK);
}

static void do_enable_hdmi(struct display_info_t const *dev)
{
	disable_lvds(dev);
	imx_enable_hdmi_phy();
}

static int detect_i2c(struct display_info_t const *dev)
{
	return (0 == i2c_set_bus_num(dev->bus)) &&
		(0 == i2c_probe(dev->addr));
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
	.bus	= 2,
	.addr	= 0x1,
	.pixfmt	= IPU_PIX_FMT_LVDS666,
	.detect	= detect_i2c,
	.enable	= enable_lvds,
	.mode	= {
		.name           = "LCD8000-97C",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 100,
		.right_margin   = 200,
		.upper_margin   = 10,
		.lower_margin   = 20,
		.hsync_len      = 20,
		.vsync_len      = 8,
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

	/* Turn on LDB0, IPU,IPU DI0 clocks */
	setbits_le32(&mxc_ccm->CCGR3,
		     MXC_CCM_CCGR3_LDB_DI0_MASK);

	/* set LDB0 clk select to 011/011 */
	clrsetbits_le32(&mxc_ccm->cs2cdr,
			MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK,
			(3 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET));

	setbits_le32(&mxc_ccm->cscmr2,
		     MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV);

	setbits_le32(&mxc_ccm->chsccdr,
		     (CHSCCDR_CLK_SEL_LDB_DI0
		     << MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET));

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	     | IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
	     | IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
	     | IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
	     | IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	     | IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
	     | IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED
	     | IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	clrsetbits_le32(&iomux->gpr[3],
			IOMUXC_GPR3_LVDS0_MUX_CTL_MASK |
			IOMUXC_GPR3_HDMI_MUX_CTL_MASK,
			IOMUXC_GPR3_MUX_SRC_IPU1_DI0
			<< IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
}
#endif /* CONFIG_VIDEO_IPUV3 */

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_early_init_f(void)
{
	u32 cputype = cpu_type(get_cpu_rev());

	switch (cputype) {
	case MXC_CPU_MX6SOLO:
		board_type = BOARD_IS_RIOTBOARD;
		break;
	case MXC_CPU_MX6D:
		board_type = BOARD_IS_MARSBOARD;
		break;
	}

	setup_iomux_uart();

	if (board_type == BOARD_IS_RIOTBOARD)
		imx_iomux_v3_setup_multiple_pads(
			tft_pads_riot, ARRAY_SIZE(tft_pads_riot));
	else if (board_type == BOARD_IS_MARSBOARD)
		imx_iomux_v3_setup_multiple_pads(
			tft_pads_mars, ARRAY_SIZE(tft_pads_mars));
#if defined(CONFIG_VIDEO_IPUV3)
	/* power ON LCD */
	gpio_direction_output(IMX_GPIO_NR(1, 29) , 1);
	/* touch interrupt is an input */
	gpio_direction_input(IMX_GPIO_NR(6, 14));
	/* power ON backlight */
	gpio_direction_output(IMX_GPIO_NR(6, 15) , 1);
	/* set backlight level to off */
	if (board_type == BOARD_IS_RIOTBOARD)
		gpio_direction_output(IMX_GPIO_NR(1, 18) , 0);
	else if (board_type == BOARD_IS_MARSBOARD)
		gpio_direction_output(IMX_GPIO_NR(2, 10) , 0);
	setup_display();
#endif

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;
	/* i2c1 : PMIC, Audio codec on RiOT, Expansion connector on MarS */
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	/* i2c2 : HDMI EDID */
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);
	/* i2c3 : LVDS, Expansion connector */
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info3);
#ifdef CONFIG_MXC_SPI
	setup_spi();
#endif
	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode riotboard_boot_modes[] = {
	{"sd2",	 MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	{"sd3",	 MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{"emmc", MAKE_CFGVAL(0x40, 0x38, 0x00, 0x00)},
	{NULL,	 0},
};
static const struct boot_mode marsboard_boot_modes[] = {
	{"sd2",	 MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	{"emmc", MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{NULL,	 0},
};
#endif

int board_late_init(void)
{
#ifdef CONFIG_CMD_BMODE
	if (board_type == BOARD_IS_RIOTBOARD)
		add_board_boot_modes(riotboard_boot_modes);
	else if (board_type == BOARD_IS_RIOTBOARD)
		add_board_boot_modes(marsboard_boot_modes);
#endif
	setup_iomux_enet();

	return 0;
}

int checkboard(void)
{
	puts("Board: ");
	if (board_type == BOARD_IS_MARSBOARD)
		puts("MarSBoard\n");
	else if (board_type == BOARD_IS_RIOTBOARD)
		puts("RIoTboard\n");
	else
		printf("unknown - cputype : %02x\n", cpu_type(get_cpu_rev()));

	return 0;
}

#ifdef CONFIG_SPL_BUILD
#include <spl.h>

void board_init_f(ulong dummy)
{
	u32 cputype = cpu_type(get_cpu_rev());

	switch (cputype) {
	case MXC_CPU_MX6SOLO:
		board_type = BOARD_IS_RIOTBOARD;
		break;
	case MXC_CPU_MX6D:
		board_type = BOARD_IS_MARSBOARD;
		break;
	}
	arch_cpu_init();

	/* setup GP timer */
	timer_init();

#ifdef CONFIG_SPL_SERIAL
	setup_iomux_uart();
	preloader_console_init();
#endif
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = BOOT_DEVICE_MMC1;
}

/*
 * In order to jump to standard u-boot shell, you have to connect pin 5 of J13
 * to pin 3 (ground).
 */
int spl_start_uboot(void)
{
	int gpio_key = IMX_GPIO_NR(4, 16);

	gpio_direction_input(gpio_key);
	if (gpio_get_value(gpio_key) == 0)
		return 1;
	else
		return 0;
}

#endif
