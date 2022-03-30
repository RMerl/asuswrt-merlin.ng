// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <i2c.h>
#include <winbond_w83627.h>
#include <asm/gpio.h>
#include <asm/ibmpc.h>
#include <asm/pnp_def.h>

int board_early_init_f(void)
{
#ifndef CONFIG_INTERNAL_UART
	/*
	 * The FSP enables the BayTrail internal legacy UART (again).
	 * Disable it again, so that the Winbond one can be used.
	 */
	setup_internal_uart(0);

	/* Enable the legacy UART in the Winbond W83627 Super IO chip */
	winbond_enable_serial(PNP_DEV(WINBOND_IO_PORT, W83627DHG_SP1),
			      UART0_BASE, UART0_IRQ);
#endif

	return 0;
}

int board_late_init(void)
{
	struct udevice *dev;
	u8 buf[8];
	int ret;

	/* Configure SMSC USB2513 USB Hub: 7bit address 0x2c */
	ret = i2c_get_chip_for_busnum(0, 0x2c, 1, &dev);
	if (ret) {
		printf("Cannot find USB2513: %d\n", ret);
		return 0;
	}

	/*
	 * The first access to the USB Hub fails sometimes, so lets read
	 * a dummy byte to be sure here
	 */
	dm_i2c_read(dev, 0x00, buf, 1);

	/*
	 * The SMSC hub is not visible on the I2C bus after the first
	 * configuration at power-up. The following code deliberately
	 * does not report upon failure of these I2C write calls.
	 */
	buf[0] = 0x93;
	dm_i2c_write(dev, 0x06, buf, 1);

	buf[0] = 0xaa;
	dm_i2c_write(dev, 0xf8, buf, 1);

	buf[0] = 0x0f;
	dm_i2c_write(dev, 0xfa, buf, 1);

	buf[0] = 0x01;
	dm_i2c_write(dev, 0xff, buf, 1);

	return 0;
}
