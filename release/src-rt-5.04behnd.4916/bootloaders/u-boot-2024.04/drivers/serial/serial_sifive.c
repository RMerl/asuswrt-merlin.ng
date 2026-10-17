// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Anup Patel <anup@brainfault.org>
 */

#include <common.h>
#include <clk.h>
#include <debug_uart.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <watchdog.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <serial.h>
#include <linux/err.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_TXFIFO_FULL	0x80000000
#define UART_RXFIFO_EMPTY	0x80000000
#define UART_RXFIFO_DATA	0x000000ff
#define UART_TXCTRL_TXEN	0x1
#define UART_RXCTRL_RXEN	0x1

/* IP register */
#define UART_IP_RXWM            0x2

struct uart_sifive {
	u32 txfifo;
	u32 rxfifo;
	u32 txctrl;
	u32 rxctrl;
	u32 ie;
	u32 ip;
	u32 div;
};

struct sifive_uart_plat {
	unsigned long clock;
	struct uart_sifive *regs;
};

/**
 * Find minimum divisor divides in_freq to max_target_hz;
 * Based on uart driver n SiFive FSBL.
 *
 * f_baud = f_in / (div + 1) => div = (f_in / f_baud) - 1
 * The nearest integer solution requires rounding up as to not exceed
 * max_target_hz.
 * div  = ceil(f_in / f_baud) - 1
 *	= floor((f_in - 1 + f_baud) / f_baud) - 1
 * This should not overflow as long as (f_in - 1 + f_baud) does not exceed
 * 2^32 - 1, which is unlikely since we represent frequencies in kHz.
 */
static inline unsigned int uart_min_clk_divisor(unsigned long in_freq,
						unsigned long max_target_hz)
{
	unsigned long quotient =
			(in_freq + max_target_hz - 1) / (max_target_hz);
	/* Avoid underflow */
	if (quotient == 0)
		return 0;
	else
		return quotient - 1;
}

/* Set up the baud rate in gd struct */
static void _sifive_serial_setbrg(struct uart_sifive *regs,
				  unsigned long clock, unsigned long baud)
{
	writel((uart_min_clk_divisor(clock, baud)), &regs->div);
}

static void _sifive_serial_init(struct uart_sifive *regs)
{
	writel(UART_TXCTRL_TXEN, &regs->txctrl);
	writel(UART_RXCTRL_RXEN, &regs->rxctrl);
	writel(0, &regs->ie);
}

static int _sifive_serial_putc(struct uart_sifive *regs, const char c)
{
	if (readl(&regs->txfifo) & UART_TXFIFO_FULL)
		return -EAGAIN;

	writel(c, &regs->txfifo);

	return 0;
}

static int _sifive_serial_getc(struct uart_sifive *regs)
{
	int ch = readl(&regs->rxfifo);

	if (ch & UART_RXFIFO_EMPTY)
		return -EAGAIN;
	ch &= UART_RXFIFO_DATA;

	return ch;
}

static int sifive_serial_setbrg(struct udevice *dev, int baudrate)
{
	int ret;
	struct clk clk;
	struct sifive_uart_plat *plat = dev_get_plat(dev);
	u32 clock = 0;

	ret = clk_get_by_index(dev, 0, &clk);
	if (IS_ERR_VALUE(ret)) {
		debug("SiFive UART failed to get clock\n");
		ret = dev_read_u32(dev, "clock-frequency", &clock);
		if (IS_ERR_VALUE(ret)) {
			debug("SiFive UART clock not defined\n");
			return 0;
		}
	} else {
		clock = clk_get_rate(&clk);
		if (IS_ERR_VALUE(clock)) {
			debug("SiFive UART clock get rate failed\n");
			return 0;
		}
	}
	plat->clock = clock;
	_sifive_serial_setbrg(plat->regs, plat->clock, baudrate);

	return 0;
}

static int sifive_serial_probe(struct udevice *dev)
{
	struct sifive_uart_plat *plat = dev_get_plat(dev);

	/* No need to reinitialize the UART after relocation */
	if (gd->flags & GD_FLG_RELOC)
		return 0;

	_sifive_serial_init(plat->regs);

	return 0;
}

static int sifive_serial_getc(struct udevice *dev)
{
	int c;
	struct sifive_uart_plat *plat = dev_get_plat(dev);
	struct uart_sifive *regs = plat->regs;

	while ((c = _sifive_serial_getc(regs)) == -EAGAIN) ;

	return c;
}

static int sifive_serial_putc(struct udevice *dev, const char ch)
{
	int rc;
	struct sifive_uart_plat *plat = dev_get_plat(dev);

	while ((rc = _sifive_serial_putc(plat->regs, ch)) == -EAGAIN) ;

	return rc;
}

static int sifive_serial_pending(struct udevice *dev, bool input)
{
	struct sifive_uart_plat *plat = dev_get_plat(dev);
	struct uart_sifive *regs = plat->regs;

	if (input)
		return (readl(&regs->ip) & UART_IP_RXWM);
	else
		return !!(readl(&regs->txfifo) & UART_TXFIFO_FULL);
}

static int sifive_serial_of_to_plat(struct udevice *dev)
{
	struct sifive_uart_plat *plat = dev_get_plat(dev);

	plat->regs = (struct uart_sifive *)(uintptr_t)dev_read_addr(dev);
	if (IS_ERR(plat->regs))
		return PTR_ERR(plat->regs);

	return 0;
}

static const struct dm_serial_ops sifive_serial_ops = {
	.putc = sifive_serial_putc,
	.getc = sifive_serial_getc,
	.pending = sifive_serial_pending,
	.setbrg = sifive_serial_setbrg,
};

static const struct udevice_id sifive_serial_ids[] = {
	{ .compatible = "sifive,uart0" },
	{ }
};

U_BOOT_DRIVER(serial_sifive) = {
	.name	= "serial_sifive",
	.id	= UCLASS_SERIAL,
	.of_match = sifive_serial_ids,
	.of_to_plat = sifive_serial_of_to_plat,
	.plat_auto	= sizeof(struct sifive_uart_plat),
	.probe = sifive_serial_probe,
	.ops	= &sifive_serial_ops,
};

#ifdef CONFIG_DEBUG_UART_SIFIVE
static inline void _debug_uart_init(void)
{
	struct uart_sifive *regs =
			(struct uart_sifive *)CONFIG_VAL(DEBUG_UART_BASE);

	_sifive_serial_setbrg(regs, CONFIG_DEBUG_UART_CLOCK,
			      CONFIG_BAUDRATE);
	_sifive_serial_init(regs);
}

static inline void _debug_uart_putc(int ch)
{
	struct uart_sifive *regs =
			(struct uart_sifive *)CONFIG_VAL(DEBUG_UART_BASE);

	while (_sifive_serial_putc(regs, ch) == -EAGAIN)
		schedule();
}

DEBUG_UART_FUNCS

#endif
