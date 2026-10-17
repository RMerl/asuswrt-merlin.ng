// SPDX-License-Identifier: GPL-2.0+
/*
 * am35x.c - TI's AM35x platform specific usb wrapper functions.
 *
 * Author: Ajay Kumar Gupta <ajay.gupta@ti.com>
 *
 * Based on drivers/usb/musb/da8xx.c
 *
 * Copyright (c) 2010 Texas Instruments Incorporated
 */

#include <common.h>
#include <linux/delay.h>

#include "am35x.h"

/* MUSB platform configuration */
struct musb_config musb_cfg = {
	.regs		= (struct musb_regs *)AM35X_USB_OTG_CORE_BASE,
	.timeout	= AM35X_USB_OTG_TIMEOUT,
	.musb_speed	= 0,
};

/*
 * Enable the USB phy
 */
static u8 phy_on(void)
{
	u32 devconf2;
	u32 timeout;

	devconf2 = readl(&am35x_scm_general_regs->devconf2);

	devconf2 &= ~(DEVCONF2_RESET | DEVCONF2_PHYPWRDN | DEVCONF2_OTGPWRDN |
		      DEVCONF2_OTGMODE | DEVCONF2_REFFREQ |
		      DEVCONF2_PHY_GPIOMODE);
	devconf2 |= DEVCONF2_SESENDEN | DEVCONF2_VBDTCTEN | DEVCONF2_PHY_PLLON |
		    DEVCONF2_REFFREQ_13MHZ | DEVCONF2_DATPOL;

	writel(devconf2, &am35x_scm_general_regs->devconf2);

	/* wait until the USB phy is turned on */
	timeout = musb_cfg.timeout;
	while (timeout--)
		if (readl(&am35x_scm_general_regs->devconf2) & DEVCONF2_PHYCKGD)
			return 1;

	/* USB phy was not turned on */
	return 0;
}

/*
 * Disable the USB phy
 */
static void phy_off(void)
{
	u32 devconf2;

	/*
	 * Power down the on-chip PHY.
	 */
	devconf2 = readl(&am35x_scm_general_regs->devconf2);

	devconf2 &= ~DEVCONF2_PHY_PLLON;
	devconf2 |= DEVCONF2_PHYPWRDN | DEVCONF2_OTGPWRDN;
	writel(devconf2, &am35x_scm_general_regs->devconf2);
}

/*
 * This function performs platform specific initialization for usb0.
 */
int musb_platform_init(void)
{
	u32 revision;
	u32 sw_reset;

	/* global usb reset */
	sw_reset = readl(&am35x_scm_general_regs->ip_sw_reset);
	sw_reset |= (1 << 0);
	writel(sw_reset, &am35x_scm_general_regs->ip_sw_reset);
	sw_reset &= ~(1 << 0);
	writel(sw_reset, &am35x_scm_general_regs->ip_sw_reset);

	/* reset the controller */
	writel(0x1, &am35x_usb_regs->control);
	udelay(5000);

	/* start the on-chip usb phy and its pll */
	if (phy_on() == 0)
		return -1;

	/* Returns zero if e.g. not clocked */
	revision = readl(&am35x_usb_regs->revision);
	if (revision == 0)
		return -1;

	return 0;
}

/*
 * This function performs platform specific deinitialization for usb0.
 */
void musb_platform_deinit(void)
{
	/* Turn off the phy */
	phy_off();
}

/*
 * This function reads data from endpoint fifo for AM35x
 * which supports only 32bit read operation.
 *
 * ep           - endpoint number
 * length       - number of bytes to read from FIFO
 * fifo_data    - pointer to data buffer into which data is read
 */
__attribute__((weak))
void read_fifo(u8 ep, u32 length, void *fifo_data)
{
	u8  *data = (u8 *)fifo_data;
	u32 val;
	int i;

	/* select the endpoint index */
	writeb(ep, &musbr->index);

	if (length > 4) {
		for (i = 0; i < (length >> 2); i++) {
			val = readl(&musbr->fifox[ep]);
			memcpy(data, &val, 4);
			data += 4;
		}
		length %= 4;
	}
	if (length > 0) {
		val = readl(&musbr->fifox[ep]);
		memcpy(data, &val, length);
	}
}
