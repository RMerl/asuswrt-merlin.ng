// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for Compulab CM-T335 board
 *
 * Copyright (C) 2013, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Ilya Ledvich <ilya@compulab.co.il>
 */

#include <common.h>
#include <environment.h>
#include <errno.h>
#include <miiphy.h>
#include <cpsw.h>

#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware_am33xx.h>
#include <asm/io.h>
#include <asm/gpio.h>

#include "../common/eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	gpmc_init();

#if defined(CONFIG_LED_STATUS) && defined(CONFIG_LED_STATUS_BOOT_ENABLE)
	status_led_set(CONFIG_LED_STATUS_BOOT, CONFIG_LED_STATUS_OFF);
#endif
	return 0;
}

#if defined (CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)
static void cpsw_control(int enabled)
{
	/* VTP can be added here */
	return;
}

static struct cpsw_slave_data cpsw_slave = {
	.slave_reg_ofs	= 0x208,
	.sliver_reg_ofs	= 0xd80,
	.phy_addr	= 0,
	.phy_if		= PHY_INTERFACE_MODE_RGMII,
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
	.slave_data		= &cpsw_slave,
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

/* PHY reset GPIO */
#define GPIO_PHY_RST		GPIO_PIN(3, 7)

static void board_phy_init(void)
{
	gpio_request(GPIO_PHY_RST, "phy_rst");
	gpio_direction_output(GPIO_PHY_RST, 0);
	mdelay(2);
	gpio_set_value(GPIO_PHY_RST, 1);
	mdelay(2);
}

static void get_efuse_mac_addr(uchar *enetaddr)
{
	uint32_t mac_hi, mac_lo;
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

	mac_lo = readl(&cdev->macid0l);
	mac_hi = readl(&cdev->macid0h);
	enetaddr[0] = mac_hi & 0xFF;
	enetaddr[1] = (mac_hi & 0xFF00) >> 8;
	enetaddr[2] = (mac_hi & 0xFF0000) >> 16;
	enetaddr[3] = (mac_hi & 0xFF000000) >> 24;
	enetaddr[4] = mac_lo & 0xFF;
	enetaddr[5] = (mac_lo & 0xFF00) >> 8;
}

/*
 * Routine: handle_mac_address
 * Description: prepare MAC address for on-board Ethernet.
 */
static int handle_mac_address(void)
{
	uchar enetaddr[6];
	int rv;

	rv = eth_env_get_enetaddr("ethaddr", enetaddr);
	if (rv)
		return 0;

	rv = cl_eeprom_read_mac_addr(enetaddr, CONFIG_SYS_I2C_EEPROM_BUS);
	if (rv)
		get_efuse_mac_addr(enetaddr);

	if (!is_valid_ethaddr(enetaddr))
		return -1;

	return eth_env_set_enetaddr("ethaddr", enetaddr);
}

#define AR8051_PHY_DEBUG_ADDR_REG	0x1d
#define AR8051_PHY_DEBUG_DATA_REG	0x1e
#define AR8051_DEBUG_RGMII_CLK_DLY_REG	0x5
#define AR8051_RGMII_TX_CLK_DLY		0x100

int board_eth_init(bd_t *bis)
{
	int rv, n = 0;
	const char *devname;
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

	rv = handle_mac_address();
	if (rv)
		printf("No MAC address found!\n");

	writel(RGMII_MODE_ENABLE | RGMII_INT_DELAY, &cdev->miisel);

	board_phy_init();

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;

	/*
	 * CPSW RGMII Internal Delay Mode is not supported in all PVT
	 * operating points.  So we must set the TX clock delay feature
	 * in the AR8051 PHY.  Since we only support a single ethernet
	 * device, we only do this for the first instance.
	 */
	devname = miiphy_get_current_dev();

	miiphy_write(devname, 0x0, AR8051_PHY_DEBUG_ADDR_REG,
		     AR8051_DEBUG_RGMII_CLK_DLY_REG);
	miiphy_write(devname, 0x0, AR8051_PHY_DEBUG_DATA_REG,
		     AR8051_RGMII_TX_CLK_DLY);
	return n;
}
#endif /* CONFIG_DRIVER_TI_CPSW && !CONFIG_SPL_BUILD */
