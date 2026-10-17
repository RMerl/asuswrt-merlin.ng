// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * This is file is based on
 * repository git.gitorious.org/u-boot-omap3/mainline.git,
 * branch omap3-dev-usb, file drivers/usb/gadget/twl4030_usb.c
 *
 * This is the unique part of its copyright :
 *
 * ------------------------------------------------------------------------
 *
 *  * (C) Copyright 2009 Atin Malaviya (atin.malaviya@gmail.com)
 *
 * Based on: twl4030_usb.c in linux 2.6 (drivers/i2c/chips/twl4030_usb.c)
 * Copyright (C) 2004-2007 Texas Instruments
 * Copyright (C) 2008 Nokia Corporation
 * Contact: Felipe Balbi <felipe.balbi@nokia.com>
 *
 * Author: Atin Malaviya (atin.malaviya@gmail.com)
 *
 * ------------------------------------------------------------------------
 */

#include <twl4030.h>
#include <linux/delay.h>

/* Defines for bits in registers */
#define OPMODE_MASK		(3 << 3)
#define XCVRSELECT_MASK		(3 << 0)
#define CARKITMODE		(1 << 2)
#define OTG_ENAB		(1 << 5)
#define PHYPWD			(1 << 0)
#define CLOCKGATING_EN		(1 << 2)
#define CLK32K_EN		(1 << 1)
#define REQ_PHY_DPLL_CLK	(1 << 0)
#define PHY_DPLL_CLK		(1 << 0)

static int twl4030_usb_write(u8 address, u8 data)
{
	int ret;

	ret = twl4030_i2c_write_u8(TWL4030_CHIP_USB, address, data);
	if (ret != 0)
		printf("TWL4030:USB:Write[0x%x] Error %d\n", address, ret);

	return ret;
}

static int twl4030_usb_read(u8 address)
{
	u8 data;
	int ret;

	ret = twl4030_i2c_read_u8(TWL4030_CHIP_USB, address, &data);
	if (ret == 0)
		ret = data;
	else
		printf("TWL4030:USB:Read[0x%x] Error %d\n", address, ret);

	return ret;
}

static void twl4030_usb_ldo_init(void)
{
	/* Enable writing to power configuration registers */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
			     TWL4030_PM_MASTER_PROTECT_KEY, 0xC0);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
			     TWL4030_PM_MASTER_PROTECT_KEY, 0x0C);

	/* put VUSB3V1 LDO in active state */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER,
			     TWL4030_PM_RECEIVER_VUSB_DEDICATED2, 0x00);

	/* input to VUSB3V1 LDO is from VBAT, not VBUS */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER,
			     TWL4030_PM_RECEIVER_VUSB_DEDICATED1, 0x14);

	/* turn on 3.1V regulator */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER,
			     TWL4030_PM_RECEIVER_VUSB3V1_DEV_GRP, 0x20);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER,
			     TWL4030_PM_RECEIVER_VUSB3V1_TYPE, 0x00);

	/* turn on 1.5V regulator */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER,
			     TWL4030_PM_RECEIVER_VUSB1V5_DEV_GRP, 0x20);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER,
			     TWL4030_PM_RECEIVER_VUSB1V5_TYPE, 0x00);

	/* turn on 1.8V regulator */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER,
			     TWL4030_PM_RECEIVER_VUSB1V8_DEV_GRP, 0x20);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER,
			     TWL4030_PM_RECEIVER_VUSB1V8_TYPE, 0x00);

	/* disable access to power configuration registers */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
			     TWL4030_PM_MASTER_PROTECT_KEY, 0x00);
}

static void twl4030_phy_power(void)
{
	u8 pwr, clk;

	/* Power the PHY */
	pwr = twl4030_usb_read(TWL4030_USB_PHY_PWR_CTRL);
	pwr &= ~PHYPWD;
	twl4030_usb_write(TWL4030_USB_PHY_PWR_CTRL, pwr);
	/* Enable clocks */
	clk = twl4030_usb_read(TWL4030_USB_PHY_CLK_CTRL);
	clk |= CLOCKGATING_EN | CLK32K_EN;
	twl4030_usb_write(TWL4030_USB_PHY_CLK_CTRL, clk);
}

/*
 * Initiaze the ULPI interface
 * ULPI : Universal Transceiver Macrocell Low Pin Interface
 * An interface between the USB link controller like musb and the
 * the PHY or transceiver that drives the actual bus.
 */
int twl4030_usb_ulpi_init(void)
{
	long timeout = 1000 * 1000; /* 1 sec */;
	u8 clk, sts, pwr;

	/* twl4030 ldo init */
	twl4030_usb_ldo_init();

	/* Enable the twl4030 phy */
	twl4030_phy_power();

	/* Enable DPLL to access PHY registers over I2C */
	clk = twl4030_usb_read(TWL4030_USB_PHY_CLK_CTRL);
	clk |= REQ_PHY_DPLL_CLK;
	twl4030_usb_write(TWL4030_USB_PHY_CLK_CTRL, clk);

	/* Check if the PHY DPLL is locked */
	sts = twl4030_usb_read(TWL4030_USB_PHY_CLK_CTRL_STS);
	while (!(sts & PHY_DPLL_CLK) && 0 < timeout) {
		udelay(10);
		sts = twl4030_usb_read(TWL4030_USB_PHY_CLK_CTRL_STS);
		timeout -= 10;
	}

	/* Final check */
	sts = twl4030_usb_read(TWL4030_USB_PHY_CLK_CTRL_STS);
	if (!(sts & PHY_DPLL_CLK)) {
		printf("Error:TWL4030:USB Timeout setting PHY DPLL clock\n");
		return -1;
	}

	/*
	 * There are two circuit blocks attached to the PHY,
	 * Carkit and USB OTG.  Disable Carkit and enable USB OTG
	 */
	twl4030_usb_write(TWL4030_USB_IFC_CTRL_CLR, CARKITMODE);
	pwr = twl4030_usb_read(TWL4030_USB_POWER_CTRL);
	pwr |= OTG_ENAB;
	twl4030_usb_write(TWL4030_USB_POWER_CTRL_SET, pwr);

	/* Clear the opmode bits to ensure normal encode */
	twl4030_usb_write(TWL4030_USB_FUNC_CTRL_CLR, OPMODE_MASK);

	/* Clear the xcvrselect bits to enable the high speed transeiver */
	twl4030_usb_write(TWL4030_USB_FUNC_CTRL_CLR, XCVRSELECT_MASK);

	/* Let ULPI control the DPLL clock */
	clk = twl4030_usb_read(TWL4030_USB_PHY_CLK_CTRL);
	clk &= ~REQ_PHY_DPLL_CLK;
	twl4030_usb_write(TWL4030_USB_PHY_CLK_CTRL, clk);

	return 0;
}
