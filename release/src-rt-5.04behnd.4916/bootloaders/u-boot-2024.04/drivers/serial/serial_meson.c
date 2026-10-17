// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/compiler.h>
#include <serial.h>
#include <clk.h>

struct meson_uart {
	u32 wfifo;
	u32 rfifo;
	u32 control;
	u32 status;
	u32 misc;
	u32 reg5; /* New baud control register */
};

struct meson_serial_plat {
	struct meson_uart *reg;
};

/* AML_UART_STATUS bits */
#define AML_UART_PARITY_ERR		BIT(16)
#define AML_UART_FRAME_ERR		BIT(17)
#define AML_UART_TX_FIFO_WERR		BIT(18)
#define AML_UART_RX_EMPTY		BIT(20)
#define AML_UART_TX_FULL		BIT(21)
#define AML_UART_TX_EMPTY		BIT(22)
#define AML_UART_XMIT_BUSY		BIT(25)
#define AML_UART_ERR			(AML_UART_PARITY_ERR | \
					 AML_UART_FRAME_ERR  | \
					 AML_UART_TX_FIFO_WERR)

/* AML_UART_CONTROL bits */
#define AML_UART_TX_EN			BIT(12)
#define AML_UART_RX_EN			BIT(13)
#define AML_UART_TX_RST			BIT(22)
#define AML_UART_RX_RST			BIT(23)
#define AML_UART_CLR_ERR		BIT(24)

/* AML_UART_REG5 bits */
#define AML_UART_REG5_XTAL_DIV2		BIT(27)
#define AML_UART_REG5_XTAL_CLK_SEL	BIT(26) /* default 0 (div by 3), 1 for no div */
#define AML_UART_REG5_USE_XTAL_CLK	BIT(24) /* default 1 (use crystal as clock source) */
#define AML_UART_REG5_USE_NEW_BAUD	BIT(23) /* default 1 (use new baud rate register) */
#define AML_UART_REG5_BAUD_MASK		0x7fffff

static u32 meson_calc_baud_divisor(ulong src_rate, u32 baud)
{
	/*
	 * Usually src_rate is 24 MHz (from crystal) as clock source for serial
	 * device. Since 8 Mb/s is the maximum supported baud rate, use div by 3
	 * to derive baud rate. This choice is used also in meson_serial_setbrg.
	 */
	return DIV_ROUND_CLOSEST(src_rate / 3, baud) - 1;
}

static void meson_serial_set_baud(struct meson_uart *uart, ulong src_rate, u32 baud)
{
	/*
	 * Set crystal divided by 3 (regardless of device tree clock property)
	 * as clock source and the corresponding divisor to approximate baud
	 */
	u32 divisor = meson_calc_baud_divisor(src_rate, baud);
	u32 val = AML_UART_REG5_USE_XTAL_CLK | AML_UART_REG5_USE_NEW_BAUD |
		(divisor & AML_UART_REG5_BAUD_MASK);
	writel(val, &uart->reg5);
}

static void meson_serial_init(struct meson_uart *uart)
{
	u32 val;

	val = readl(&uart->control);
	val |= (AML_UART_RX_RST | AML_UART_TX_RST | AML_UART_CLR_ERR);
	writel(val, &uart->control);
	val &= ~(AML_UART_RX_RST | AML_UART_TX_RST | AML_UART_CLR_ERR);
	writel(val, &uart->control);
	val |= (AML_UART_RX_EN | AML_UART_TX_EN);
	writel(val, &uart->control);
}

static int meson_serial_probe(struct udevice *dev)
{
	struct meson_serial_plat *plat = dev_get_plat(dev);
	struct meson_uart *const uart = plat->reg;
	struct clk per_clk;
	int ret = clk_get_by_name(dev, "baud", &per_clk);

	if (ret)
		return ret;
	ulong rate = clk_get_rate(&per_clk);

	meson_serial_set_baud(uart, rate, CONFIG_BAUDRATE);
	meson_serial_init(uart);

	return 0;
}

static void meson_serial_rx_error(struct udevice *dev)
{
	struct meson_serial_plat *plat = dev_get_plat(dev);
	struct meson_uart *const uart = plat->reg;
	u32 val = readl(&uart->control);

	/* Clear error */
	val |= AML_UART_CLR_ERR;
	writel(val, &uart->control);
	val &= ~AML_UART_CLR_ERR;
	writel(val, &uart->control);

	/* Remove spurious byte from fifo */
	readl(&uart->rfifo);
}

static int meson_serial_getc(struct udevice *dev)
{
	struct meson_serial_plat *plat = dev_get_plat(dev);
	struct meson_uart *const uart = plat->reg;
	uint32_t status = readl(&uart->status);

	if (status & AML_UART_RX_EMPTY)
		return -EAGAIN;

	if (status & AML_UART_ERR) {
		meson_serial_rx_error(dev);
		return -EIO;
	}

	return readl(&uart->rfifo) & 0xff;
}

static int meson_serial_putc(struct udevice *dev, const char ch)
{
	struct meson_serial_plat *plat = dev_get_plat(dev);
	struct meson_uart *const uart = plat->reg;

	if (readl(&uart->status) & AML_UART_TX_FULL)
		return -EAGAIN;

	writel(ch, &uart->wfifo);

	return 0;
}

static int meson_serial_setbrg(struct udevice *dev, const int baud)
{
	/*
	 * Change device baud rate if baud is reasonable (considering a 23 bit
	 * counter with an 8 MHz clock input) and the actual baud
	 * rate is within 2% of the requested value (2% is arbitrary).
	 */
	if (baud < 1 || baud > 8000000)
		return -EINVAL;

	struct meson_serial_plat *const plat = dev_get_plat(dev);
	struct meson_uart *const uart = plat->reg;
	struct clk per_clk;
	int ret = clk_get_by_name(dev, "baud", &per_clk);

	if (ret)
		return ret;
	ulong rate = clk_get_rate(&per_clk);
	u32 divisor = meson_calc_baud_divisor(rate, baud);
	u32 calc_baud = (rate / 3) / (divisor + 1);
	u32 calc_err = baud > calc_baud ? baud - calc_baud : calc_baud - baud;

	if (((calc_err * 100) / baud) > 2)
		return -EINVAL;

	meson_serial_set_baud(uart, rate, baud);

	return 0;
}

static int meson_serial_pending(struct udevice *dev, bool input)
{
	struct meson_serial_plat *plat = dev_get_plat(dev);
	struct meson_uart *const uart = plat->reg;
	uint32_t status = readl(&uart->status);

	if (input) {
		if (status & AML_UART_RX_EMPTY)
			return false;

		/*
		 * Handle and drop any RX error here to avoid
		 * returning true here when an error byte is in the FIFO
		 */
		if (status & AML_UART_ERR) {
			meson_serial_rx_error(dev);
			return false;
		}

		return true;
	} else {
		if (status & AML_UART_TX_EMPTY)
			return false;

		return true;
	}
}

static int meson_serial_of_to_plat(struct udevice *dev)
{
	struct meson_serial_plat *plat = dev_get_plat(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->reg = (struct meson_uart *)addr;

	return 0;
}

static const struct dm_serial_ops meson_serial_ops = {
	.putc = meson_serial_putc,
	.pending = meson_serial_pending,
	.getc = meson_serial_getc,
	.setbrg = meson_serial_setbrg,
};

static const struct udevice_id meson_serial_ids[] = {
	{ .compatible = "amlogic,meson-uart" },
	{ .compatible = "amlogic,meson-gx-uart" },
	{ .compatible = "amlogic,meson-a1-uart" },
	{ }
};

U_BOOT_DRIVER(serial_meson) = {
	.name		= "serial_meson",
	.id		= UCLASS_SERIAL,
	.of_match	= meson_serial_ids,
	.probe		= meson_serial_probe,
	.ops		= &meson_serial_ops,
	.of_to_plat = meson_serial_of_to_plat,
	.plat_auto	= sizeof(struct meson_serial_plat),
};

#ifdef CONFIG_DEBUG_UART_MESON

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
}

static inline void _debug_uart_putc(int ch)
{
	struct meson_uart *regs = (struct meson_uart *)CONFIG_VAL(DEBUG_UART_BASE);

	while (readl(&regs->status) & AML_UART_TX_FULL)
		;

	writel(ch, &regs->wfifo);
}

DEBUG_UART_FUNCS

#endif
