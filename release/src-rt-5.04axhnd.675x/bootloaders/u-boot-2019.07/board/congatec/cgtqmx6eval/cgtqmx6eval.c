// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
 * Based on mx6qsabrelite.c file
 * Copyright (C) 2013, Adeneo Embedded <www.adeneo-embedded.com>
 * Leo Sartre, <lsartre@adeneo-embedded.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/sata.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <input.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <malloc.h>
#include <miiphy.h>
#include <netdev.h>
#include <micrel.h>
#include <spi_flash.h>
#include <spi.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define SPI_PAD_CTRL (PAD_CTL_HYS |				\
	PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define MX6Q_QMX6_PFUZE_MUX		IMX_GPIO_NR(6, 9)


#define ENET_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED   |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static iomux_v3_cfg_t const uart2_pads[] = {
	IOMUX_PADS(PAD_EIM_D26__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D27__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

#ifndef CONFIG_SPL_BUILD
static iomux_v3_cfg_t const usdhc2_pads[] = {
	IOMUX_PADS(PAD_SD2_CLK__SD2_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_CMD__SD2_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT0__SD2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT1__SD2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT2__SD2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT3__SD2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_4__GPIO1_IO04      | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT4__SD3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT5__SD3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT6__SD3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT7__SD3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_RST__SD3_RESET | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};
#endif

static iomux_v3_cfg_t const usdhc4_pads[] = {
	IOMUX_PADS(PAD_SD4_CLK__SD4_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CMD__SD4_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT0__SD4_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT1__SD4_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT2__SD4_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT3__SD4_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT4__SD4_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT5__SD4_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT6__SD4_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT7__SD4_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D6__GPIO2_IO06    | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static iomux_v3_cfg_t const usb_otg_pads[] = {
	IOMUX_PADS(PAD_EIM_D22__USB_OTG_PWR | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_1__USB_OTG_ID | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static iomux_v3_cfg_t enet_pads_ksz9031[] = {
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TXC__RGMII_TXC | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD0__RGMII_TD0 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD1__RGMII_TD1 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD2__RGMII_TD2 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD3__RGMII_TD3 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TX_CTL__RGMII_TX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_REF_CLK__ENET_TX_CLK | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RXC__GPIO6_IO30 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD0__GPIO6_IO25 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__GPIO6_IO27 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__GPIO6_IO28 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__GPIO6_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__GPIO6_IO24 | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static iomux_v3_cfg_t enet_pads_final_ksz9031[] = {
	IOMUX_PADS(PAD_RGMII_RXC__RGMII_RXC | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD0__RGMII_RD0 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__RGMII_RD1 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__RGMII_RD2 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__RGMII_RD3 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__RGMII_RX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL)),
};

static iomux_v3_cfg_t enet_pads_ar8035[] = {
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TXC__RGMII_TXC | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD0__RGMII_TD0 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD1__RGMII_TD1 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD2__RGMII_TD2 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD3__RGMII_TD3 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TX_CTL__RGMII_TX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_REF_CLK__ENET_TX_CLK | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RXC__RGMII_RXC | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD0__RGMII_RD0 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__RGMII_RD1 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__RGMII_RD2 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__RGMII_RD3 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__RGMII_RX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL)),
};

static iomux_v3_cfg_t const ecspi1_pads[] = {
	IOMUX_PADS(PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D19__GPIO3_IO19 | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
struct i2c_pads_info mx6q_i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6Q_PAD_KEY_COL3__I2C2_SCL | PC,
		.gpio_mode = MX6Q_PAD_KEY_COL3__GPIO4_IO12 | PC,
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_KEY_ROW3__I2C2_SDA | PC,
		.gpio_mode = MX6Q_PAD_KEY_ROW3__GPIO4_IO13 | PC,
		.gp = IMX_GPIO_NR(4, 13)
	}
};

struct i2c_pads_info mx6dl_i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6DL_PAD_KEY_COL3__I2C2_SCL | PC,
		.gpio_mode = MX6DL_PAD_KEY_COL3__GPIO4_IO12 | PC,
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6DL_PAD_KEY_ROW3__I2C2_SDA | PC,
		.gpio_mode = MX6DL_PAD_KEY_ROW3__GPIO4_IO13 | PC,
		.gp = IMX_GPIO_NR(4, 13)
	}
};

#define I2C_PMIC	1	/* I2C2 port is used to connect to the PMIC */

struct interface_level {
	char *name;
	uchar value;
};

static struct interface_level mipi_levels[] = {
	{"0V0", 0x00},
	{"2V5", 0x17},
};

/* setup board specific PMIC */
int power_init_board(void)
{
	struct pmic *p;
	u32 id1, id2, i;
	int ret;
	char const *lv_mipi;

	/* configure I2C multiplexer */
	gpio_direction_output(MX6Q_QMX6_PFUZE_MUX, 1);

	power_pfuze100_init(I2C_PMIC);
	p = pmic_get("PFUZE100");
	if (!p)
		return -EINVAL;

	ret = pmic_probe(p);
	if (ret)
		return ret;

	pmic_reg_read(p, PFUZE100_DEVICEID, &id1);
	pmic_reg_read(p, PFUZE100_REVID, &id2);
	printf("PFUZE100 Rev. [%02x/%02x] detected\n", id1, id2);

	if (id2 >= 0x20)
		return 0;

	/* set level of MIPI if specified */
	lv_mipi = env_get("lv_mipi");
	if (lv_mipi)
		return 0;

	for (i = 0; i < ARRAY_SIZE(mipi_levels); i++) {
		if (!strcmp(mipi_levels[i].name, lv_mipi)) {
			printf("set MIPI level %s\n", mipi_levels[i].name);
			ret = pmic_reg_write(p, PFUZE100_VGEN4VOL,
					     mipi_levels[i].value);
			if (ret)
				return ret;
		}
	}

	return 0;
}

int board_eth_init(bd_t *bis)
{
	struct phy_device *phydev;
	struct mii_dev *bus;
	unsigned short id1, id2;
	int ret;

	/* check whether KSZ9031 or AR8035 has to be configured */
	SETUP_IOMUX_PADS(enet_pads_ar8035);

	/* phy reset */
	gpio_direction_output(IMX_GPIO_NR(3, 23), 0);
	udelay(2000);
	gpio_set_value(IMX_GPIO_NR(3, 23), 1);
	udelay(500);

	bus = fec_get_miibus(IMX_FEC_BASE, -1);
	if (!bus)
		return -EINVAL;
	phydev = phy_find_by_mask(bus, (0xf << 4), PHY_INTERFACE_MODE_RGMII);
	if (!phydev) {
		printf("Error: phy device not found.\n");
		ret = -ENODEV;
		goto free_bus;
	}

	/* get the PHY id */
	id1 = phy_read(phydev, MDIO_DEVAD_NONE, 2);
	id2 = phy_read(phydev, MDIO_DEVAD_NONE, 3);

	if ((id1 == 0x22) && ((id2 & 0xFFF0) == 0x1620)) {
		/* re-configure for Micrel KSZ9031 */
		printf("configure Micrel KSZ9031 Ethernet Phy at address %d\n",
		       phydev->addr);

		/* phy reset: gpio3-23 */
		gpio_set_value(IMX_GPIO_NR(3, 23), 0);
		gpio_set_value(IMX_GPIO_NR(6, 30), (phydev->addr >> 2));
		gpio_set_value(IMX_GPIO_NR(6, 25), 1);
		gpio_set_value(IMX_GPIO_NR(6, 27), 1);
		gpio_set_value(IMX_GPIO_NR(6, 28), 1);
		gpio_set_value(IMX_GPIO_NR(6, 29), 1);
		SETUP_IOMUX_PADS(enet_pads_ksz9031);
		gpio_set_value(IMX_GPIO_NR(6, 24), 1);
		udelay(500);
		gpio_set_value(IMX_GPIO_NR(3, 23), 1);
		SETUP_IOMUX_PADS(enet_pads_final_ksz9031);
	} else if ((id1 == 0x004d) && (id2 == 0xd072)) {
		/* configure Atheros AR8035 - actually nothing to do */
		printf("configure Atheros AR8035 Ethernet Phy at address %d\n",
		       phydev->addr);
	} else {
		printf("Unknown Ethernet-Phy: 0x%04x 0x%04x\n", id1, id2);
		ret = -EINVAL;
		goto free_phydev;
	}

	ret = fec_probe(bis, -1, IMX_FEC_BASE, bus, phydev);
	if (ret)
		goto free_phydev;

	return 0;

free_phydev:
	free(phydev);
free_bus:
	free(bus);
	return ret;
}

int mx6_rgmii_rework(struct phy_device *phydev)
{
	unsigned short id1, id2;
	unsigned short val;

	/* check whether KSZ9031 or AR8035 has to be configured */
	id1 = phy_read(phydev, MDIO_DEVAD_NONE, 2);
	id2 = phy_read(phydev, MDIO_DEVAD_NONE, 3);

	if ((id1 == 0x22) && ((id2 & 0xFFF0) == 0x1620)) {
		/* finalize phy configuration for Micrel KSZ9031 */
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, 2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 4);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_POST_INC_W | 0x2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0x0000);

		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, 2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 5);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_POST_INC_W | 0x2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, MII_KSZ9031_MOD_REG);

		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, 2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 6);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_POST_INC_W | 0x2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0xFFFF);

		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, 2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 8);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_POST_INC_W | 0x2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0x3FFF);

		/* fix KSZ9031 link up issue */
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, 0x0);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0x4);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_NO_POST_INC);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0x6);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_REG);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0x3);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_NO_POST_INC);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0x1A80);
	}

	if ((id1 == 0x004d) && (id2 == 0xd072)) {
		/* enable AR8035 ouput a 125MHz clk from CLK_25M */
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, 0x7);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, MII_KSZ9031_MOD_DATA_POST_INC_RW | 0x16);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_NO_POST_INC | 0x7);
		val = phy_read(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA);
		val &= 0xfe63;
		val |= 0x18;
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, val);

		/* introduce tx clock delay */
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
		val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
		val |= 0x0100;
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

		/* disable hibernation */
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0xb);
		val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x3c40);
	}
	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
 
static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart2_pads);
}

#ifdef CONFIG_MXC_SPI
static void setup_spi(void)
{
	SETUP_IOMUX_PADS(ecspi1_pads);
	gpio_direction_output(IMX_GPIO_NR(3, 19), 0);
}
#endif

#ifdef CONFIG_FSL_ESDHC
static struct fsl_esdhc_cfg usdhc_cfg[] = {
	{USDHC2_BASE_ADDR},
	{USDHC3_BASE_ADDR},
	{USDHC4_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC2_BASE_ADDR:
		gpio_direction_input(IMX_GPIO_NR(1, 4));
		ret = !gpio_get_value(IMX_GPIO_NR(1, 4));
		break;
	case USDHC3_BASE_ADDR:
		ret = 1;	/* eMMC is always present */
		break;
	case USDHC4_BASE_ADDR:
		gpio_direction_input(IMX_GPIO_NR(2, 6));
		ret = !gpio_get_value(IMX_GPIO_NR(2, 6));
		break;
	default:
		printf("Bad USDHC interface\n");
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
#ifndef CONFIG_SPL_BUILD
	s32 status = 0;
	int i;

	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);

	SETUP_IOMUX_PADS(usdhc2_pads);
	SETUP_IOMUX_PADS(usdhc3_pads);
	SETUP_IOMUX_PADS(usdhc4_pads);

	for (i = 0; i < ARRAY_SIZE(usdhc_cfg); i++) {
		status = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (status)
			return status;
	}

	return 0;
#else
	SETUP_IOMUX_PADS(usdhc4_pads);
	usdhc_cfg[0].esdhc_base = USDHC4_BASE_ADDR;
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
	gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;

	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
#endif
}
#endif

int board_ehci_hcd_init(int port)
{
	switch (port) {
	case 0:
		SETUP_IOMUX_PADS(usb_otg_pads);
		/*
		 * set daisy chain for otg_pin_id on 6q.
		 * for 6dl, this bit is reserved
		 */
		imx_iomux_set_gpr_register(1, 13, 1, 1);
		break;
	case 1:
		/* nothing to do */
		break;
	default:
		printf("Invalid USB port: %d\n", port);
		return -EINVAL;
	}

	return 0;
}

int board_ehci_power(int port, int on)
{
	switch (port) {
	case 0:
		break;
	case 1:
		gpio_direction_output(IMX_GPIO_NR(5, 5), on);
		break;
	default:
		printf("Invalid USB port: %d\n", port);
		return -EINVAL;
	}

	return 0;
}

struct display_info_t {
	int bus;
	int addr;
	int pixfmt;
	int (*detect)(struct display_info_t const *dev);
	void (*enable)(struct display_info_t const *dev);
	struct fb_videomode mode;
};

static void disable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	clrbits_le32(&iomux->gpr[2], IOMUXC_GPR2_LVDS_CH0_MODE_MASK |
		     IOMUXC_GPR2_LVDS_CH1_MODE_MASK);
}

static void do_enable_hdmi(struct display_info_t const *dev)
{
	disable_lvds(dev);
	imx_enable_hdmi_phy();
}

static struct display_info_t const displays[] = {
{
	.bus = -1,
	.addr = 0,
	.pixfmt = IPU_PIX_FMT_RGB666,
	.detect = NULL,
	.enable = NULL,
	.mode = {
		.name =
		"Hannstar-XGA",
		.refresh = 60,
		.xres = 1024,
		.yres = 768,
		.pixclock = 15385,
		.left_margin = 220,
		.right_margin = 40,
		.upper_margin = 21,
		.lower_margin = 7,
		.hsync_len = 60,
		.vsync_len = 10,
		.sync = FB_SYNC_EXT,
		.vmode = FB_VMODE_NONINTERLACED } },
{
	.bus = -1,
	.addr = 0,
	.pixfmt = IPU_PIX_FMT_RGB24,
	.detect = NULL,
	.enable = do_enable_hdmi,
	.mode = {
		.name = "HDMI",
		.refresh = 60,
		.xres = 1024,
		.yres = 768,
		.pixclock = 15385,
		.left_margin = 220,
		.right_margin = 40,
		.upper_margin = 21,
		.lower_margin = 7,
		.hsync_len = 60,
		.vsync_len = 10,
		.sync = FB_SYNC_EXT,
		.vmode = FB_VMODE_NONINTERLACED } }
};

int board_video_skip(void)
{
	int i;
	int ret;
	char const *panel = env_get("panel");
	if (!panel) {
		for (i = 0; i < ARRAY_SIZE(displays); i++) {
			struct display_info_t const *dev = displays + i;
			if (dev->detect && dev->detect(dev)) {
				panel = dev->mode.name;
				printf("auto-detected panel %s\n", panel);
				break;
			}
		}
		if (!panel) {
			panel = displays[0].mode.name;
			printf("No panel detected: default to %s\n", panel);
			i = 0;
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(displays); i++) {
			if (!strcmp(panel, displays[i].mode.name))
				break;
		}
	}
	if (i < ARRAY_SIZE(displays)) {
		ret = ipuv3_fb_init(&displays[i].mode, 0, displays[i].pixfmt);
		if (!ret) {
			if (displays[i].enable)
				displays[i].enable(displays + i);
			printf("Display: %s (%ux%u)\n",
			       displays[i].mode.name, displays[i].mode.xres,
			       displays[i].mode.yres);
		} else
			printf("LCD %s cannot be configured: %d\n",
			       displays[i].mode.name, ret);
	} else {
		printf("unsupported panel %s\n", panel);
		return -EINVAL;
	}

	return 0;
}

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	enable_ipu_clock();
	imx_setup_hdmi();

	/* Turn on LDB0, LDB1, IPU,IPU DI0 clocks */
	setbits_le32(&mxc_ccm->CCGR3, MXC_CCM_CCGR3_LDB_DI0_MASK |
		     MXC_CCM_CCGR3_LDB_DI1_MASK);

	/* set LDB0, LDB1 clk select to 011/011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK |
		 MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET) |
		(3 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	setbits_le32(&mxc_ccm->cscmr2, MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV |
		     MXC_CCM_CSCMR2_LDB_DI1_IPU_DIV);

	setbits_le32(&mxc_ccm->chsccdr, CHSCCDR_CLK_SEL_LDB_DI0 <<
		     MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET |
		     CHSCCDR_CLK_SEL_LDB_DI0 <<
		     MXC_CCM_CHSCCDR_IPU1_DI1_CLK_SEL_OFFSET);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
		| IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW
		| IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
		| IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
		| IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
		| IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
		| IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
		| IOMUXC_GPR2_LVDS_CH0_MODE_DISABLED
		| IOMUXC_GPR2_LVDS_CH1_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~(IOMUXC_GPR3_LVDS1_MUX_CTL_MASK |
		       IOMUXC_GPR3_HDMI_MUX_CTL_MASK)) |
		(IOMUXC_GPR3_MUX_SRC_IPU1_DI0 <<
		 IOMUXC_GPR3_LVDS1_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);
}

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
	setup_iomux_uart();
#ifdef CONFIG_MXC_SPI
	setup_spi();
#endif
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;


	if (is_mx6dq())
		setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c_pad_info1);
	else
		setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dl_i2c_pad_info1);

	setup_display();

#ifdef CONFIG_SATA
	setup_sata();
#endif

	return 0;
}

int checkboard(void)
{
	char *type = "unknown";

	if (is_cpu_type(MXC_CPU_MX6Q))
		type = "Quad";
	else if (is_cpu_type(MXC_CPU_MX6D))
		type = "Dual";
	else if (is_cpu_type(MXC_CPU_MX6DL))
		type = "Dual-Lite";
	else if (is_cpu_type(MXC_CPU_MX6SOLO))
		type = "Solo";

	printf("Board: conga-QMX6 %s\n", type);

	return 0;
}

#ifdef CONFIG_MXC_SPI
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? (IMX_GPIO_NR(3, 19)) : -EINVAL;
}
#endif

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0",	MAKE_CFGVAL(0x50, 0x20, 0x00, 0x00)},
	{"mmc1",	MAKE_CFGVAL(0x50, 0x38, 0x00, 0x00)},
	{NULL,		0},
};
#endif

int misc_init_r(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif
	return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	if (is_mx6dq())
		env_set("board_rev", "MX6Q");
	else
		env_set("board_rev", "MX6DL");
#endif

	return 0;
}

#ifdef CONFIG_SPL_BUILD
#include <asm/arch/mx6-ddr.h>
#include <spl.h>
#include <linux/libfdt.h>
#include <spi_flash.h>
#include <spi.h>

const struct mx6dq_iomux_ddr_regs mx6q_ddr_ioregs = {
	.dram_sdclk_0 =  0x00000030,
	.dram_sdclk_1 =  0x00000030,
	.dram_cas =  0x00000030,
	.dram_ras =  0x00000030,
	.dram_reset =  0x00000030,
	.dram_sdcke0 =  0x00003000,
	.dram_sdcke1 =  0x00003000,
	.dram_sdba2 =  0x00000000,
	.dram_sdodt0 =  0x00000030,
	.dram_sdodt1 =  0x00000030,
	.dram_sdqs0 =  0x00000030,
	.dram_sdqs1 =  0x00000030,
	.dram_sdqs2 =  0x00000030,
	.dram_sdqs3 =  0x00000030,
	.dram_sdqs4 =  0x00000030,
	.dram_sdqs5 =  0x00000030,
	.dram_sdqs6 =  0x00000030,
	.dram_sdqs7 =  0x00000030,
	.dram_dqm0 =  0x00000030,
	.dram_dqm1 =  0x00000030,
	.dram_dqm2 =  0x00000030,
	.dram_dqm3 =  0x00000030,
	.dram_dqm4 =  0x00000030,
	.dram_dqm5 =  0x00000030,
	.dram_dqm6 =  0x00000030,
	.dram_dqm7 =  0x00000030,
};

static const struct mx6sdl_iomux_ddr_regs mx6dl_ddr_ioregs = {
	.dram_sdclk_0 = 0x00000030,
	.dram_sdclk_1 = 0x00000030,
	.dram_cas =	0x00000030,
	.dram_ras =	0x00000030,
	.dram_reset =	0x00000030,
	.dram_sdcke0 =	0x00003000,
	.dram_sdcke1 =	0x00003000,
	.dram_sdba2 =	0x00000000,
	.dram_sdodt0 =	0x00000030,
	.dram_sdodt1 =	0x00000030,
	.dram_sdqs0 =	0x00000030,
	.dram_sdqs1 =	0x00000030,
	.dram_sdqs2 =	0x00000030,
	.dram_sdqs3 =	0x00000030,
	.dram_sdqs4 =	0x00000030,
	.dram_sdqs5 =	0x00000030,
	.dram_sdqs6 =	0x00000030,
	.dram_sdqs7 =	0x00000030,
	.dram_dqm0 =	0x00000030,
	.dram_dqm1 =	0x00000030,
	.dram_dqm2 =	0x00000030,
	.dram_dqm3 =	0x00000030,
	.dram_dqm4 =	0x00000030,
	.dram_dqm5 =	0x00000030,
	.dram_dqm6 =	0x00000030,
	.dram_dqm7 =	0x00000030,
};

const struct mx6dq_iomux_grp_regs mx6q_grp_ioregs = {
	.grp_ddr_type =  0x000C0000,
	.grp_ddrmode_ctl =  0x00020000,
	.grp_ddrpke =  0x00000000,
	.grp_addds =  0x00000030,
	.grp_ctlds =  0x00000030,
	.grp_ddrmode =  0x00020000,
	.grp_b0ds =  0x00000030,
	.grp_b1ds =  0x00000030,
	.grp_b2ds =  0x00000030,
	.grp_b3ds =  0x00000030,
	.grp_b4ds =  0x00000030,
	.grp_b5ds =  0x00000030,
	.grp_b6ds =  0x00000030,
	.grp_b7ds =  0x00000030,
};

static const struct mx6sdl_iomux_grp_regs mx6sdl_grp_ioregs = {
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_addds = 0x00000030,
	.grp_ctlds = 0x00000030,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = 0x00000030,
	.grp_b1ds = 0x00000030,
	.grp_b2ds = 0x00000030,
	.grp_b3ds = 0x00000030,
	.grp_b4ds = 0x00000030,
	.grp_b5ds = 0x00000030,
	.grp_b6ds = 0x00000030,
	.grp_b7ds = 0x00000030,
};

const struct mx6_mmdc_calibration mx6q_mmcd_calib = {
	.p0_mpwldectrl0 =  0x0016001A,
	.p0_mpwldectrl1 =  0x0023001C,
	.p1_mpwldectrl0 =  0x0028003A,
	.p1_mpwldectrl1 =  0x001F002C,
	.p0_mpdgctrl0 =  0x43440354,
	.p0_mpdgctrl1 =  0x033C033C,
	.p1_mpdgctrl0 =  0x43300368,
	.p1_mpdgctrl1 =  0x03500330,
	.p0_mprddlctl =  0x3228242E,
	.p1_mprddlctl =  0x2C2C2636,
	.p0_mpwrdlctl =  0x36323A38,
	.p1_mpwrdlctl =  0x42324440,
};

const struct mx6_mmdc_calibration mx6q_2g_mmcd_calib = {
	.p0_mpwldectrl0 =  0x00080016,
	.p0_mpwldectrl1 =  0x001D0016,
	.p1_mpwldectrl0 =  0x0018002C,
	.p1_mpwldectrl1 =  0x000D001D,
	.p0_mpdgctrl0 =    0x43200334,
	.p0_mpdgctrl1 =    0x0320031C,
	.p1_mpdgctrl0 =    0x0344034C,
	.p1_mpdgctrl1 =    0x03380314,
	.p0_mprddlctl =    0x3E36383A,
	.p1_mprddlctl =    0x38363240,
	.p0_mpwrdlctl =	   0x36364238,
	.p1_mpwrdlctl =    0x4230423E,
};

static const struct mx6_mmdc_calibration mx6s_mmcd_calib = {
	.p0_mpwldectrl0 =  0x00480049,
	.p0_mpwldectrl1 =  0x00410044,
	.p0_mpdgctrl0 =    0x42480248,
	.p0_mpdgctrl1 =    0x023C023C,
	.p0_mprddlctl =    0x40424644,
	.p0_mpwrdlctl =    0x34323034,
};

const struct mx6_mmdc_calibration mx6dl_mmcd_calib = {
	.p0_mpwldectrl0 =  0x0043004B,
	.p0_mpwldectrl1 =  0x003A003E,
	.p1_mpwldectrl0 =  0x0047004F,
	.p1_mpwldectrl1 =  0x004E0061,
	.p0_mpdgctrl0 =    0x42500250,
	.p0_mpdgctrl1 =	   0x0238023C,
	.p1_mpdgctrl0 =    0x42640264,
	.p1_mpdgctrl1 =    0x02500258,
	.p0_mprddlctl =    0x40424846,
	.p1_mprddlctl =    0x46484842,
	.p0_mpwrdlctl =    0x38382C30,
	.p1_mpwrdlctl =    0x34343430,
};

static struct mx6_ddr3_cfg mem_ddr_2g = {
	.mem_speed = 1600,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1310,
	.trcmin = 4875,
	.trasmin = 3500,
};

static struct mx6_ddr3_cfg mem_ddr_4g = {
	.mem_speed = 1600,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1310,
	.trcmin = 4875,
	.trasmin = 3500,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

/* Define a minimal structure so that the part number can be read via SPL */
struct mfgdata {
	unsigned char tsize;
	/* size of checksummed part in bytes */
	unsigned char ckcnt;
	/* checksum corrected byte */
	unsigned char cksum;
	/* decimal serial number, packed BCD */
	unsigned char serial[6];
	 /* part number, right justified, ASCII */
	unsigned char pn[16];
};

static void conv_ascii(unsigned char *dst, unsigned char *src, int len)
{
	int remain = len;
	unsigned char *sptr = src;
	unsigned char *dptr = dst;

	while (remain) {
		if (*sptr) {
			*dptr = *sptr;
			dptr++;
		}
		sptr++;
		remain--;
	}
	*dptr = 0x0;
}

#define CFG_MFG_ADDR_OFFSET	(spi->size - SZ_16K)
static bool is_2gb(void)
{
	struct spi_flash *spi;
	int ret;
	char buf[sizeof(struct mfgdata)];
	struct mfgdata *data = (struct mfgdata *)buf;
	unsigned char outbuf[32];

	spi = spi_flash_probe(CONFIG_ENV_SPI_BUS,
			      CONFIG_ENV_SPI_CS,
			      CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	ret = spi_flash_read(spi, CFG_MFG_ADDR_OFFSET, sizeof(struct mfgdata),
			     buf);
	if (ret)
		return false;

	/* Congatec Part Numbers 104 and 105 have 2GiB of RAM */
	conv_ascii(outbuf, data->pn, sizeof(data->pn));
	if (!memcmp(outbuf, "016104", 6) || !memcmp(outbuf, "016105", 6))
		return true;
	else
		return false;
}

static void spl_dram_init(int width)
{
	struct mx6_ddr_sysinfo sysinfo = {
		/* width of data bus:0=16,1=32,2=64 */
		.dsize = width / 32,
		/* config for full 4GB range so that get_mem_size() works */
		.cs_density = 32, /* 32Gb per CS */
		/* single chip select */
		.ncs = 1,
		.cs1_mirror = 0,
		.rtt_wr = 2,
		.rtt_nom = 2,
		.walat = 0,
		.ralat = 5,
		.mif3_mode = 3,
		.bi_on = 1,
		.sde_to_rst = 0x0d,
		.rst_to_cke = 0x20,
		.refsel = 1,	/* Refresh cycles at 32KHz */
		.refr = 7,	/* 8 refresh commands per refresh cycle */
	};

	if (is_cpu_type(MXC_CPU_MX6Q) && is_2gb()) {
		mx6dq_dram_iocfg(width, &mx6q_ddr_ioregs, &mx6q_grp_ioregs);
		mx6_dram_cfg(&sysinfo, &mx6q_2g_mmcd_calib, &mem_ddr_4g);
		return;
	}

	if (is_mx6dq()) {
		mx6dq_dram_iocfg(width, &mx6q_ddr_ioregs, &mx6q_grp_ioregs);
		mx6_dram_cfg(&sysinfo, &mx6q_mmcd_calib, &mem_ddr_2g);
	} else if (is_cpu_type(MXC_CPU_MX6SOLO)) {
		sysinfo.walat = 1;
		mx6sdl_dram_iocfg(width, &mx6dl_ddr_ioregs, &mx6sdl_grp_ioregs);
		mx6_dram_cfg(&sysinfo, &mx6s_mmcd_calib, &mem_ddr_4g);
	} else if (is_cpu_type(MXC_CPU_MX6DL)) {
		sysinfo.walat = 1;
		mx6sdl_dram_iocfg(width, &mx6dl_ddr_ioregs, &mx6sdl_grp_ioregs);
		mx6_dram_cfg(&sysinfo, &mx6dl_mmcd_calib, &mem_ddr_2g);
	}
}

void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* iomux and setup of i2c */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* Needed for malloc() to work in SPL prior to board_init_r() */
	spl_init();

	/* DDR initialization */
	if (is_cpu_type(MXC_CPU_MX6SOLO))
		spl_dram_init(32);
	else
		spl_dram_init(64);

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
#endif
