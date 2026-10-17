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
#include <linux/bitops.h>

/* data register */
#define ALTERA_JTAG_RVALID	BIT(15)	/* Read valid */

/* control register */
#define ALTERA_JTAG_AC		BIT(10)	/* activity indicator */
#define ALTERA_JTAG_RRDY	BIT(12)	/* read available */
#define ALTERA_JTAG_WSPACE(d)	((d) >> 16)	/* Write space avail */
/* Write fifo size. FIXME: this should be extracted with sopc2dts */
#define ALTERA_JTAG_WRITE_DEPTH	64

struct altera_jtaguart_regs {
	u32	data;			/* Data register */
	u32	control;		/* Control register */
};

struct altera_jtaguart_plat {
	struct altera_jtaguart_regs *regs;
};

static int altera_jtaguart_setbrg(struct udevice *dev, int baudrate)
{
	return 0;
}

static int altera_jtaguart_putc(struct udevice *dev, const char ch)
{
	struct altera_jtaguart_plat *plat = dev_get_plat(dev);
	struct altera_jtaguart_regs *const regs = plat->regs;
	u32 st = readl(&regs->control);

#ifdef CONFIG_ALTERA_JTAG_UART_BYPASS
	if (!(st & ALTERA_JTAG_AC)) /* no connection yet */
		return -ENETUNREACH;
#endif

	if (ALTERA_JTAG_WSPACE(st) == 0)
		return -EAGAIN;

	writel(ch, &regs->data);

	return 0;
}

static int altera_jtaguart_pending(struct udevice *dev, bool input)
{
	struct altera_jtaguart_plat *plat = dev_get_plat(dev);
	struct altera_jtaguart_regs *const regs = plat->regs;
	u32 st = readl(&regs->control);

	if (input)
		return st & ALTERA_JTAG_RRDY ? 1 : 0;
	else
		return !(ALTERA_JTAG_WSPACE(st) == ALTERA_JTAG_WRITE_DEPTH);
}

static int altera_jtaguart_getc(struct udevice *dev)
{
	struct altera_jtaguart_plat *plat = dev_get_plat(dev);
	struct altera_jtaguart_regs *const regs = plat->regs;
	u32 val;

	val = readl(&regs->data);

	if (!(val & ALTERA_JTAG_RVALID))
		return -EAGAIN;

	return val & 0xff;
}

static int altera_jtaguart_probe(struct udevice *dev)
{
#ifdef CONFIG_ALTERA_JTAG_UART_BYPASS
	struct altera_jtaguart_plat *plat = dev_get_plat(dev);
	struct altera_jtaguart_regs *const regs = plat->regs;

	writel(ALTERA_JTAG_AC, &regs->control); /* clear AC flag */
#endif
	return 0;
}

static int altera_jtaguart_of_to_plat(struct udevice *dev)
{
	struct altera_jtaguart_plat *plat = dev_get_plat(dev);

	plat->regs = map_physmem(dev_read_addr(dev),
				 sizeof(struct altera_jtaguart_regs),
				 MAP_NOCACHE);

	return 0;
}

static const struct dm_serial_ops altera_jtaguart_ops = {
	.putc = altera_jtaguart_putc,
	.pending = altera_jtaguart_pending,
	.getc = altera_jtaguart_getc,
	.setbrg = altera_jtaguart_setbrg,
};

static const struct udevice_id altera_jtaguart_ids[] = {
	{ .compatible = "altr,juart-1.0" },
	{}
};

U_BOOT_DRIVER(altera_jtaguart) = {
	.name	= "altera_jtaguart",
	.id	= UCLASS_SERIAL,
	.of_match = altera_jtaguart_ids,
	.of_to_plat = altera_jtaguart_of_to_plat,
	.plat_auto	= sizeof(struct altera_jtaguart_plat),
	.probe = altera_jtaguart_probe,
	.ops	= &altera_jtaguart_ops,
};

#ifdef CONFIG_DEBUG_UART_ALTERA_JTAGUART

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
}

static inline void _debug_uart_putc(int ch)
{
	struct altera_jtaguart_regs *regs = (void *)CONFIG_VAL(DEBUG_UART_BASE);

	while (1) {
		u32 st = readl(&regs->control);

		if (ALTERA_JTAG_WSPACE(st))
			break;
	}

	writel(ch, &regs->data);
}

DEBUG_UART_FUNCS

#endif
