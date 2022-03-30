// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012-2015 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <dm.h>
#include <linux/bug.h>
#include <linux/io.h>
#include <linux/serial_reg.h>
#include <linux/sizes.h>
#include <linux/errno.h>
#include <serial.h>
#include <fdtdec.h>

/*
 * Note: Register map is slightly different from that of 16550.
 */
struct uniphier_serial {
	u32 rx;			/* In:  Receive buffer */
#define tx rx			/* Out: Transmit buffer */
	u32 ier;		/* Interrupt Enable Register */
	u32 iir;		/* In: Interrupt ID Register */
	u32 char_fcr;		/* Charactor / FIFO Control Register */
	u32 lcr_mcr;		/* Line/Modem Control Register */
#define LCR_SHIFT	8
#define LCR_MASK	(0xff << (LCR_SHIFT))
	u32 lsr;		/* In: Line Status Register */
	u32 msr;		/* In: Modem Status Register */
	u32 __rsv0;
	u32 __rsv1;
	u32 dlr;		/* Divisor Latch Register */
};

struct uniphier_serial_priv {
	struct uniphier_serial __iomem *membase;
	unsigned int uartclk;
};

#define uniphier_serial_port(dev)	\
	((struct uniphier_serial_priv *)dev_get_priv(dev))->membase

static int uniphier_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct uniphier_serial_priv *priv = dev_get_priv(dev);
	struct uniphier_serial __iomem *port = uniphier_serial_port(dev);
	const unsigned int mode_x_div = 16;
	unsigned int divisor;

	divisor = DIV_ROUND_CLOSEST(priv->uartclk, mode_x_div * baudrate);

	writel(divisor, &port->dlr);

	return 0;
}

static int uniphier_serial_getc(struct udevice *dev)
{
	struct uniphier_serial __iomem *port = uniphier_serial_port(dev);

	if (!(readl(&port->lsr) & UART_LSR_DR))
		return -EAGAIN;

	return readl(&port->rx);
}

static int uniphier_serial_putc(struct udevice *dev, const char c)
{
	struct uniphier_serial __iomem *port = uniphier_serial_port(dev);

	if (!(readl(&port->lsr) & UART_LSR_THRE))
		return -EAGAIN;

	writel(c, &port->tx);

	return 0;
}

static int uniphier_serial_pending(struct udevice *dev, bool input)
{
	struct uniphier_serial __iomem *port = uniphier_serial_port(dev);

	if (input)
		return readl(&port->lsr) & UART_LSR_DR;
	else
		return !(readl(&port->lsr) & UART_LSR_THRE);
}

/*
 * SPL does not have enough memory footprint for the clock driver.
 * Hardcode clock frequency for each SoC.
 */
struct uniphier_serial_clk_data {
	const char *compatible;
	unsigned int clk_rate;
};

static const struct uniphier_serial_clk_data uniphier_serial_clk_data[] = {
	{ .compatible = "socionext,uniphier-ld4",  .clk_rate = 36864000 },
	{ .compatible = "socionext,uniphier-pro4", .clk_rate = 73728000 },
	{ .compatible = "socionext,uniphier-sld8", .clk_rate = 80000000 },
	{ .compatible = "socionext,uniphier-pro5", .clk_rate = 73728000 },
	{ .compatible = "socionext,uniphier-pxs2", .clk_rate = 88888888 },
	{ .compatible = "socionext,uniphier-ld6b", .clk_rate = 88888888 },
	{ .compatible = "socionext,uniphier-ld11", .clk_rate = 58823529 },
	{ .compatible = "socionext,uniphier-ld20", .clk_rate = 58823529 },
	{ .compatible = "socionext,uniphier-pxs3", .clk_rate = 58823529 },
	{ /* sentinel */ },
};

static int uniphier_serial_probe(struct udevice *dev)
{
	struct uniphier_serial_priv *priv = dev_get_priv(dev);
	struct uniphier_serial __iomem *port;
	const struct uniphier_serial_clk_data *clk_data;
	ofnode root_node;
	fdt_addr_t base;
	u32 tmp;

	base = devfdt_get_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	port = devm_ioremap(dev, base, SZ_64);
	if (!port)
		return -ENOMEM;

	priv->membase = port;

	root_node = ofnode_path("/");
	clk_data = uniphier_serial_clk_data;
	while (clk_data->compatible) {
		if (ofnode_device_is_compatible(root_node,
						clk_data->compatible))
			break;
		clk_data++;
	}

	if (WARN_ON(!clk_data->compatible))
		return -ENOTSUPP;

	priv->uartclk = clk_data->clk_rate;

	tmp = readl(&port->lcr_mcr);
	tmp &= ~LCR_MASK;
	tmp |= UART_LCR_WLEN8 << LCR_SHIFT;
	writel(tmp, &port->lcr_mcr);

	return 0;
}

static const struct udevice_id uniphier_uart_of_match[] = {
	{ .compatible = "socionext,uniphier-uart" },
	{ /* sentinel */ }
};

static const struct dm_serial_ops uniphier_serial_ops = {
	.setbrg = uniphier_serial_setbrg,
	.getc = uniphier_serial_getc,
	.putc = uniphier_serial_putc,
	.pending = uniphier_serial_pending,
};

U_BOOT_DRIVER(uniphier_serial) = {
	.name = "uniphier-uart",
	.id = UCLASS_SERIAL,
	.of_match = uniphier_uart_of_match,
	.probe = uniphier_serial_probe,
	.priv_auto_alloc_size = sizeof(struct uniphier_serial_priv),
	.ops = &uniphier_serial_ops,
};
