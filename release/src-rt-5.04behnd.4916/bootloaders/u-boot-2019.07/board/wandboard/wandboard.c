// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2014 O.S. Systems Software LTDA.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/video.h>
#include <asm/mach-imx/sata.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include <common.h>
#include <miiphy.h>
#include <netdev.h>
#include <phy.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define ETH_PHY_RESET		IMX_GPIO_NR(3, 29)
#define ETH_PHY_AR8035_POWER	IMX_GPIO_NR(7, 13)
#define REV_DETECTION		IMX_GPIO_NR(2, 28)

/* Speed defined in Kconfig is only applicable when not using DM_I2C.  */
#ifdef CONFIG_DM_I2C
#define I2C1_SPEED_NON_DM	0
#define I2C2_SPEED_NON_DM	0
#else
#define I2C1_SPEED_NON_DM	CONFIG_SYS_MXC_I2C1_SPEED
#define I2C2_SPEED_NON_DM	CONFIG_SYS_MXC_I2C2_SPEED
#endif

static bool with_pmic;

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
	IOMUX_PADS(PAD_CSI0_DAT10__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT11__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static iomux_v3_cfg_t const enet_pads[] = {
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC    | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TXC__RGMII_TXC  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD0__RGMII_TD0  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD1__RGMII_TD1  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD2__RGMII_TD2  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD3__RGMII_TD3  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TX_CTL__RGMII_TX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_REF_CLK__ENET_TX_CLK  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RXC__RGMII_RXC  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD0__RGMII_RD0  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__RGMII_RD1  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__RGMII_RD2  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__RGMII_RD3  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__RGMII_RX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	/* AR8031 PHY Reset */
	IOMUX_PADS(PAD_EIM_D29__GPIO3_IO29    | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static iomux_v3_cfg_t const enet_ar8035_power_pads[] = {
	/* AR8035 POWER */
	IOMUX_PADS(PAD_GPIO_18__GPIO7_IO13    | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static iomux_v3_cfg_t const rev_detection_pad[] = {
	IOMUX_PADS(PAD_EIM_EB0__GPIO2_IO28  | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart1_pads);
}

static void setup_iomux_enet(void)
{
	SETUP_IOMUX_PADS(enet_pads);

	if (with_pmic) {
		SETUP_IOMUX_PADS(enet_ar8035_power_pads);
		/* enable AR8035 POWER */
		gpio_request(ETH_PHY_AR8035_POWER, "PHY_POWER");
		gpio_direction_output(ETH_PHY_AR8035_POWER, 0);
	}
	/* wait until 3.3V of PHY and clock become stable */
	mdelay(10);

	/* Reset AR8031 PHY */
	gpio_request(ETH_PHY_RESET, "PHY_RESET");
	gpio_direction_output(ETH_PHY_RESET, 0);
	mdelay(10);
	gpio_set_value(ETH_PHY_RESET, 1);
	udelay(100);
}

static int ar8031_phy_fixup(struct phy_device *phydev)
{
	unsigned short val;
	int mask;

	/* To enable AR8031 ouput a 125MHz clk from CLK_25M */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);

	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	if (with_pmic)
		mask = 0xffe7;	/* AR8035 */
	else
		mask = 0xffe3;	/* AR8031 */

	val &= mask;
	val |= 0x18;
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

	/* introduce tx clock delay */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0100;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	ar8031_phy_fixup(phydev);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#if defined(CONFIG_VIDEO_IPUV3)
struct i2c_pads_info mx6q_i2c2_pad_info = {
	.scl = {
		.i2c_mode = MX6Q_PAD_KEY_COL3__I2C2_SCL
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6Q_PAD_KEY_COL3__GPIO4_IO12
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_KEY_ROW3__I2C2_SDA
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6Q_PAD_KEY_ROW3__GPIO4_IO13
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 13)
	}
};

struct i2c_pads_info mx6dl_i2c2_pad_info = {
	.scl = {
		.i2c_mode = MX6DL_PAD_KEY_COL3__I2C2_SCL
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6DL_PAD_KEY_COL3__GPIO4_IO12
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6DL_PAD_KEY_ROW3__I2C2_SDA
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6DL_PAD_KEY_ROW3__GPIO4_IO13
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 13)
	}
};

struct i2c_pads_info mx6q_i2c3_pad_info = {
	.scl = {
		.i2c_mode = MX6Q_PAD_GPIO_5__I2C3_SCL
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6Q_PAD_GPIO_5__GPIO1_IO05
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 5)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_GPIO_16__I2C3_SDA
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6Q_PAD_GPIO_16__GPIO7_IO11
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(7, 11)
	}
};

struct i2c_pads_info mx6dl_i2c3_pad_info = {
	.scl = {
		.i2c_mode = MX6DL_PAD_GPIO_5__I2C3_SCL
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6DL_PAD_GPIO_5__GPIO1_IO05
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 5)
	},
	.sda = {
		.i2c_mode = MX6DL_PAD_GPIO_16__I2C3_SDA
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6DL_PAD_GPIO_16__GPIO7_IO11
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(7, 11)
	}
};

static iomux_v3_cfg_t const fwadapt_7wvga_pads[] = {
	IOMUX_PADS(PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK),
	IOMUX_PADS(PAD_DI0_PIN2__IPU1_DI0_PIN02), /* HSync */
	IOMUX_PADS(PAD_DI0_PIN3__IPU1_DI0_PIN03), /* VSync */
	IOMUX_PADS(PAD_DI0_PIN4__IPU1_DI0_PIN04	| MUX_PAD_CTRL(PAD_CTL_DSE_120ohm)), /* Contrast */
	IOMUX_PADS(PAD_DI0_PIN15__IPU1_DI0_PIN15), /* DISP0_DRDY */
	IOMUX_PADS(PAD_DISP0_DAT0__IPU1_DISP0_DATA00),
	IOMUX_PADS(PAD_DISP0_DAT1__IPU1_DISP0_DATA01),
	IOMUX_PADS(PAD_DISP0_DAT2__IPU1_DISP0_DATA02),
	IOMUX_PADS(PAD_DISP0_DAT3__IPU1_DISP0_DATA03),
	IOMUX_PADS(PAD_DISP0_DAT4__IPU1_DISP0_DATA04),
	IOMUX_PADS(PAD_DISP0_DAT5__IPU1_DISP0_DATA05),
	IOMUX_PADS(PAD_DISP0_DAT6__IPU1_DISP0_DATA06),
	IOMUX_PADS(PAD_DISP0_DAT7__IPU1_DISP0_DATA07),
	IOMUX_PADS(PAD_DISP0_DAT8__IPU1_DISP0_DATA08),
	IOMUX_PADS(PAD_DISP0_DAT9__IPU1_DISP0_DATA09),
	IOMUX_PADS(PAD_DISP0_DAT10__IPU1_DISP0_DATA10),
	IOMUX_PADS(PAD_DISP0_DAT11__IPU1_DISP0_DATA11),
	IOMUX_PADS(PAD_DISP0_DAT12__IPU1_DISP0_DATA12),
	IOMUX_PADS(PAD_DISP0_DAT13__IPU1_DISP0_DATA13),
	IOMUX_PADS(PAD_DISP0_DAT14__IPU1_DISP0_DATA14),
	IOMUX_PADS(PAD_DISP0_DAT15__IPU1_DISP0_DATA15),
	IOMUX_PADS(PAD_DISP0_DAT16__IPU1_DISP0_DATA16),
	IOMUX_PADS(PAD_DISP0_DAT17__IPU1_DISP0_DATA17),
	IOMUX_PADS(PAD_SD4_DAT2__GPIO2_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL)), /* DISP0_BKLEN */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL)), /* DISP0_VDDEN */
};

static void do_enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

static int detect_i2c(struct display_info_t const *dev)
{
#ifdef CONFIG_DM_I2C
	struct udevice *bus, *udev;
	int rc;

	rc = uclass_get_device_by_seq(UCLASS_I2C, dev->bus, &bus);
	if (rc)
		return rc;
	rc = dm_i2c_probe(bus, dev->addr, 0, &udev);
	if (rc)
		return 0;
	return 1;
#else
	return (0 == i2c_set_bus_num(dev->bus)) &&
			(0 == i2c_probe(dev->addr));
#endif
}

static void enable_fwadapt_7wvga(struct display_info_t const *dev)
{
	SETUP_IOMUX_PADS(fwadapt_7wvga_pads);

	gpio_request(IMX_GPIO_NR(2, 10), "DISP0_BKLEN");
	gpio_request(IMX_GPIO_NR(2, 11), "DISP0_VDDEN");
	gpio_direction_output(IMX_GPIO_NR(2, 10), 1);
	gpio_direction_output(IMX_GPIO_NR(2, 11), 1);
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
	.bus	= 1,
	.addr	= 0x10,
	.pixfmt	= IPU_PIX_FMT_RGB666,
	.detect	= detect_i2c,
	.enable	= enable_fwadapt_7wvga,
	.mode	= {
		.name           = "FWBADAPT-LCD-F07A-0102",
		.refresh        = 60,
		.xres           = 800,
		.yres           = 480,
		.pixclock       = 33260,
		.left_margin    = 128,
		.right_margin   = 128,
		.upper_margin   = 22,
		.lower_margin   = 22,
		.hsync_len      = 1,
		.vsync_len      = 1,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} } };
size_t display_count = ARRAY_SIZE(displays);

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;

	enable_ipu_clock();
	imx_setup_hdmi();

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	/* Disable LCD backlight */
	SETUP_IOMUX_PAD(PAD_DI0_PIN4__GPIO4_IO20);
	gpio_request(IMX_GPIO_NR(4, 20), "LCD_BKLEN");
	gpio_direction_input(IMX_GPIO_NR(4, 20));
}
#endif /* CONFIG_VIDEO_IPUV3 */

int board_eth_init(bd_t *bis)
{
	setup_iomux_enet();

	return cpu_eth_init(bis);
}

int board_early_init_f(void)
{
	setup_iomux_uart();
#ifdef CONFIG_SATA
	setup_sata();
#endif

	return 0;
}

#define PMIC_I2C_BUS		2

int power_init_board(void)
{
	struct udevice *dev;
	int reg, ret;

	puts("PMIC:  ");

	ret = pmic_get("pfuze100", &dev);
	if (ret < 0) {
		printf("pmic_get() ret %d\n", ret);
		return 0;
	}

	reg = pmic_reg_read(dev, PFUZE100_DEVICEID);
	if (reg < 0) {
		printf("pmic_reg_read() ret %d\n", reg);
		return 0;
	}
	printf("PMIC:  PFUZE100 ID=0x%02x\n", reg);
	with_pmic = true;

	/* Set VGEN2 to 1.5V and enable */
	reg = pmic_reg_read(dev, PFUZE100_VGEN2VOL);
	reg &= ~(LDO_VOL_MASK);
	reg |= (LDOA_1_50V | (1 << (LDO_EN)));
	pmic_reg_write(dev, PFUZE100_VGEN2VOL, reg);
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

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0",	  MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{"mmc1",	  MAKE_CFGVAL(0x40, 0x20, 0x00, 0x00)},
	{NULL,	 0},
};
#endif

static bool is_revc1(void)
{
	SETUP_IOMUX_PADS(rev_detection_pad);
	gpio_direction_input(REV_DETECTION);

	if (gpio_get_value(REV_DETECTION))
		return true;
	else
		return false;
}

static bool is_revd1(void)
{
	if (with_pmic)
		return true;
	else
		return false;
}

int board_late_init(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	if (is_mx6dqp())
		env_set("board_rev", "MX6QP");
	else if (is_mx6dq())
		env_set("board_rev", "MX6Q");
	else
		env_set("board_rev", "MX6DL");

	if (is_revd1())
		env_set("board_name", "D1");
	else if (is_revc1())
		env_set("board_name", "C1");
	else
		env_set("board_name", "B1");
#endif
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#if defined(CONFIG_VIDEO_IPUV3)
	setup_i2c(1, I2C1_SPEED_NON_DM, 0x7f, &mx6dl_i2c2_pad_info);
	if (is_mx6dq() || is_mx6dqp()) {
		setup_i2c(1, I2C1_SPEED_NON_DM, 0x7f, &mx6q_i2c2_pad_info);
		setup_i2c(2, I2C2_SPEED_NON_DM, 0x7f, &mx6q_i2c3_pad_info);
	} else {
		setup_i2c(1, I2C1_SPEED_NON_DM, 0x7f, &mx6dl_i2c2_pad_info);
		setup_i2c(2, I2C2_SPEED_NON_DM, 0x7f, &mx6dl_i2c3_pad_info);
	}

	setup_display();
#endif

	return 0;
}

int checkboard(void)
{
	gpio_request(REV_DETECTION, "REV_DETECT");

	if (is_revd1())
		puts("Board: Wandboard rev D1\n");
	else if (is_revc1())
		puts("Board: Wandboard rev C1\n");
	else
		puts("Board: Wandboard rev B1\n");

	return 0;
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	if (is_mx6dq()) {
		if (!strcmp(name, "imx6q-wandboard-revb1"))
			return 0;
	} else if (is_mx6dqp()) {
		if (!strcmp(name, "imx6qp-wandboard-revd1"))
			return 0;
	} else if (is_mx6dl() || is_mx6solo()) {
		if (!strcmp(name, "imx6dl-wandboard-revb1"))
			return 0;
	}

	return -EINVAL;
}
#endif
