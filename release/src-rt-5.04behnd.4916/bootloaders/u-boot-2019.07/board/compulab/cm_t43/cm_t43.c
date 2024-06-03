// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Compulab, Ltd.
 */

#include <common.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <asm/gpio.h>
#include <asm/arch/sys_proto.h>
#include <asm/emif.h>
#include <power/pmic.h>
#include <power/tps65218.h>
#include "board.h"
#include <usb.h>
#include <asm/omap_common.h>

DECLARE_GLOBAL_DATA_PTR;

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

/* setup board specific PMIC */
int power_init_board(void)
{
	struct pmic *p;
	uchar tps_status = 0;

	power_tps65218_init(I2C_PMIC);
	p = pmic_get("TPS65218_PMIC");
	if (p && !pmic_probe(p)) {
		puts("PMIC:  TPS65218\n");
		/* We don't care if fseal is locked, but we do need it set */
		tps65218_lock_fseal();
		tps65218_reg_read(TPS65218_STATUS, &tps_status);
		if (!(tps_status & TPS65218_FSEAL))
			printf("WARNING: RTC not backed by battery!\n");
	}

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	gpmc_init();
	set_i2c_pin_mux();
	i2c_init(CONFIG_SYS_OMAP24_I2C_SPEED, CONFIG_SYS_OMAP24_I2C_SLAVE);
	i2c_probe(TPS65218_CHIP_PM);

	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	enable_usb_clocks(index);
	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	disable_usb_clocks(index);
	return 0;
}

#ifdef CONFIG_DRIVER_TI_CPSW

static void cpsw_control(int enabled)
{
	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 0,
		.phy_if		= PHY_INTERFACE_MODE_RGMII,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 1,
		.phy_if		= PHY_INTERFACE_MODE_RGMII,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 2,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};

#define GPIO_PHY1_RST		170
#define GPIO_PHY2_RST		168

int board_phy_config(struct phy_device *phydev)
{
	unsigned short val;

	/* introduce tx clock delay */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0100;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

	if (phydev->drv->config)
		return phydev->drv->config(phydev);

	return 0;
}

static void board_phy_init(void)
{
	set_mdio_pin_mux();
	writel(0x40003, 0x44e10a74); /* Mux pin as clkout2 */
	writel(0x10006, 0x44df4108); /* Select EXTDEV as clock source */
	writel(0x4, 0x44df2e60); /* Set EXTDEV as MNbypass */

	/* For revision A */
	writel(0x2000009, 0x44df2e6c);
	writel(0x38a, 0x44df2e70);

	mdelay(10);

	gpio_request(GPIO_PHY1_RST, "phy1_rst");
	gpio_request(GPIO_PHY2_RST, "phy2_rst");
	gpio_direction_output(GPIO_PHY1_RST, 0);
	gpio_direction_output(GPIO_PHY2_RST, 0);
	mdelay(2);

	gpio_set_value(GPIO_PHY1_RST, 1);
	gpio_set_value(GPIO_PHY2_RST, 1);
	mdelay(2);
}

int board_eth_init(bd_t *bis)
{
	int rv;

	set_rgmii_pin_mux();
	writel(RGMII_MODE_ENABLE | RGMII_INT_DELAY, &cdev->miisel);
	board_phy_init();

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);

	return rv;
}
#endif
