// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/sys_proto.h>
#include <errno.h>
#include <asm/gpio.h>
#include <malloc.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/spi.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <i2c.h>
#include <environment.h>

#include <dm.h>
#include <dm/platform_data/serial_mxc.h>
#include <dm/platdata.h>

#ifndef CONFIG_MXC_SPI
#error "CONFIG_SPI must be set for this board"
#error "Please check your config file"
#endif

#include "common.h"

DECLARE_GLOBAL_DATA_PTR;

static bool hw_ids_valid;
static bool sw_ids_valid;
static u32 cpu_id;
static u32 unit_id;

#define EM_PAD IMX_GPIO_NR(3, 29)
#define SW0	IMX_GPIO_NR(2, 4)
#define SW1	IMX_GPIO_NR(2, 5)
#define SW2	IMX_GPIO_NR(2, 6)
#define SW3	IMX_GPIO_NR(2, 7)
#define HW0	IMX_GPIO_NR(6, 7)
#define HW1	IMX_GPIO_NR(6, 9)
#define HW2	IMX_GPIO_NR(6, 10)
#define HW3	IMX_GPIO_NR(6, 11)
#define HW4	IMX_GPIO_NR(4, 7)
#define HW5	IMX_GPIO_NR(4, 11)
#define HW6	IMX_GPIO_NR(4, 13)
#define HW7	IMX_GPIO_NR(4, 15)

int gpio_table_sw_ids[] = {
	SW0, SW1, SW2, SW3
};

const char *gpio_table_sw_ids_names[] = {
	"sw0", "sw1", "sw2", "sw3"
};

int gpio_table_hw_ids[] = {
	HW0, HW1, HW2, HW3, HW4, HW5, HW6, HW7
};

const char *gpio_table_hw_ids_names[] = {
	"hw0", "hw1", "hw2", "hw3", "hw4", "hw5", "hw6", "hw7"
};

static int get_board_id(int *ids, const char **c, int size,
			bool *valid, u32 *id)
{
	int i, ret, val;

	*valid = false;

	for (i = 0; i < size; i++) {
		ret = gpio_request(ids[i], c[i]);
		if (ret) {
			printf("Can't request SWx gpios\n");
			return ret;
		}
	}

	for (i = 0; i < size; i++) {
		ret = gpio_direction_input(ids[i]);
		if (ret) {
			printf("Can't set SWx gpios direction\n");
			return ret;
		}
	}

	for (i = 0; i < size; i++) {
		val = gpio_get_value(ids[i]);
		if (val < 0) {
			printf("Can't get SW%d ID\n", i);
			*id = 0;
			return val;
		}
		*id |= val << i;
	}
	*valid = true;

	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

#define PC	MUX_PAD_CTRL(I2C_PAD_CTRL)
/* I2C1: TFA9879 */
struct i2c_pads_info i2c_pad_info0 = {
	.scl = {
		.i2c_mode = MX6_PAD_EIM_D21__I2C1_SCL | PC,
		.gpio_mode = MX6_PAD_EIM_D21__GPIO3_IO21 | PC,
		.gp = IMX_GPIO_NR(3, 21)
	},
	.sda = {
		.i2c_mode = MX6_PAD_EIM_D28__I2C1_SDA | PC,
		.gpio_mode = MX6_PAD_EIM_D28__GPIO3_IO28 | PC,
		.gp = IMX_GPIO_NR(3, 28)
	}
};

/* I2C2: TIVO TM4C123 */
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_EIM_EB2__I2C2_SCL | PC,
		.gpio_mode = MX6_PAD_EIM_EB2__GPIO2_IO30 | PC,
		.gp = IMX_GPIO_NR(2, 30)
	},
	.sda = {
		.i2c_mode = MX6_PAD_EIM_D16__I2C2_SDA | PC,
		.gpio_mode = MX6_PAD_EIM_D16__GPIO3_IO16 | PC,
		.gp = IMX_GPIO_NR(3, 16)
	}
};

/* I2C3: PMIC PF0100, EEPROM AT24C256C */
struct i2c_pads_info i2c_pad_info2 = {
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

iomux_v3_cfg_t const misc_pads[] = {
	/* Prod ID GPIO pins */
	MX6_PAD_NANDF_D4__GPIO2_IO04    | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D5__GPIO2_IO05    | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D6__GPIO2_IO06    | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D7__GPIO2_IO07    | MUX_PAD_CTRL(NO_PAD_CTRL),

	/* HW revision GPIO pins */
	MX6_PAD_NANDF_CLE__GPIO6_IO07   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_WP_B__GPIO6_IO09  | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_RB0__GPIO6_IO10   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_CS0__GPIO6_IO11   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_KEY_ROW0__GPIO4_IO07 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_KEY_ROW2__GPIO4_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_KEY_ROW3__GPIO4_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_KEY_ROW4__GPIO4_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),

	/* XTALOSC */
	MX6_PAD_GPIO_3__XTALOSC_REF_CLK_24M | MUX_PAD_CTRL(NO_PAD_CTRL),

	/* Emergency recovery pin */
	MX6_PAD_EIM_D29__GPIO3_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg usdhc_cfg[1] = {
	{ USDHC4_BASE_ADDR, 0, 8, },
};

int board_mmc_getcd(struct mmc *mmc)
{
	return 1;
}

int board_mmc_init(bd_t *bis)
{
	displ5_set_iomux_usdhc();

	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);

	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
}
#endif /* CONFIG_FSL_ESDHC */

static void displ5_setup_ecspi(void)
{
	int ret;

	displ5_set_iomux_ecspi();

	ret = gpio_request(IMX_GPIO_NR(5, 29), "spi2_cs0");
	if (!ret)
		gpio_direction_output(IMX_GPIO_NR(5, 29), 1);

	ret = gpio_request(IMX_GPIO_NR(7, 0), "spi2_#wp");
	if (!ret)
		gpio_direction_output(IMX_GPIO_NR(7, 0), 1);
}

#ifdef CONFIG_FEC_MXC
iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_ENET_TXD1__ENET_1588_EVENT0_IN	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_RXD1__ENET_1588_EVENT3_OUT | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL),

	/* for old evalboard with R159 present and R160 not populated */
	MX6_PAD_GPIO_16__ENET_REF_CLK		| MUX_PAD_CTRL(NO_PAD_CTRL),

	MX6_PAD_RGMII_TXC__RGMII_TXC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__RGMII_TD0		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__RGMII_TD1		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__RGMII_TD2		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__RGMII_TD3		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),

	MX6_PAD_RGMII_RXC__RGMII_RXC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD0__RGMII_RD0		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD1__RGMII_RD1		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD2__RGMII_RD2		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD3__RGMII_RD3		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	/*INT#_GBE*/
	MX6_PAD_ENET_TX_EN__GPIO1_IO28		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_enet(void)
{
	SETUP_IOMUX_PADS(enet_pads);
	gpio_direction_input(IMX_GPIO_NR(1, 28)); /*INT#_GBE*/
}

static int setup_mac_from_fuse(void)
{
	unsigned char enetaddr[6];
	int ret;

	ret = eth_env_get_enetaddr("ethaddr", enetaddr);
	if (ret)	/* ethaddr is already set */
		return 0;

	imx_get_mac_from_fuse(0, enetaddr);

	if (is_valid_ethaddr(enetaddr)) {
		eth_env_set_enetaddr("ethaddr", enetaddr);
		return 0;
	}

	return 0;
}

int board_eth_init(bd_t *bd)
{
	struct phy_device *phydev;
	struct mii_dev *bus;
	int ret;

	setup_iomux_enet();

	iomuxc_set_rgmii_io_voltage(DDR_SEL_1P5V_IO);

	ret = enable_fec_anatop_clock(0, ENET_125MHZ);
	if (ret)
		return ret;

	setup_mac_from_fuse();

	bus = fec_get_miibus(IMX_FEC_BASE, -1);
	if (!bus)
		return -ENODEV;

	/*
	 * We use here the "rgmii-id" mode of operation and allow M88E1512
	 * PHY to use its internally callibrated RX/TX delays
	 */
	phydev = phy_find_by_mask(bus, 0xffffffff /* (0xf << 4) */,
				  PHY_INTERFACE_MODE_RGMII_ID);
	if (!phydev) {
		ret = -ENODEV;
		goto err_phy;
	}

	/* display5 due to PCB routing can only work with 100 Mbps */
	phydev->advertising &= ~(ADVERTISED_1000baseX_Half |
				 ADVERTISED_1000baseX_Full |
				 SUPPORTED_1000baseT_Half |
				 SUPPORTED_1000baseT_Full);

	ret  = fec_probe(bd, -1, IMX_FEC_BASE, bus, phydev);
	if (ret)
		goto err_sw;

	return 0;

err_sw:
	free(phydev);
err_phy:
	mdio_unregister(bus);
	free(bus);
	return ret;
}
#endif /* CONFIG_FEC_MXC */

/*
 * Do not overwrite the console
 * Always use serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	fdt_fixup_ethernet(blob);
	return 0;
}
#endif

int board_init(void)
{
	debug("board init\n");
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	/* Setup iomux for non console UARTS */
	displ5_set_iomux_uart();

	displ5_setup_ecspi();

	SETUP_IOMUX_PADS(misc_pads);

	get_board_id(gpio_table_sw_ids, &gpio_table_sw_ids_names[0],
		     ARRAY_SIZE(gpio_table_sw_ids), &sw_ids_valid, &unit_id);
	debug("SWx unit_id 0x%x\n", unit_id);

	get_board_id(gpio_table_hw_ids, &gpio_table_hw_ids_names[0],
		     ARRAY_SIZE(gpio_table_hw_ids), &hw_ids_valid, &cpu_id);
	debug("HWx cpu_id 0x%x\n", cpu_id);

	if (hw_ids_valid && sw_ids_valid)
		printf("ID:    unit type 0x%x rev 0x%x\n", unit_id, cpu_id);

	udelay(25);

	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info0);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);

	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* eMMC, USDHC-4, 8-bit bus width */
	/* SPI-NOR, ECSPI-2 SS0, 3-bytes addressing */
	{"emmc",    MAKE_CFGVAL(0x60, 0x58, 0x00, 0x00)},
	{"spinor",  MAKE_CFGVAL(0x30, 0x00, 0x00, 0x09)},
	{NULL,	0},
};

static void setup_boot_modes(void)
{
	add_board_boot_modes(board_boot_modes);
}
#else
static inline void setup_boot_modes(void) {}
#endif

int misc_init_r(void)
{
	int ret;

	setup_boot_modes();

	ret = gpio_request(EM_PAD, "Emergency_PAD");
	if (ret) {
		printf("Can't request emergency PAD gpio\n");
		return ret;
	}

	ret = gpio_direction_input(EM_PAD);
	if (ret) {
		printf("Can't set emergency PAD direction\n");
		return ret;
	}

	return 0;
}

static struct mxc_serial_platdata mxc_serial_plat = {
	.reg = (struct mxc_uart *)UART5_BASE,
};

U_BOOT_DEVICE(mxc_serial) = {
	.name = "serial_mxc",
	.platdata = &mxc_serial_plat,
};
