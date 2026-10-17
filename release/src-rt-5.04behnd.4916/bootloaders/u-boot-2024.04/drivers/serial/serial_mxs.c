// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Marek Vasut <marex@denx.de>
 */
#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <serial.h>
#include <wait_bit.h>

#define SET_REG					0x4
#define CLR_REG					0x8

#define AUART_CTRL0				0x00
#define AUART_CTRL1				0x10
#define AUART_CTRL2				0x20
#define AUART_LINECTRL				0x30
#define AUART_INTR				0x50
#define AUART_DATA				0x60
#define AUART_STAT				0x70

#define AUART_CTRL0_SFTRST			BIT(31)
#define AUART_CTRL0_CLKGATE			BIT(30)

#define AUART_CTRL2_UARTEN			BIT(0)

#define AUART_LINECTRL_BAUD_DIVINT(v)		(((v) & 0xffff) << 16)
#define AUART_LINECTRL_BAUD_DIVFRAC(v)		(((v) & 0x3f) << 8)
#define AUART_LINECTRL_WLEN(v)			((((v) - 5) & 0x3) << 5)

#define AUART_STAT_TXFE				BIT(27)
#define AUART_STAT_TXFF				BIT(25)
#define AUART_STAT_RXFE				BIT(24)

#define AUART_CLK				24000000

struct mxs_auart_uart_priv {
	void __iomem *base;
};

static int mxs_auart_uart_setbrg(struct udevice *dev, int baudrate)
{
	struct mxs_auart_uart_priv *priv = dev_get_priv(dev);
	u32 div;

	writel(AUART_CTRL0_CLKGATE, priv->base + AUART_CTRL0 + CLR_REG);
	writel(AUART_CTRL0_SFTRST, priv->base + AUART_CTRL0 + CLR_REG);

	writel(AUART_CTRL2_UARTEN, priv->base + AUART_CTRL2 + SET_REG);

	writel(0, priv->base + AUART_INTR);

	div = DIV_ROUND_CLOSEST(AUART_CLK * 32, baudrate);

	/* Disable FIFO, baudrate, 8N1. */
	writel(AUART_LINECTRL_BAUD_DIVFRAC(div & 0x3F) |
	       AUART_LINECTRL_BAUD_DIVINT(div >> 6) |
	       AUART_LINECTRL_WLEN(8),
	       priv->base + AUART_LINECTRL);

	return 0;
}

static int mxs_auart_uart_pending(struct udevice *dev, bool input)
{
	struct mxs_auart_uart_priv *priv = dev_get_priv(dev);
	u32 stat = readl(priv->base + AUART_STAT);

	if (input)
		return !(stat & AUART_STAT_RXFE);

	return !!(stat & AUART_STAT_TXFE);
}

static int mxs_auart_uart_putc(struct udevice *dev, const char ch)
{
	struct mxs_auart_uart_priv *priv = dev_get_priv(dev);
	u32 stat = readl(priv->base + AUART_STAT);

	if (stat & AUART_STAT_TXFF)
		return -EAGAIN;

	writel(ch, priv->base + AUART_DATA);

	return 0;
}

static int mxs_auart_uart_getc(struct udevice *dev)
{
	struct mxs_auart_uart_priv *priv = dev_get_priv(dev);

	if (!mxs_auart_uart_pending(dev, true))
		return -EAGAIN;

	return readl(priv->base + AUART_DATA) & 0xff;
}

static int mxs_auart_uart_probe(struct udevice *dev)
{
	struct mxs_auart_uart_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	return mxs_auart_uart_setbrg(dev, CONFIG_BAUDRATE);
}

static const struct dm_serial_ops mxs_auart_uart_ops = {
	.putc		= mxs_auart_uart_putc,
	.pending	= mxs_auart_uart_pending,
	.getc		= mxs_auart_uart_getc,
	.setbrg		= mxs_auart_uart_setbrg,
};

static const struct udevice_id mxs_auart_uart_ids[] = {
	{ .compatible = "fsl,imx23-auart", },
	{ .compatible = "fsl,imx28-auart", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mxs_auart_serial) = {
	.name		= "mxs-auart",
	.id		= UCLASS_SERIAL,
	.of_match	= mxs_auart_uart_ids,
	.probe		= mxs_auart_uart_probe,
	.ops		= &mxs_auart_uart_ops,
	.priv_auto	= sizeof(struct mxs_auart_uart_priv),
};
