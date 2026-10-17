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
#include <linux/bitops.h>
#include <linux/compiler.h>
#include <serial.h>

#define SR_TX_FIFO_FULL		BIT(3) /* transmit FIFO full */
#define SR_TX_FIFO_EMPTY	BIT(2) /* transmit FIFO empty */
#define SR_RX_FIFO_VALID_DATA	BIT(0) /* data in receive FIFO */
#define SR_RX_FIFO_FULL		BIT(1) /* receive FIFO full */

#define ULITE_CONTROL_RST_TX	0x01
#define ULITE_CONTROL_RST_RX	0x02

static bool little_endian;

struct uartlite {
	unsigned int rx_fifo;
	unsigned int tx_fifo;
	unsigned int status;
	unsigned int control;
};

struct uartlite_plat {
	struct uartlite *regs;
};

static u32 uart_in32(void __iomem *addr)
{
	if (little_endian)
		return in_le32(addr);
	else
		return in_be32(addr);
}

static void uart_out32(void __iomem *addr, u32 val)
{
	if (little_endian)
		out_le32(addr, val);
	else
		out_be32(addr, val);
}

static int uartlite_serial_putc(struct udevice *dev, const char ch)
{
	struct uartlite_plat *plat = dev_get_plat(dev);
	struct uartlite *regs = plat->regs;

	if (uart_in32(&regs->status) & SR_TX_FIFO_FULL)
		return -EAGAIN;

	uart_out32(&regs->tx_fifo, ch & 0xff);

	return 0;
}

static int uartlite_serial_getc(struct udevice *dev)
{
	struct uartlite_plat *plat = dev_get_plat(dev);
	struct uartlite *regs = plat->regs;

	if (!(uart_in32(&regs->status) & SR_RX_FIFO_VALID_DATA))
		return -EAGAIN;

	return uart_in32(&regs->rx_fifo) & 0xff;
}

static int uartlite_serial_pending(struct udevice *dev, bool input)
{
	struct uartlite_plat *plat = dev_get_plat(dev);
	struct uartlite *regs = plat->regs;

	if (input)
		return uart_in32(&regs->status) & SR_RX_FIFO_VALID_DATA;

	return !(uart_in32(&regs->status) & SR_TX_FIFO_EMPTY);
}

static int uartlite_serial_probe(struct udevice *dev)
{
	struct uartlite_plat *plat = dev_get_plat(dev);
	struct uartlite *regs = plat->regs;
	int ret;

	uart_out32(&regs->control, 0);
	uart_out32(&regs->control, ULITE_CONTROL_RST_RX | ULITE_CONTROL_RST_TX);
	ret = uart_in32(&regs->status);
	/* Endianness detection */
	if ((ret & SR_TX_FIFO_EMPTY) != SR_TX_FIFO_EMPTY) {
		little_endian = true;
		uart_out32(&regs->control, ULITE_CONTROL_RST_RX |
			   ULITE_CONTROL_RST_TX);
	}

	return 0;
}

static int uartlite_serial_of_to_plat(struct udevice *dev)
{
	struct uartlite_plat *plat = dev_get_plat(dev);

	plat->regs = dev_read_addr_ptr(dev);

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
	.of_to_plat = uartlite_serial_of_to_plat,
	.plat_auto	= sizeof(struct uartlite_plat),
	.probe = uartlite_serial_probe,
	.ops	= &uartlite_serial_ops,
};

#ifdef CONFIG_DEBUG_UART_UARTLITE

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	struct uartlite *regs = (struct uartlite *)CONFIG_VAL(DEBUG_UART_BASE);
	int ret;

	uart_out32(&regs->control, 0);
	uart_out32(&regs->control, ULITE_CONTROL_RST_RX | ULITE_CONTROL_RST_TX);
	ret = uart_in32(&regs->status);
	/* Endianness detection */
	if ((ret & SR_TX_FIFO_EMPTY) != SR_TX_FIFO_EMPTY) {
		little_endian = true;
		uart_out32(&regs->control, ULITE_CONTROL_RST_RX |
			   ULITE_CONTROL_RST_TX);
	}
}

static inline void _debug_uart_putc(int ch)
{
	struct uartlite *regs = (struct uartlite *)CONFIG_VAL(DEBUG_UART_BASE);

	while (uart_in32(&regs->status) & SR_TX_FIFO_FULL)
		;

	uart_out32(&regs->tx_fifo, ch & 0xff);
}

DEBUG_UART_FUNCS
#endif
