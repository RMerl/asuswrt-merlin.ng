// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 */

/* Simple U-Boot driver for the PrimeCell PL010/PL011 UARTs */

#include <common.h>
#include <asm/global_data.h>
/* For get_bus_freq() */
#include <clock_legacy.h>
#include <dm.h>
#include <clk.h>
#include <errno.h>
#include <watchdog.h>
#include <asm/io.h>
#include <serial.h>
#include <dm/device_compat.h>
#include <dm/platform_data/serial_pl01x.h>
#include <linux/compiler.h>
#include "serial_pl01x_internal.h"

DECLARE_GLOBAL_DATA_PTR;

#if !CONFIG_IS_ENABLED(DM_SERIAL)
static volatile unsigned char *const port[] = CFG_PL01x_PORTS;
static enum pl01x_type pl01x_type __section(".data");
static struct pl01x_regs *base_regs __section(".data");
#define NUM_PORTS (sizeof(port)/sizeof(port[0]))

#endif

static int pl01x_putc(struct pl01x_regs *regs, char c)
{
	/* Wait until there is space in the FIFO */
	if (readl(&regs->fr) & UART_PL01x_FR_TXFF)
		return -EAGAIN;

	/* Send the character */
	writel(c, &regs->dr);

	return 0;
}

static int pl01x_getc(struct pl01x_regs *regs)
{
	unsigned int data;

	/* Wait until there is data in the FIFO */
	if (readl(&regs->fr) & UART_PL01x_FR_RXFE)
		return -EAGAIN;

	data = readl(&regs->dr);

	/* Check for an error flag */
	if (data & 0xFFFFFF00) {
		/* Clear the error */
		writel(0xFFFFFFFF, &regs->ecr);
		return -1;
	}

	return (int) data;
}

static int pl01x_tstc(struct pl01x_regs *regs)
{
	schedule();
	return !(readl(&regs->fr) & UART_PL01x_FR_RXFE);
}

static int pl01x_generic_serial_init(struct pl01x_regs *regs,
				     enum pl01x_type type)
{
	switch (type) {
	case TYPE_PL010:
		/* disable everything */
		writel(0, &regs->pl010_cr);
		break;
	case TYPE_PL011:
		/* disable everything */
		writel(0, &regs->pl011_cr);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int pl011_set_line_control(struct pl01x_regs *regs)
{
	unsigned int lcr;
	/*
	 * Internal update of baud rate register require line
	 * control register write
	 */
	lcr = UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_FEN;
	writel(lcr, &regs->pl011_lcrh);
	return 0;
}

static int pl01x_generic_setbrg(struct pl01x_regs *regs, enum pl01x_type type,
				int clock, int baudrate)
{
	switch (type) {
	case TYPE_PL010: {
		unsigned int divisor;

		/* disable everything */
		writel(0, &regs->pl010_cr);

		switch (baudrate) {
		case 9600:
			divisor = UART_PL010_BAUD_9600;
			break;
		case 19200:
			divisor = UART_PL010_BAUD_19200;
			break;
		case 38400:
			divisor = UART_PL010_BAUD_38400;
			break;
		case 57600:
			divisor = UART_PL010_BAUD_57600;
			break;
		case 115200:
			divisor = UART_PL010_BAUD_115200;
			break;
		default:
			divisor = UART_PL010_BAUD_38400;
		}

		writel((divisor & 0xf00) >> 8, &regs->pl010_lcrm);
		writel(divisor & 0xff, &regs->pl010_lcrl);

		/*
		 * Set line control for the PL010 to be 8 bits, 1 stop bit,
		 * no parity, fifo enabled
		 */
		writel(UART_PL010_LCRH_WLEN_8 | UART_PL010_LCRH_FEN,
		       &regs->pl010_lcrh);
		/* Finally, enable the UART */
		writel(UART_PL010_CR_UARTEN, &regs->pl010_cr);
		break;
	}
	case TYPE_PL011: {
		unsigned int temp;
		unsigned int divider;
		unsigned int remainder;
		unsigned int fraction;

		/* Without a valid clock rate we cannot set up the baudrate. */
		if (clock) {
			/*
			 * Set baud rate
			 *
			 * IBRD = UART_CLK / (16 * BAUD_RATE)
			 * FBRD = RND((64 * MOD(UART_CLK,(16 * BAUD_RATE)))
			 *		/ (16 * BAUD_RATE))
			 */
			temp = 16 * baudrate;
			divider = clock / temp;
			remainder = clock % temp;
			temp = (8 * remainder) / baudrate;
			fraction = (temp >> 1) + (temp & 1);

			writel(divider, &regs->pl011_ibrd);
			writel(fraction, &regs->pl011_fbrd);
		}

		pl011_set_line_control(regs);
		/* Finally, enable the UART */
		writel(UART_PL011_CR_UARTEN | UART_PL011_CR_TXE |
		       UART_PL011_CR_RXE | UART_PL011_CR_RTS, &regs->pl011_cr);
		break;
	}
	default:
		return -EINVAL;
	}

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_SERIAL)
static void pl01x_serial_init_baud(int baudrate)
{
	int clock = 0;

#if defined(CONFIG_PL011_SERIAL)
	pl01x_type = TYPE_PL011;
	clock = CFG_PL011_CLOCK;
#endif
	base_regs = (struct pl01x_regs *)port[CONFIG_CONS_INDEX];

	pl01x_generic_serial_init(base_regs, pl01x_type);
	pl01x_generic_setbrg(base_regs, pl01x_type, clock, baudrate);
}

/*
 * Integrator AP has two UARTs, we use the first one, at 38400-8-N-1
 * Integrator CP has two UARTs, use the first one, at 38400-8-N-1
 * Versatile PB has four UARTs.
 */
int pl01x_serial_init(void)
{
	pl01x_serial_init_baud(CONFIG_BAUDRATE);

	return 0;
}

static void pl01x_serial_putc(const char c)
{
	if (c == '\n')
		while (pl01x_putc(base_regs, '\r') == -EAGAIN);

	while (pl01x_putc(base_regs, c) == -EAGAIN);
}

static int pl01x_serial_getc(void)
{
	while (1) {
		int ch = pl01x_getc(base_regs);

		if (ch == -EAGAIN) {
			schedule();
			continue;
		}

		return ch;
	}
}

static int pl01x_serial_tstc(void)
{
	return pl01x_tstc(base_regs);
}

static void pl01x_serial_setbrg(void)
{
	/*
	 * Flush FIFO and wait for non-busy before changing baudrate to avoid
	 * crap in console
	 */
	while (!(readl(&base_regs->fr) & UART_PL01x_FR_TXFE))
		schedule();
	while (readl(&base_regs->fr) & UART_PL01x_FR_BUSY)
		schedule();
	pl01x_serial_init_baud(gd->baudrate);
}

static struct serial_device pl01x_serial_drv = {
	.name	= "pl01x_serial",
	.start	= pl01x_serial_init,
	.stop	= NULL,
	.setbrg	= pl01x_serial_setbrg,
	.putc	= pl01x_serial_putc,
	.puts	= default_serial_puts,
	.getc	= pl01x_serial_getc,
	.tstc	= pl01x_serial_tstc,
};

void pl01x_serial_initialize(void)
{
	serial_register(&pl01x_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &pl01x_serial_drv;
}
#else
int pl01x_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct pl01x_serial_plat *plat = dev_get_plat(dev);
	struct pl01x_priv *priv = dev_get_priv(dev);

	if (!plat->skip_init) {
		pl01x_generic_setbrg(priv->regs, priv->type, plat->clock,
				     baudrate);
	}

	return 0;
}

int pl01x_serial_probe(struct udevice *dev)
{
	struct pl01x_serial_plat *plat = dev_get_plat(dev);
	struct pl01x_priv *priv = dev_get_priv(dev);

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_serial_pl01x *dtplat = &plat->dtplat;

	priv->regs = (struct pl01x_regs *)dtplat->reg[0];
	plat->type = dtplat->type;
#else
	priv->regs = (struct pl01x_regs *)plat->base;
#endif
	priv->type = plat->type;

	if (!plat->skip_init)
		return pl01x_generic_serial_init(priv->regs, priv->type);
	else
		return 0;
}

int pl01x_serial_getc(struct udevice *dev)
{
	struct pl01x_priv *priv = dev_get_priv(dev);

	return pl01x_getc(priv->regs);
}

int pl01x_serial_putc(struct udevice *dev, const char ch)
{
	struct pl01x_priv *priv = dev_get_priv(dev);

	return pl01x_putc(priv->regs, ch);
}

int pl01x_serial_pending(struct udevice *dev, bool input)
{
	struct pl01x_priv *priv = dev_get_priv(dev);
	unsigned int fr = readl(&priv->regs->fr);

	if (input)
		return pl01x_tstc(priv->regs);
	else
		return fr & UART_PL01x_FR_TXFE ? 0 : 1;
}

static const struct dm_serial_ops pl01x_serial_ops = {
	.putc = pl01x_serial_putc,
	.pending = pl01x_serial_pending,
	.getc = pl01x_serial_getc,
	.setbrg = pl01x_serial_setbrg,
};

#if CONFIG_IS_ENABLED(OF_REAL)
static const struct udevice_id pl01x_serial_id[] ={
	{.compatible = "arm,pl011", .data = TYPE_PL011},
	{.compatible = "arm,pl010", .data = TYPE_PL010},
	{}
};

#ifndef CFG_PL011_CLOCK
#define CFG_PL011_CLOCK 0
#endif

int pl01x_serial_of_to_plat(struct udevice *dev)
{
	struct pl01x_serial_plat *plat = dev_get_plat(dev);
	struct clk clk;
	fdt_addr_t addr;
	int ret;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = addr;
	plat->clock = dev_read_u32_default(dev, "clock", CFG_PL011_CLOCK);
	ret = clk_get_by_index(dev, 0, &clk);
	if (!ret) {
		ret = clk_enable(&clk);
		if (ret && ret != -ENOSYS) {
			dev_err(dev, "failed to enable clock\n");
			return ret;
		}

		plat->clock = clk_get_rate(&clk);
		if (IS_ERR_VALUE(plat->clock)) {
			dev_err(dev, "failed to get rate\n");
			return plat->clock;
		}
		debug("%s: CLK %d\n", __func__, plat->clock);
	}
	plat->type = dev_get_driver_data(dev);
	plat->skip_init = dev_read_bool(dev, "skip-init");

	return 0;
}
#endif

U_BOOT_DRIVER(serial_pl01x) = {
	.name	= "serial_pl01x",
	.id	= UCLASS_SERIAL,
#if CONFIG_IS_ENABLED(OF_REAL)
	.of_match = of_match_ptr(pl01x_serial_id),
	.of_to_plat = of_match_ptr(pl01x_serial_of_to_plat),
#endif
	.plat_auto	= sizeof(struct pl01x_serial_plat),
	.probe = pl01x_serial_probe,
	.ops	= &pl01x_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
	.priv_auto	= sizeof(struct pl01x_priv),
};

DM_DRIVER_ALIAS(serial_pl01x, arm_pl011)
DM_DRIVER_ALIAS(serial_pl01x, arm_pl010)
#endif

#if defined(CONFIG_DEBUG_UART_PL010) || defined(CONFIG_DEBUG_UART_PL011)

#include <debug_uart.h>

static void _debug_uart_init(void)
{
#ifndef CONFIG_DEBUG_UART_SKIP_INIT
	struct pl01x_regs *regs = (struct pl01x_regs *)CONFIG_VAL(DEBUG_UART_BASE);
	enum pl01x_type type;

	if (IS_ENABLED(CONFIG_DEBUG_UART_PL011))
		type = TYPE_PL011;
	else
		type = TYPE_PL010;

	pl01x_generic_serial_init(regs, type);
	pl01x_generic_setbrg(regs, type,
			     CONFIG_DEBUG_UART_CLOCK, CONFIG_BAUDRATE);
#endif
}

static inline void _debug_uart_putc(int ch)
{
	struct pl01x_regs *regs = (struct pl01x_regs *)CONFIG_VAL(DEBUG_UART_BASE);

	while (pl01x_putc(regs, ch) == -EAGAIN)
		;
}

DEBUG_UART_FUNCS

#endif
