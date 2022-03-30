// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2011-2012 Xilinx, Inc. All rights reserved.
 */

#include <clk.h>
#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <watchdog.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <serial.h>

DECLARE_GLOBAL_DATA_PTR;

#define ZYNQ_UART_SR_TXACTIVE	BIT(11) /* TX active */
#define ZYNQ_UART_SR_TXFULL	BIT(4) /* TX FIFO full */
#define ZYNQ_UART_SR_RXEMPTY	BIT(1) /* RX FIFO empty */

#define ZYNQ_UART_CR_TX_EN	BIT(4) /* TX enabled */
#define ZYNQ_UART_CR_RX_EN	BIT(2) /* RX enabled */
#define ZYNQ_UART_CR_TXRST	BIT(1) /* TX logic reset */
#define ZYNQ_UART_CR_RXRST	BIT(0) /* RX logic reset */

#define ZYNQ_UART_MR_PARITY_NONE	0x00000020  /* No parity mode */

struct uart_zynq {
	u32 control; /* 0x0 - Control Register [8:0] */
	u32 mode; /* 0x4 - Mode Register [10:0] */
	u32 reserved1[4];
	u32 baud_rate_gen; /* 0x18 - Baud Rate Generator [15:0] */
	u32 reserved2[4];
	u32 channel_sts; /* 0x2c - Channel Status [11:0] */
	u32 tx_rx_fifo; /* 0x30 - FIFO [15:0] or [7:0] */
	u32 baud_rate_divider; /* 0x34 - Baud Rate Divider [7:0] */
};

struct zynq_uart_platdata {
	struct uart_zynq *regs;
};

/* Set up the baud rate in gd struct */
static void _uart_zynq_serial_setbrg(struct uart_zynq *regs,
				     unsigned long clock, unsigned long baud)
{
	/* Calculation results. */
	unsigned int calc_bauderror, bdiv, bgen;
	unsigned long calc_baud = 0;

	/* Covering case where input clock is so slow */
	if (clock < 1000000 && baud > 4800)
		baud = 4800;

	/*                master clock
	 * Baud rate = ------------------
	 *              bgen * (bdiv + 1)
	 *
	 * Find acceptable values for baud generation.
	 */
	for (bdiv = 4; bdiv < 255; bdiv++) {
		bgen = clock / (baud * (bdiv + 1));
		if (bgen < 2 || bgen > 65535)
			continue;

		calc_baud = clock / (bgen * (bdiv + 1));

		/*
		 * Use first calculated baudrate with
		 * an acceptable (<3%) error
		 */
		if (baud > calc_baud)
			calc_bauderror = baud - calc_baud;
		else
			calc_bauderror = calc_baud - baud;
		if (((calc_bauderror * 100) / baud) < 3)
			break;
	}

	writel(bdiv, &regs->baud_rate_divider);
	writel(bgen, &regs->baud_rate_gen);
}

/* Initialize the UART, with...some settings. */
static void _uart_zynq_serial_init(struct uart_zynq *regs)
{
	/* RX/TX enabled & reset */
	writel(ZYNQ_UART_CR_TX_EN | ZYNQ_UART_CR_RX_EN | ZYNQ_UART_CR_TXRST | \
					ZYNQ_UART_CR_RXRST, &regs->control);
	writel(ZYNQ_UART_MR_PARITY_NONE, &regs->mode); /* 8 bit, no parity */
}

static int _uart_zynq_serial_putc(struct uart_zynq *regs, const char c)
{
	if (readl(&regs->channel_sts) & ZYNQ_UART_SR_TXFULL)
		return -EAGAIN;

	writel(c, &regs->tx_rx_fifo);

	return 0;
}

static int zynq_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct zynq_uart_platdata *platdata = dev_get_platdata(dev);
	unsigned long clock;

	int ret;
	struct clk clk;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		dev_err(dev, "failed to get clock\n");
		return ret;
	}

	clock = clk_get_rate(&clk);
	if (IS_ERR_VALUE(clock)) {
		dev_err(dev, "failed to get rate\n");
		return clock;
	}
	debug("%s: CLK %ld\n", __func__, clock);

	ret = clk_enable(&clk);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	_uart_zynq_serial_setbrg(platdata->regs, clock, baudrate);

	return 0;
}

static int zynq_serial_probe(struct udevice *dev)
{
	struct zynq_uart_platdata *platdata = dev_get_platdata(dev);

	/* No need to reinitialize the UART after relocation */
	if (gd->flags & GD_FLG_RELOC)
		return 0;

	_uart_zynq_serial_init(platdata->regs);

	return 0;
}

static int zynq_serial_getc(struct udevice *dev)
{
	struct zynq_uart_platdata *platdata = dev_get_platdata(dev);
	struct uart_zynq *regs = platdata->regs;

	if (readl(&regs->channel_sts) & ZYNQ_UART_SR_RXEMPTY)
		return -EAGAIN;

	return readl(&regs->tx_rx_fifo);
}

static int zynq_serial_putc(struct udevice *dev, const char ch)
{
	struct zynq_uart_platdata *platdata = dev_get_platdata(dev);

	return _uart_zynq_serial_putc(platdata->regs, ch);
}

static int zynq_serial_pending(struct udevice *dev, bool input)
{
	struct zynq_uart_platdata *platdata = dev_get_platdata(dev);
	struct uart_zynq *regs = platdata->regs;

	if (input)
		return !(readl(&regs->channel_sts) & ZYNQ_UART_SR_RXEMPTY);
	else
		return !!(readl(&regs->channel_sts) & ZYNQ_UART_SR_TXACTIVE);
}

static int zynq_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct zynq_uart_platdata *platdata = dev_get_platdata(dev);

	platdata->regs = (struct uart_zynq *)dev_read_addr(dev);
	if (IS_ERR(platdata->regs))
		return PTR_ERR(platdata->regs);

	return 0;
}

static const struct dm_serial_ops zynq_serial_ops = {
	.putc = zynq_serial_putc,
	.pending = zynq_serial_pending,
	.getc = zynq_serial_getc,
	.setbrg = zynq_serial_setbrg,
};

static const struct udevice_id zynq_serial_ids[] = {
	{ .compatible = "xlnx,xuartps" },
	{ .compatible = "cdns,uart-r1p8" },
	{ .compatible = "cdns,uart-r1p12" },
	{ }
};

U_BOOT_DRIVER(serial_zynq) = {
	.name	= "serial_zynq",
	.id	= UCLASS_SERIAL,
	.of_match = zynq_serial_ids,
	.ofdata_to_platdata = zynq_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct zynq_uart_platdata),
	.probe = zynq_serial_probe,
	.ops	= &zynq_serial_ops,
};

#ifdef CONFIG_DEBUG_UART_ZYNQ
static inline void _debug_uart_init(void)
{
	struct uart_zynq *regs = (struct uart_zynq *)CONFIG_DEBUG_UART_BASE;

	_uart_zynq_serial_init(regs);
	_uart_zynq_serial_setbrg(regs, CONFIG_DEBUG_UART_CLOCK,
				 CONFIG_BAUDRATE);
}

static inline void _debug_uart_putc(int ch)
{
	struct uart_zynq *regs = (struct uart_zynq *)CONFIG_DEBUG_UART_BASE;

	while (_uart_zynq_serial_putc(regs, ch) == -EAGAIN)
		WATCHDOG_RESET();
}

DEBUG_UART_FUNCS

#endif
