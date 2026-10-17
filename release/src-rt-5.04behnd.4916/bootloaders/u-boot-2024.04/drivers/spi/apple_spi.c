// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 * Copyright The Asahi Linux Contributors
 */

#include <common.h>
#include <dm.h>
#include <clk.h>
#include <spi.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/delay.h>

#define APPLE_SPI_CTRL			0x000
#define APPLE_SPI_CTRL_RUN		BIT(0)
#define APPLE_SPI_CTRL_TX_RESET		BIT(2)
#define APPLE_SPI_CTRL_RX_RESET		BIT(3)

#define APPLE_SPI_CFG			0x004
#define APPLE_SPI_CFG_CPHA		BIT(1)
#define APPLE_SPI_CFG_CPOL		BIT(2)
#define APPLE_SPI_CFG_MODE		GENMASK(6, 5)
#define APPLE_SPI_CFG_MODE_POLLED	0
#define APPLE_SPI_CFG_MODE_IRQ		1
#define APPLE_SPI_CFG_MODE_DMA		2
#define APPLE_SPI_CFG_IE_RXCOMPLETE	BIT(7)
#define APPLE_SPI_CFG_IE_TXRXTHRESH	BIT(8)
#define APPLE_SPI_CFG_LSB_FIRST		BIT(13)
#define APPLE_SPI_CFG_WORD_SIZE		GENMASK(16, 15)
#define APPLE_SPI_CFG_WORD_SIZE_8B	0
#define APPLE_SPI_CFG_WORD_SIZE_16B	1
#define APPLE_SPI_CFG_WORD_SIZE_32B	2
#define APPLE_SPI_CFG_FIFO_THRESH	GENMASK(18, 17)
#define APPLE_SPI_CFG_FIFO_THRESH_8B	0
#define APPLE_SPI_CFG_FIFO_THRESH_4B	1
#define APPLE_SPI_CFG_FIFO_THRESH_1B	2
#define APPLE_SPI_CFG_IE_TXCOMPLETE	BIT(21)

#define APPLE_SPI_STATUS		0x008
#define APPLE_SPI_STATUS_RXCOMPLETE	BIT(0)
#define APPLE_SPI_STATUS_TXRXTHRESH	BIT(1)
#define APPLE_SPI_STATUS_TXCOMPLETE	BIT(2)

#define APPLE_SPI_PIN			0x00c
#define APPLE_SPI_PIN_KEEP_MOSI		BIT(0)
#define APPLE_SPI_PIN_CS		BIT(1)

#define APPLE_SPI_TXDATA		0x010
#define APPLE_SPI_RXDATA		0x020
#define APPLE_SPI_CLKDIV		0x030
#define APPLE_SPI_CLKDIV_MIN		0x002
#define APPLE_SPI_CLKDIV_MAX		0x7ff
#define APPLE_SPI_RXCNT			0x034
#define APPLE_SPI_WORD_DELAY		0x038
#define APPLE_SPI_TXCNT			0x04c

#define APPLE_SPI_FIFOSTAT		0x10c
#define APPLE_SPI_FIFOSTAT_TXFULL	BIT(4)
#define APPLE_SPI_FIFOSTAT_LEVEL_TX	GENMASK(15, 8)
#define APPLE_SPI_FIFOSTAT_RXEMPTY	BIT(20)
#define APPLE_SPI_FIFOSTAT_LEVEL_RX	GENMASK(31, 24)

#define APPLE_SPI_IE_XFER		0x130
#define APPLE_SPI_IF_XFER		0x134
#define APPLE_SPI_XFER_RXCOMPLETE	BIT(0)
#define APPLE_SPI_XFER_TXCOMPLETE	BIT(1)

#define APPLE_SPI_IE_FIFO		0x138
#define APPLE_SPI_IF_FIFO		0x13c
#define APPLE_SPI_FIFO_RXTHRESH		BIT(4)
#define APPLE_SPI_FIFO_TXTHRESH		BIT(5)
#define APPLE_SPI_FIFO_RXFULL		BIT(8)
#define APPLE_SPI_FIFO_TXEMPTY		BIT(9)
#define APPLE_SPI_FIFO_RXUNDERRUN	BIT(16)
#define APPLE_SPI_FIFO_TXOVERFLOW	BIT(17)

#define APPLE_SPI_SHIFTCFG		0x150
#define APPLE_SPI_SHIFTCFG_CLK_ENABLE	BIT(0)
#define APPLE_SPI_SHIFTCFG_CS_ENABLE	BIT(1)
#define APPLE_SPI_SHIFTCFG_AND_CLK_DATA	BIT(8)
#define APPLE_SPI_SHIFTCFG_CS_AS_DATA	BIT(9)
#define APPLE_SPI_SHIFTCFG_TX_ENABLE	BIT(10)
#define APPLE_SPI_SHIFTCFG_RX_ENABLE	BIT(11)
#define APPLE_SPI_SHIFTCFG_BITS		GENMASK(21, 16)
#define APPLE_SPI_SHIFTCFG_OVERRIDE_CS	BIT(24)

#define APPLE_SPI_PINCFG		0x154
#define APPLE_SPI_PINCFG_KEEP_CLK	BIT(0)
#define APPLE_SPI_PINCFG_KEEP_CS	BIT(1)
#define APPLE_SPI_PINCFG_KEEP_MOSI	BIT(2)
#define APPLE_SPI_PINCFG_CLK_IDLE_VAL	BIT(8)
#define APPLE_SPI_PINCFG_CS_IDLE_VAL	BIT(9)
#define APPLE_SPI_PINCFG_MOSI_IDLE_VAL	BIT(10)

#define APPLE_SPI_DELAY_PRE		0x160
#define APPLE_SPI_DELAY_POST		0x168
#define APPLE_SPI_DELAY_ENABLE		BIT(0)
#define APPLE_SPI_DELAY_NO_INTERBYTE	BIT(1)
#define APPLE_SPI_DELAY_SET_SCK		BIT(4)
#define APPLE_SPI_DELAY_SET_MOSI	BIT(6)
#define APPLE_SPI_DELAY_SCK_VAL		BIT(8)
#define APPLE_SPI_DELAY_MOSI_VAL	BIT(12)

#define APPLE_SPI_FIFO_DEPTH		16

#define APPLE_SPI_TIMEOUT_MS		200

struct apple_spi_priv {
	void *base;
	u32 clkfreq;		/* Input clock frequency */
};

static void apple_spi_set_cs(struct apple_spi_priv *priv, int on)
{
	writel(on ? 0 : APPLE_SPI_PIN_CS, priv->base + APPLE_SPI_PIN);
}

/* Fill Tx FIFO. */
static void apple_spi_tx(struct apple_spi_priv *priv, uint *len,
			 const void **dout)
{
	const u8 *out = *dout;
	u32 data, fifostat;
	uint count;

	fifostat = readl(priv->base + APPLE_SPI_FIFOSTAT);
	count = APPLE_SPI_FIFO_DEPTH -
		FIELD_GET(APPLE_SPI_FIFOSTAT_LEVEL_TX, fifostat);
	while (*len > 0 && count > 0) {
		data = out ? *out++ : 0;
		writel(data, priv->base + APPLE_SPI_TXDATA);
		(*len)--;
		count--;
	}

	*dout = out;
}

/* Empty Rx FIFO. */
static void apple_spi_rx(struct apple_spi_priv *priv, uint *len,
			 void **din)
{
	u8 *in = *din;
	u32 data, fifostat;
	uint count;

	fifostat = readl(priv->base + APPLE_SPI_FIFOSTAT);
	count = FIELD_GET(APPLE_SPI_FIFOSTAT_LEVEL_RX, fifostat);
	while (*len > 0 && count > 0) {
		data = readl(priv->base + APPLE_SPI_RXDATA);
		if (in)
			*in++ = data;
		(*len)--;
		count--;
	}

	*din = in;
}

static int apple_spi_xfer(struct udevice *dev, unsigned int bitlen,
			  const void *dout, void *din, unsigned long flags)
{
	struct apple_spi_priv *priv = dev_get_priv(dev->parent);
	unsigned long start = get_timer(0);
	uint txlen, rxlen;
	int ret = 0;

	if ((bitlen % 8) != 0)
		return -EINVAL;
	txlen = rxlen = bitlen / 8;

	if (flags & SPI_XFER_BEGIN)
		apple_spi_set_cs(priv, 1);

	if (txlen > 0) {
		/* Reset FIFOs */
		writel(APPLE_SPI_CTRL_RX_RESET | APPLE_SPI_CTRL_TX_RESET,
		       priv->base + APPLE_SPI_CTRL);

		/* Set the transfer length */
		writel(txlen, priv->base + APPLE_SPI_TXCNT);
		writel(rxlen, priv->base + APPLE_SPI_RXCNT);

		/* Prime transmit FIFO */
		apple_spi_tx(priv, &txlen, &dout);

		/* Start transfer */
		writel(APPLE_SPI_CTRL_RUN, priv->base + APPLE_SPI_CTRL);

		while ((txlen > 0 || rxlen > 0)) {
			apple_spi_rx(priv, &rxlen, &din);
			apple_spi_tx(priv, &txlen, &dout);

			if (get_timer(start) > APPLE_SPI_TIMEOUT_MS) {
				ret = -ETIMEDOUT;
				break;
			}
		}

		/* Stop transfer. */
		writel(0, priv->base + APPLE_SPI_CTRL);
	}

	if (flags & SPI_XFER_END)
		apple_spi_set_cs(priv, 0);

	return ret;
}

static int apple_spi_set_speed(struct udevice *dev, uint speed)
{
	struct apple_spi_priv *priv = dev_get_priv(dev);
	u32 div;

	div = DIV_ROUND_UP(priv->clkfreq, speed);
	if (div < APPLE_SPI_CLKDIV_MIN)
		div = APPLE_SPI_CLKDIV_MIN;
	if (div > APPLE_SPI_CLKDIV_MAX)
		div = APPLE_SPI_CLKDIV_MAX;

	writel(div, priv->base + APPLE_SPI_CLKDIV);

	return 0;
}

static int apple_spi_set_mode(struct udevice *bus, uint mode)
{
	return 0;
}

struct dm_spi_ops apple_spi_ops = {
	.xfer = apple_spi_xfer,
	.set_speed = apple_spi_set_speed,
	.set_mode = apple_spi_set_mode,
};

static int apple_spi_probe(struct udevice *dev)
{
	struct apple_spi_priv *priv = dev_get_priv(dev);
	struct clk clkdev;
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clkdev);
	if (ret)
		return ret;
	priv->clkfreq = clk_get_rate(&clkdev);

	/* Set CS high (inactive) and disable override and auto-CS */
	writel(APPLE_SPI_PIN_CS, priv->base + APPLE_SPI_PIN);
	writel(readl(priv->base + APPLE_SPI_SHIFTCFG) & ~APPLE_SPI_SHIFTCFG_OVERRIDE_CS,
	       priv->base + APPLE_SPI_SHIFTCFG);
	writel((readl(priv->base + APPLE_SPI_PINCFG) & ~APPLE_SPI_PINCFG_CS_IDLE_VAL) |
	       APPLE_SPI_PINCFG_KEEP_CS, priv->base + APPLE_SPI_PINCFG);

	/* Reset FIFOs */
	writel(APPLE_SPI_CTRL_RX_RESET | APPLE_SPI_CTRL_TX_RESET,
	       priv->base + APPLE_SPI_CTRL);

	/* Configure defaults */
	writel(FIELD_PREP(APPLE_SPI_CFG_MODE, APPLE_SPI_CFG_MODE_IRQ) |
	       FIELD_PREP(APPLE_SPI_CFG_WORD_SIZE, APPLE_SPI_CFG_WORD_SIZE_8B) |
	       FIELD_PREP(APPLE_SPI_CFG_FIFO_THRESH, APPLE_SPI_CFG_FIFO_THRESH_8B),
	       priv->base + APPLE_SPI_CFG);

	return 0;
}

static const struct udevice_id apple_spi_of_match[] = {
	{ .compatible = "apple,spi" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(apple_spi) = {
	.name = "apple_spi",
	.id = UCLASS_SPI,
	.of_match = apple_spi_of_match,
	.probe = apple_spi_probe,
	.priv_auto = sizeof(struct apple_spi_priv),
	.ops = &apple_spi_ops,
};
