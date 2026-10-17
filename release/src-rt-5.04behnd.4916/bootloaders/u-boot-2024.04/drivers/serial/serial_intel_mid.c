// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Intel Corporation
 */

#include <common.h>
#include <dm.h>
#include <ns16550.h>
#include <serial.h>

/*
 * The UART clock is calculated as
 *
 *	UART clock = XTAL * UART_MUL / UART_DIV
 *
 * The baudrate is calculated as
 *
 *	baud rate = UART clock / UART_PS / DLAB
 */
#define UART_PS		0x30
#define UART_MUL	0x34
#define UART_DIV	0x38

static void mid_writel(struct ns16550_plat *plat, int offset, int value)
{
	unsigned char *addr;

	offset *= 1 << plat->reg_shift;
	addr = (unsigned char *)plat->base + offset;

	writel(value, addr + plat->reg_offset);
}

static int mid_serial_probe(struct udevice *dev)
{
	struct ns16550_plat *plat = dev_get_plat(dev);

	/*
	 * Initialize fractional divider correctly for Intel Edison
	 * platform.
	 *
	 * For backward compatibility we have to set initial DLAB value
	 * to 16 and speed to 115200 baud, where initial frequency is
	 * 29491200Hz, and XTAL frequency is 38.4MHz.
	 */
	mid_writel(plat, UART_MUL, 96);
	mid_writel(plat, UART_DIV, 125);
	mid_writel(plat, UART_PS, 16);

	return ns16550_serial_probe(dev);
}

static const struct udevice_id mid_serial_ids[] = {
	{ .compatible = "intel,mid-uart" },
	{}
};

U_BOOT_DRIVER(serial_intel_mid) = {
	.name	= "serial_intel_mid",
	.id	= UCLASS_SERIAL,
	.of_match = mid_serial_ids,
	.of_to_plat = ns16550_serial_of_to_plat,
	.plat_auto	= sizeof(struct ns16550_plat),
	.priv_auto	= sizeof(struct ns16550),
	.probe	= mid_serial_probe,
	.ops	= &ns16550_serial_ops,
};
