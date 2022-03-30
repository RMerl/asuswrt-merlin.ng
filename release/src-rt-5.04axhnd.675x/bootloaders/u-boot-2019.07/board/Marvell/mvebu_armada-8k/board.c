// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Information specific to the DB-88F7040 eval board. We strive to use
 * DT for such platform specfic configurations. At some point, this
 * might be removed here and implemented via DT.
 */
/* IO expander I2C device */
#define I2C_IO_EXP_ADDR		0x21
#define I2C_IO_CFG_REG_0	0x6
#define I2C_IO_DATA_OUT_REG_0	0x2
/* VBus enable */
#define I2C_IO_REG_0_USB_H0_OFF	0
#define I2C_IO_REG_0_USB_H1_OFF	1
#define I2C_IO_REG_VBUS		((1 << I2C_IO_REG_0_USB_H0_OFF) | \
				 (1 << I2C_IO_REG_0_USB_H1_OFF))
/* Current limit */
#define I2C_IO_REG_0_USB_H0_CL	4
#define I2C_IO_REG_0_USB_H1_CL	5
#define I2C_IO_REG_CL		((1 << I2C_IO_REG_0_USB_H0_CL) | \
				 (1 << I2C_IO_REG_0_USB_H1_CL))

static int usb_enabled = 0;

/* Board specific xHCI dis-/enable code */

/*
 * Set USB VBUS signals (via I2C IO expander/GPIO) as output and set
 * output value as disabled
 *
 * Set USB Current Limit signals (via I2C IO expander/GPIO) as output
 * and set output value as enabled
 */
int board_xhci_config(void)
{
	struct udevice *dev;
	int ret;
	u8 buf[8];

	if (of_machine_is_compatible("marvell,armada7040-db")) {
		/* Configure IO exander PCA9555: 7bit address 0x21 */
		ret = i2c_get_chip_for_busnum(0, I2C_IO_EXP_ADDR, 1, &dev);
		if (ret) {
			printf("Cannot find PCA9555: %d\n", ret);
			return 0;
		}

		/*
		 * Read configuration (direction) and set VBUS pin as output
		 * (reset pin = output)
		 */
		ret = dm_i2c_read(dev, I2C_IO_CFG_REG_0, buf, 1);
		if (ret) {
			printf("Failed to read IO expander value via I2C\n");
			return -EIO;
		}
		buf[0] &= ~I2C_IO_REG_VBUS;
		buf[0] &= ~I2C_IO_REG_CL;
		ret = dm_i2c_write(dev, I2C_IO_CFG_REG_0, buf, 1);
		if (ret) {
			printf("Failed to set IO expander via I2C\n");
			return -EIO;
		}

		/* Read output value and configure it */
		ret = dm_i2c_read(dev, I2C_IO_DATA_OUT_REG_0, buf, 1);
		if (ret) {
			printf("Failed to read IO expander value via I2C\n");
			return -EIO;
		}
		buf[0] &= ~I2C_IO_REG_VBUS;
		buf[0] |= I2C_IO_REG_CL;
		ret = dm_i2c_write(dev, I2C_IO_DATA_OUT_REG_0, buf, 1);
		if (ret) {
			printf("Failed to set IO expander via I2C\n");
			return -EIO;
		}

		mdelay(500); /* required delay to let output value settle */
	}

	return 0;
}

int board_xhci_enable(fdt_addr_t base)
{
	struct udevice *dev;
	int ret;
	u8 buf[8];

	if (of_machine_is_compatible("marvell,armada7040-db")) {
		/*
		 * This function enables all USB ports simultaniously,
		 * it only needs to get called once
		 */
		if (usb_enabled)
			return 0;

		/* Configure IO exander PCA9555: 7bit address 0x21 */
		ret = i2c_get_chip_for_busnum(0, I2C_IO_EXP_ADDR, 1, &dev);
		if (ret) {
			printf("Cannot find PCA9555: %d\n", ret);
			return 0;
		}

		/* Read VBUS output value */
		ret = dm_i2c_read(dev, I2C_IO_DATA_OUT_REG_0, buf, 1);
		if (ret) {
			printf("Failed to read IO expander value via I2C\n");
			return -EIO;
		}

		/* Enable VBUS power: Set output value of VBUS pin as enabled */
		buf[0] |= I2C_IO_REG_VBUS;
		ret = dm_i2c_write(dev, I2C_IO_DATA_OUT_REG_0, buf, 1);
		if (ret) {
			printf("Failed to set IO expander via I2C\n");
			return -EIO;
		}

		mdelay(500); /* required delay to let output value settle */
		usb_enabled = 1;
	}

	return 0;
}

int board_early_init_f(void)
{
	/* Nothing to do (yet), perhaps later some pin-muxing etc */

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int board_late_init(void)
{
	/* Pre-configure the USB ports (overcurrent, VBus) */
	board_xhci_config();

	return 0;
}
