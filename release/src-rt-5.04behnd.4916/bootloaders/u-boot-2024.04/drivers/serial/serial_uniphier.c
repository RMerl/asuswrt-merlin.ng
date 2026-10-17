// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012-2015 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <dm.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/io.h>
#include <linux/serial_reg.h>
#include <linux/sizes.h>
#include <linux/errno.h>
#include <serial.h>
#include <fdtdec.h>

#define UNIPHIER_UART_REGSHIFT		2

#define UNIPHIER_UART_RX		(0 << (UNIPHIER_UART_REGSHIFT))
#define UNIPHIER_UART_TX		UNIPHIER_UART_RX
/* bit[15:8] = CHAR, bit[7:0] = FCR */
#define UNIPHIER_UART_CHAR_FCR		(3 << (UNIPHIER_UART_REGSHIFT))
#define   UNIPHIER_UART_FCR_MASK		GENMASK(7, 0)
/* bit[15:8] = LCR, bit[7:0] = MCR */
#define UNIPHIER_UART_LCR_MCR		(4 << (UNIPHIER_UART_REGSHIFT))
#define   UNIPHIER_UART_LCR_MASK		GENMASK(15, 8)
#define UNIPHIER_UART_LSR		(5 << (UNIPHIER_UART_REGSHIFT))
/* Divisor Latch Register */
#define UNIPHIER_UART_DLR		(9 << (UNIPHIER_UART_REGSHIFT))

struct uniphier_serial_priv {
	void __iomem *membase;
	unsigned int uartclk;
};

static int uniphier_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct uniphier_serial_priv *priv = dev_get_priv(dev);
	static const unsigned int mode_x_div = 16;
	unsigned int divisor;

	divisor = DIV_ROUND_CLOSEST(priv->uartclk, mode_x_div * baudrate);

	/* flush the trasmitter before changing hw setting */
	while (!(readl(priv->membase + UNIPHIER_UART_LSR) & UART_LSR_TEMT))
		;

	writel(divisor, priv->membase + UNIPHIER_UART_DLR);

	return 0;
}

static int uniphier_serial_getc(struct udevice *dev)
{
	struct uniphier_serial_priv *priv = dev_get_priv(dev);

	if (!(readl(priv->membase + UNIPHIER_UART_LSR) & UART_LSR_DR))
		return -EAGAIN;

	return readl(priv->membase + UNIPHIER_UART_RX);
}

static int uniphier_serial_putc(struct udevice *dev, const char c)
{
	struct uniphier_serial_priv *priv = dev_get_priv(dev);

	if (!(readl(priv->membase + UNIPHIER_UART_LSR) & UART_LSR_THRE))
		return -EAGAIN;

	writel(c, priv->membase + UNIPHIER_UART_TX);

	return 0;
}

static int uniphier_serial_pending(struct udevice *dev, bool input)
{
	struct uniphier_serial_priv *priv = dev_get_priv(dev);

	if (input)
		return readl(priv->membase + UNIPHIER_UART_LSR) & UART_LSR_DR;
	else
		return !(readl(priv->membase + UNIPHIER_UART_LSR) & UART_LSR_THRE);
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
	const struct uniphier_serial_clk_data *clk_data;
	ofnode root_node;
	fdt_addr_t base;
	u32 tmp;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->membase = devm_ioremap(dev, base, SZ_64);
	if (!priv->membase)
		return -ENOMEM;

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

	/* flush the trasmitter before changing hw setting */
	while (!(readl(priv->membase + UNIPHIER_UART_LSR) & UART_LSR_TEMT))
		;

	/* enable FIFO */
	tmp = readl(priv->membase + UNIPHIER_UART_CHAR_FCR);
	tmp &= ~UNIPHIER_UART_FCR_MASK;
	tmp |= FIELD_PREP(UNIPHIER_UART_FCR_MASK, UART_FCR_ENABLE_FIFO);
	writel(tmp, priv->membase + UNIPHIER_UART_CHAR_FCR);

	tmp = readl(priv->membase + UNIPHIER_UART_LCR_MCR);
	tmp &= ~UNIPHIER_UART_LCR_MASK;
	tmp |= FIELD_PREP(UNIPHIER_UART_LCR_MASK, UART_LCR_WLEN8);
	writel(tmp, priv->membase + UNIPHIER_UART_LCR_MCR);

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
	.priv_auto	= sizeof(struct uniphier_serial_priv),
	.ops = &uniphier_serial_ops,
};
