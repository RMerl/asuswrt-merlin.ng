// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2011 Jana Rapava <fermata7@gmail.com>
 * Copyright (C) 2011 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Jana Rapava <fermata7@gmail.com>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 *
 * Based on:
 * linux/drivers/usb/otg/ulpi_viewport.c
 *
 * Original Copyright follow:
 * Copyright (C) 2011 Google, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <usb/ulpi.h>

/* ULPI viewport control bits */
#define ULPI_SS		(1 << 27)
#define ULPI_RWCTRL	(1 << 29)
#define ULPI_RWRUN	(1 << 30)
#define ULPI_WU		(1 << 31)

/*
 * Wait for the ULPI request to complete
 *
 * @ulpi_viewport	- the address of the viewport
 * @mask		- expected value to wait for
 *
 * returns 0 on mask match, ULPI_ERROR on time out.
 */
static int ulpi_wait(struct ulpi_viewport *ulpi_vp, u32 mask)
{
	int timeout = CONFIG_USB_ULPI_TIMEOUT;

	/* Wait for the bits in mask to become zero. */
	while (--timeout) {
		if ((readl(ulpi_vp->viewport_addr) & mask) == 0)
			return 0;

		udelay(1);
	}

	return ULPI_ERROR;
}

/*
 * Wake the ULPI PHY up for communication
 *
 * returns 0 on success.
 */
static int ulpi_wakeup(struct ulpi_viewport *ulpi_vp)
{
	int err;

	if (readl(ulpi_vp->viewport_addr) & ULPI_SS)
		return 0; /* already awake */

	writel(ULPI_WU, ulpi_vp->viewport_addr);

	err = ulpi_wait(ulpi_vp, ULPI_WU);
	if (err)
		printf("ULPI wakeup timed out\n");

	return err;
}

/*
 * Issue a ULPI read/write request
 *
 * @value - the ULPI request
 */
static int ulpi_request(struct ulpi_viewport *ulpi_vp, u32 value)
{
	int err;

	err = ulpi_wakeup(ulpi_vp);
	if (err)
		return err;

	writel(value, ulpi_vp->viewport_addr);

	err = ulpi_wait(ulpi_vp, ULPI_RWRUN);
	if (err)
		printf("ULPI request timed out\n");

	return err;
}

int ulpi_write(struct ulpi_viewport *ulpi_vp, u8 *reg, u32 value)
{
	u32 addr = (uintptr_t)reg & 0xFF;
	u32 val = ULPI_RWRUN | ULPI_RWCTRL | addr << 16 | (value & 0xff);

	val |= (ulpi_vp->port_num & 0x7) << 24;
	return ulpi_request(ulpi_vp, val);
}

u32 ulpi_read(struct ulpi_viewport *ulpi_vp, u8 *reg)
{
	int err;
	u32 val = ULPI_RWRUN | ((uintptr_t)reg & 0xFF) << 16;

	val |= (ulpi_vp->port_num & 0x7) << 24;
	err = ulpi_request(ulpi_vp, val);
	if (err)
		return err;

	return (readl(ulpi_vp->viewport_addr) >> 8) & 0xff;
}
