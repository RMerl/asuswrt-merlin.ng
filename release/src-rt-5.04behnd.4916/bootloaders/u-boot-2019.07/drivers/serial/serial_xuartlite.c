// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 - 2015 Michal Simek <monstr@monstr.eu>
 * Clean driver and add xilinx constant from header file
 *
 * (C) Copyright 2004 Atmark Techno, Inc.
 * Yasushi SHOJI <yashi@atmark-techno.com>
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <serial.h>

#define SR_TX_FIFO_FULL		BIT(3) /* transmit FIFO full */
#define SR_TX_FIFO_EMPTY	BIT(2) /* transmit FIFO empty */
#define SR_RX_FIFO_VALID_DATA	BIT(0) /* data in receive FIFO */
#define SR_RX_FIFO_FULL		BIT(1) /* receive FIFO full */

#define ULITE_CONTROL_RST_TX	0x01
#define ULITE_CONTROL_RST_RX	0x02

struct uartlite {
	unsigned int rx_fifo;
	unsigned int tx_fifo;
	unsigned int status;
	unsigned int control;
};

struct uartlite_platdata {
	struct uartlite *regs;
};

static int uartlite_serial_putc(struct udevice *dev, const char ch)
{
	struct uartlite_platdata *plat = dev_get_platdata(dev);
	struct uartlite *regs = plat->regs;

	if (in_be32(&regs->status) & SR_TX_FIFO_FULL)
		return -EAGAIN;

	out_be32(&regs->tx_fifo, ch & 0xff);

	return 0;
}

static int uartlite_serial_getc(struct udevice *dev)
{
	struct uartlite_platdata *plat = dev_get_platdata(dev);
	struct uartlite *regs = plat->regs;

	if (!(in_be32(&regs->status) & SR_RX_FIFO_VALID_DATA))
		return -EAGAIN;

	return in_be32(&regs->rx_fifo) & 0xff;
}

static int uartlite_serial_pending(struct udevice *dev, bool input)
{
	struct uartlite_platdata *plat = dev_get_platdata(dev);
	struct uartlite *regs = plat->regs;

	if (input)
		return in_be32(&regs->status) & SR_RX_FIFO_VALID_DATA;

	return !(in_be32(&regs->status) & SR_TX_FIFO_EMPTY);
}

static int uartlite_serial_probe(struct udevice *dev)
{
	struct uartlite_platdata *plat = dev_get_platdata(dev);
	struct uartlite *regs = plat->regs;

	out_be32(&regs->control, 0);
	out_be32(&regs->control, ULITE_CONTROL_RST_RX | ULITE_CONTROL_RST_TX);
	in_be32(&regs->control);

	return 0;
}

static int uartlite_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct uartlite_platdata *plat = dev_get_platdata(dev);

	plat->regs = (struct uartlite *)devfdt_get_addr(dev);

	return 0;
}

static const struct dm_serial_ops uartlite_serial_ops = {
	.putc = uartlite_serial_putc,
	.pending = uartlite_serial_pending,
	.getc = uartlite_serial_getc,
};

static const struct udevice_id uartlite_serial_ids[] = {
	{ .compatible = "xlnx,opb-uartlite-1.00.b", },
	{ .compatible = "xlnx,xps-uartlite-1.00.a" },
	{ }
};

U_BOOT_DRIVER(serial_uartlite) = {
	.name	= "serial_uartlite",
	.id	= UCLASS_SERIAL,
	.of_match = uartlite_serial_ids,
	.ofdata_to_platdata = uartlite_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct uartlite_platdata),
	.probe = uartlite_serial_probe,
	.ops	= &uartlite_serial_ops,
};

#ifdef CONFIG_DEBUG_UART_UARTLITE

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	struct uartlite *regs = (struct uartlite *)CONFIG_DEBUG_UART_BASE;

	out_be32(&regs->control, 0);
	out_be32(&regs->control, ULITE_CONTROL_RST_RX | ULITE_CONTROL_RST_TX);
	in_be32(&regs->control);
}

static inline void _debug_uart_putc(int ch)
{
	struct uartlite *regs = (struct uartlite *)CONFIG_DEBUG_UART_BASE;

	while (in_be32(&regs->status) & SR_TX_FIFO_FULL)
		;

	out_be32(&regs->tx_fifo, ch & 0xff);
}

DEBUG_UART_FUNCS
#endif
