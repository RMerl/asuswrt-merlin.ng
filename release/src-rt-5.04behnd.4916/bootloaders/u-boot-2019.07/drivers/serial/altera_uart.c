// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <serial.h>
#include <asm/io.h>

/* status register */
#define ALTERA_UART_TMT		BIT(5)	/* tx empty */
#define ALTERA_UART_TRDY	BIT(6)	/* tx ready */
#define ALTERA_UART_RRDY	BIT(7)	/* rx ready */

struct altera_uart_regs {
	u32	rxdata;		/* Rx data reg */
	u32	txdata;		/* Tx data reg */
	u32	status;		/* Status reg */
	u32	control;	/* Control reg */
	u32	divisor;	/* Baud rate divisor reg */
	u32	endofpacket;	/* End-of-packet reg */
};

struct altera_uart_platdata {
	struct altera_uart_regs *regs;
	unsigned int uartclk;
};

static int altera_uart_setbrg(struct udevice *dev, int baudrate)
{
	struct altera_uart_platdata *plat = dev->platdata;
	struct altera_uart_regs *const regs = plat->regs;
	u32 div;

	div = (plat->uartclk / baudrate) - 1;
	writel(div, &regs->divisor);

	return 0;
}

static int altera_uart_putc(struct udevice *dev, const char ch)
{
	struct altera_uart_platdata *plat = dev->platdata;
	struct altera_uart_regs *const regs = plat->regs;

	if (!(readl(&regs->status) & ALTERA_UART_TRDY))
		return -EAGAIN;

	writel(ch, &regs->txdata);

	return 0;
}

static int altera_uart_pending(struct udevice *dev, bool input)
{
	struct altera_uart_platdata *plat = dev->platdata;
	struct altera_uart_regs *const regs = plat->regs;
	u32 st = readl(&regs->status);

	if (input)
		return st & ALTERA_UART_RRDY ? 1 : 0;
	else
		return !(st & ALTERA_UART_TMT);
}

static int altera_uart_getc(struct udevice *dev)
{
	struct altera_uart_platdata *plat = dev->platdata;
	struct altera_uart_regs *const regs = plat->regs;

	if (!(readl(&regs->status) & ALTERA_UART_RRDY))
		return -EAGAIN;

	return readl(&regs->rxdata) & 0xff;
}

static int altera_uart_probe(struct udevice *dev)
{
	return 0;
}

static int altera_uart_ofdata_to_platdata(struct udevice *dev)
{
	struct altera_uart_platdata *plat = dev_get_platdata(dev);

	plat->regs = map_physmem(devfdt_get_addr(dev),
				 sizeof(struct altera_uart_regs),
				 MAP_NOCACHE);
	plat->uartclk = dev_read_u32_default(dev, "clock-frequency", 0);

	return 0;
}

static const struct dm_serial_ops altera_uart_ops = {
	.putc = altera_uart_putc,
	.pending = altera_uart_pending,
	.getc = altera_uart_getc,
	.setbrg = altera_uart_setbrg,
};

static const struct udevice_id altera_uart_ids[] = {
	{ .compatible = "altr,uart-1.0" },
	{}
};

U_BOOT_DRIVER(altera_uart) = {
	.name	= "altera_uart",
	.id	= UCLASS_SERIAL,
	.of_match = altera_uart_ids,
	.ofdata_to_platdata = altera_uart_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct altera_uart_platdata),
	.probe = altera_uart_probe,
	.ops	= &altera_uart_ops,
};

#ifdef CONFIG_DEBUG_UART_ALTERA_UART

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	struct altera_uart_regs *regs = (void *)CONFIG_DEBUG_UART_BASE;
	u32 div;

	div = (CONFIG_DEBUG_UART_CLOCK / CONFIG_BAUDRATE) - 1;
	writel(div, &regs->divisor);
}

static inline void _debug_uart_putc(int ch)
{
	struct altera_uart_regs *regs = (void *)CONFIG_DEBUG_UART_BASE;

	while (1) {
		u32 st = readl(&regs->status);

		if (st & ALTERA_UART_TRDY)
			break;
	}

	writel(ch, &regs->txdata);
}

DEBUG_UART_FUNCS

#endif
