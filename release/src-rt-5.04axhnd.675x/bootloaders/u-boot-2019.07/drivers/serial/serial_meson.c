// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/compiler.h>
#include <serial.h>

struct meson_uart {
	u32 wfifo;
	u32 rfifo;
	u32 control;
	u32 status;
	u32 misc;
};

struct meson_serial_platdata {
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
	struct meson_serial_platdata *plat = dev->platdata;
	struct meson_uart *const uart = plat->reg;

	meson_serial_init(uart);

	return 0;
}

static int meson_serial_getc(struct udevice *dev)
{
	struct meson_serial_platdata *plat = dev->platdata;
	struct meson_uart *const uart = plat->reg;

	if (readl(&uart->status) & AML_UART_RX_EMPTY)
		return -EAGAIN;

	return readl(&uart->rfifo) & 0xff;
}

static int meson_serial_putc(struct udevice *dev, const char ch)
{
	struct meson_serial_platdata *plat = dev->platdata;
	struct meson_uart *const uart = plat->reg;

	if (readl(&uart->status) & AML_UART_TX_FULL)
		return -EAGAIN;

	writel(ch, &uart->wfifo);

	return 0;
}

static int meson_serial_pending(struct udevice *dev, bool input)
{
	struct meson_serial_platdata *plat = dev->platdata;
	struct meson_uart *const uart = plat->reg;
	uint32_t status = readl(&uart->status);

	if (input)
		return !(status & AML_UART_RX_EMPTY);
	else
		return !(status & AML_UART_TX_FULL);
}

static int meson_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct meson_serial_platdata *plat = dev->platdata;
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->reg = (struct meson_uart *)addr;

	return 0;
}

static const struct dm_serial_ops meson_serial_ops = {
	.putc = meson_serial_putc,
	.pending = meson_serial_pending,
	.getc = meson_serial_getc,
};

static const struct udevice_id meson_serial_ids[] = {
	{ .compatible = "amlogic,meson-uart" },
	{ .compatible = "amlogic,meson-gx-uart" },
	{ }
};

U_BOOT_DRIVER(serial_meson) = {
	.name		= "serial_meson",
	.id		= UCLASS_SERIAL,
	.of_match	= meson_serial_ids,
	.probe		= meson_serial_probe,
	.ops		= &meson_serial_ops,
	.ofdata_to_platdata = meson_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct meson_serial_platdata),
};

#ifdef CONFIG_DEBUG_UART_MESON

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
}

static inline void _debug_uart_putc(int ch)
{
	struct meson_uart *regs = (struct meson_uart *)CONFIG_DEBUG_UART_BASE;

	while (readl(&regs->status) & AML_UART_TX_FULL)
		;

	writel(ch, &regs->wfifo);
}

DEBUG_UART_FUNCS

#endif
