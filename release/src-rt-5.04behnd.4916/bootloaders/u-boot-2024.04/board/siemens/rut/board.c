// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for TI AM335X based rut board
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * u-boot:/board/ti/am335x/board.c
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - https://www.ti.com/
 */

#include <cpsw.h>
#include <env.h>
#include <init.h>
#include <linux/delay.h>
#include <nand.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include "../common/board_am335x.h"
#include "../common/eeprom.h"
#include "../common/factoryset.h"

#ifdef CONFIG_SPL_BUILD
/*
 * Read header information from EEPROM into global structure.
 */
int draco_read_eeprom(void)
{
	return 0;
}

void draco_init_ddr(void)
{
struct emif_regs rut_ddr3_emif_reg_data = {
	.sdram_config = 0x61C04AB2,
	.sdram_tim1 = 0x0888A39B,
	.sdram_tim2 = 0x26337FDA,
	.sdram_tim3 = 0x501F830F,
	.emif_ddr_phy_ctlr_1 = 0x6,
	.zq_config = 0x50074BE4,
	.ref_ctrl = 0x93B,
};

struct ddr_data rut_ddr3_data = {
	.datardsratio0 = 0x3b,
	.datawdsratio0 = 0x85,
	.datafwsratio0 = 0x100,
	.datawrsratio0 = 0xc1,
};

struct cmd_control rut_ddr3_cmd_ctrl_data = {
	.cmd0csratio = 0x40,
	.cmd0iclkout = 1,
	.cmd1csratio = 0x40,
	.cmd1iclkout = 1,
	.cmd2csratio = 0x40,
	.cmd2iclkout = 1,
};

const struct ctrl_ioregs ioregs = {
	.cm0ioctl		= RUT_IOCTRL_VAL,
	.cm1ioctl		= RUT_IOCTRL_VAL,
	.cm2ioctl		= RUT_IOCTRL_VAL,
	.dt0ioctl		= RUT_IOCTRL_VAL,
	.dt1ioctl		= RUT_IOCTRL_VAL,
};

	config_ddr(DDR_PLL_FREQ, &ioregs, &rut_ddr3_data,
		   &rut_ddr3_cmd_ctrl_data, &rut_ddr3_emif_reg_data, 0);
}

static int request_and_pulse_reset(int gpio, const char *name)
{
	int ret;
	const int delay_us = 2000; /* 2ms */

	ret = gpio_request(gpio, name);
	if (ret < 0) {
		printf("%s: Unable to request %s\n", __func__, name);
		goto err;
	}

	ret = gpio_direction_output(gpio, 0);
	if (ret < 0) {
		printf("%s: Unable to set %s  as output\n", __func__, name);
		goto err_free_gpio;
	}

	udelay(delay_us);

	gpio_set_value(gpio, 1);

	return 0;

err_free_gpio:
	gpio_free(gpio);
err:
	return ret;
}

#define GPIO_TO_PIN(bank, gpio)		(32 * (bank) + (gpio))
#define ETH_PHY_RESET_GPIO		GPIO_TO_PIN(2, 18)
#define MAXTOUCH_RESET_GPIO		GPIO_TO_PIN(3, 18)
#define DISPLAY_RESET_GPIO		GPIO_TO_PIN(3, 19)

#define REQUEST_AND_PULSE_RESET(N) \
		request_and_pulse_reset(N, #N);

void spl_draco_board_init(void)
{
	REQUEST_AND_PULSE_RESET(ETH_PHY_RESET_GPIO);
	REQUEST_AND_PULSE_RESET(MAXTOUCH_RESET_GPIO);
	REQUEST_AND_PULSE_RESET(DISPLAY_RESET_GPIO);
}
#endif /* if def CONFIG_SPL_BUILD */

#if defined(CONFIG_DRIVER_TI_CPSW)
static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 1,
		.phy_if		= PHY_INTERFACE_MODE_RMII,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 0,
		.phy_if		= PHY_INTERFACE_MODE_RMII,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
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

#if defined(CONFIG_DRIVER_TI_CPSW) || \
	(defined(CONFIG_USB_ETHER) && defined(CONFIG_USB_MUSB_GADGET))
int board_eth_init(struct bd_info *bis)
{
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;
	int n = 0;
	int rv;

#ifndef CONFIG_SPL_BUILD
	factoryset_env_set();
#endif

	/* Set rgmii mode and enable rmii clock to be sourced from chip */
	writel((RMII_MODE_ENABLE | RMII_CHIPCKL_ENABLE), &cdev->miisel);

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;
	return n;
}
#endif /* #if defined(CONFIG_DRIVER_TI_CPSW) */
#endif /* #if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) */

#if defined(CONFIG_HW_WATCHDOG)
static bool hw_watchdog_init_done;
static int  hw_watchdog_trigger_level;

void hw_watchdog_reset(void)
{
	if (!hw_watchdog_init_done)
		return;

	hw_watchdog_trigger_level = hw_watchdog_trigger_level ? 0 : 1;
	gpio_set_value(WATCHDOG_TRIGGER_GPIO, hw_watchdog_trigger_level);
}

void hw_watchdog_init(void)
{
	gpio_request(WATCHDOG_TRIGGER_GPIO, "watchdog_trigger");
	gpio_direction_output(WATCHDOG_TRIGGER_GPIO, hw_watchdog_trigger_level);

	hw_watchdog_reset();

	hw_watchdog_init_done = 1;
}
#endif /* defined(CONFIG_HW_WATCHDOG) */

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	int ret;
	char tmp[2 * MAX_STRING_LENGTH + 2];

	omap_nand_switch_ecc(1, 8);

	if (factory_dat.asn[0] != 0)
		sprintf(tmp, "%s_%s", factory_dat.asn,
			factory_dat.comp_version);
	else
		strcpy(tmp, "QMX7.E38_4.0");

	ret = env_set("boardid", tmp);
	if (ret)
		printf("error setting board id\n");

	return 0;
}
#endif
