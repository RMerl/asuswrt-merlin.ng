// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011-2015 Vladimir Zapolskiy <vz@mleia.com>
 */

#include <common.h>
#include <dm.h>
#include <serial.h>
#include <dm/platform_data/lpc32xx_hsuart.h>

#include <asm/arch/uart.h>
#include <linux/compiler.h>

struct lpc32xx_hsuart_priv {
	struct hsuart_regs *hsuart;
};

static int lpc32xx_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct lpc32xx_hsuart_priv *priv = dev_get_priv(dev);
	struct hsuart_regs *hsuart = priv->hsuart;
	u32 div;

	/* UART rate = PERIPH_CLK / ((HSU_RATE + 1) x 14) */
	div = (get_serial_clock() / 14 + baudrate / 2) / baudrate - 1;
	if (div > 255)
		div = 255;

	writel(div, &hsuart->rate);

	return 0;
}

static int lpc32xx_serial_getc(struct udevice *dev)
{
	struct lpc32xx_hsuart_priv *priv = dev_get_priv(dev);
	struct hsuart_regs *hsuart = priv->hsuart;

	if (!(readl(&hsuart->level) & HSUART_LEVEL_RX))
		return -EAGAIN;

	return readl(&hsuart->rx) & HSUART_RX_DATA;
}

static int lpc32xx_serial_putc(struct udevice *dev, const char c)
{
	struct lpc32xx_hsuart_priv *priv = dev_get_priv(dev);
	struct hsuart_regs *hsuart = priv->hsuart;

	/* Wait for empty FIFO */
	if (readl(&hsuart->level) & HSUART_LEVEL_TX)
		return -EAGAIN;

	writel(c, &hsuart->tx);

	return 0;
}

static int lpc32xx_serial_pending(struct udevice *dev, bool input)
{
	struct lpc32xx_hsuart_priv *priv = dev_get_priv(dev);
	struct hsuart_regs *hsuart = priv->hsuart;

	if (input) {
		if (readl(&hsuart->level) & HSUART_LEVEL_RX)
			return 1;
	} else {
		if (readl(&hsuart->level) & HSUART_LEVEL_TX)
			return 1;
	}

	return 0;
}

static int lpc32xx_serial_init(struct hsuart_regs *hsuart)
{
	/* Disable hardware RTS and CTS flow control, set up RX and TX FIFO */
	writel(HSUART_CTRL_TMO_16 | HSUART_CTRL_HSU_OFFSET(20) |
	       HSUART_CTRL_HSU_RX_TRIG_32 | HSUART_CTRL_HSU_TX_TRIG_0,
	       &hsuart->ctrl);

	return 0;
}

static int lpc32xx_hsuart_probe(struct udevice *dev)
{
	struct lpc32xx_hsuart_platdata *platdata = dev_get_platdata(dev);
	struct lpc32xx_hsuart_priv *priv = dev_get_priv(dev);

	priv->hsuart = (struct hsuart_regs *)platdata->base;

	lpc32xx_serial_init(priv->hsuart);

	return 0;
}

static const struct dm_serial_ops lpc32xx_hsuart_ops = {
	.setbrg	= lpc32xx_serial_setbrg,
	.getc	= lpc32xx_serial_getc,
	.putc	= lpc32xx_serial_putc,
	.pending = lpc32xx_serial_pending,
};

U_BOOT_DRIVER(lpc32xx_hsuart) = {
	.name	= "lpc32xx_hsuart",
	.id	= UCLASS_SERIAL,
	.probe	= lpc32xx_hsuart_probe,
	.ops	= &lpc32xx_hsuart_ops,
	.priv_auto_alloc_size = sizeof(struct lpc32xx_hsuart_priv),
	.flags	= DM_FLAG_PRE_RELOC,
};
