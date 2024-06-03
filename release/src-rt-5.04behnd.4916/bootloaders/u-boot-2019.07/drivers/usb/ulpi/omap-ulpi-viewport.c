// SPDX-License-Identifier: GPL-2.0
/*
 * OMAP ulpi viewport support
 * Based on drivers/usb/ulpi/ulpi-viewport.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com
 * Author: Govindraj R <govindraj.raja@ti.com>
 */

#include <common.h>
#include <asm/io.h>
#include <usb/ulpi.h>

#define OMAP_ULPI_WR_OPSEL	(2 << 22)
#define OMAP_ULPI_RD_OPSEL	(3 << 22)
#define OMAP_ULPI_START		(1 << 31)

/*
 * Wait for having ulpi in done state
 */
static int ulpi_wait(struct ulpi_viewport *ulpi_vp, u32 mask)
{
	int timeout = CONFIG_USB_ULPI_TIMEOUT;

	while (--timeout) {
		if (!(readl(ulpi_vp->viewport_addr) & mask))
			return 0;

		udelay(1);
	}

	return ULPI_ERROR;
}

/*
 * Issue a ULPI read/write request
 */
static int ulpi_request(struct ulpi_viewport *ulpi_vp, u32 value)
{
	int err;

	writel(value, ulpi_vp->viewport_addr);

	err = ulpi_wait(ulpi_vp, OMAP_ULPI_START);
	if (err)
		debug("ULPI request timed out\n");

	return err;
}

int ulpi_write(struct ulpi_viewport *ulpi_vp, u8 *reg, u32 value)
{
	u32 val = OMAP_ULPI_START | (((ulpi_vp->port_num + 1) & 0xf) << 24) |
			OMAP_ULPI_WR_OPSEL | ((u32)reg << 16) | (value & 0xff);

	return ulpi_request(ulpi_vp, val);
}

u32 ulpi_read(struct ulpi_viewport *ulpi_vp, u8 *reg)
{
	int err;
	u32 val = OMAP_ULPI_START | (((ulpi_vp->port_num + 1) & 0xf) << 24) |
			 OMAP_ULPI_RD_OPSEL | ((u32)reg << 16);

	err = ulpi_request(ulpi_vp, val);
	if (err)
		return err;

	return readl(ulpi_vp->viewport_addr) & 0xff;
}
