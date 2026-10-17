// SPDX-License-Identifier:	GPL-2.0+
/*
 * Copyright 2017-2018 NXP
 * Copyright (C) 2019 Oliver Graute <oliver.graute@kococonnector.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <env.h>
#include <errno.h>
#include <init.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <firmware/imx/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

static iomux_cfg_t uart0_pads[] = {
	SC_P_UART0_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_UART0_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart0_pads, ARRAY_SIZE(uart0_pads));
}

int board_early_init_f(void)
{
	sc_pm_clock_rate_t rate = SC_80MHZ;
	int ret;

	/* Set UART0 clock root to 80 MHz */
	ret = sc_pm_setup_uart(SC_R_UART_0, rate);
	if (ret)
		return ret;

	setup_iomux_uart();

	/* This is needed to because Kernel do not Power Up DC_0 */
	sc_pm_set_resource_power_mode(-1, SC_R_DC_0, SC_PM_PW_MODE_ON);
	sc_pm_set_resource_power_mode(-1, SC_R_GPIO_5, SC_PM_PW_MODE_ON);

	return 0;
}

#if IS_ENABLED(CONFIG_FEC_MXC)
#include <miiphy.h>

int board_phy_config(struct phy_device *phydev)
{
#ifdef CONFIG_FEC_ENABLE_MAX7322
	u8 value;

	/* This is needed to drive the pads to 1.8V instead of 1.5V */
	i2c_set_bus_num(CONFIG_MAX7322_I2C_BUS);

	if (!i2c_probe(CONFIG_MAX7322_I2C_ADDR)) {
		/* Write 0x1 to enable O0 output, this device has no addr */
		/* hence addr length is 0 */
		value = 0x1;
		if (dm_i2c_write(CONFIG_MAX7322_I2C_ADDR, 0, 0, &value, 1))
			printf("MAX7322 write failed\n");
	} else {
		printf("MAX7322 Not found\n");
	}
	mdelay(1);
#endif

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x00);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x82ee);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif

int checkboard(void)
{
	puts("Board: ROM-7720-A1 4GB\n");

	build_info();
	print_bootinfo();

	return 0;
}

int board_init(void)
{
	/* Power up base board */
	sc_pm_set_resource_power_mode(-1, SC_R_BOARD_R1, SC_PM_PW_MODE_ON);

	return 0;
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(void)
{
	/* TODO */
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "ROM-7720-A1");
	env_set("board_rev", "iMX8QM");
#endif

	env_set("sec_boot", "no");
#ifdef CONFIG_AHAB_BOOT
	env_set("sec_boot", "yes");
#endif

	return 0;
}
