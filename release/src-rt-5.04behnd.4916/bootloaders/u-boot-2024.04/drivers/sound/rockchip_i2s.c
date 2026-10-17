// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google LLC
 * Copyright 2014 Rockchip Electronics Co., Ltd.
 * Taken from dc i2s/rockchip.c
 */

#define LOG_CATEGORY UCLASS_I2S

#include <common.h>
#include <dm.h>
#include <i2s.h>
#include <log.h>
#include <sound.h>
#include <asm/io.h>
#include <linux/bitops.h>

struct rk_i2s_regs {
	u32 txcr;		/* I2S_TXCR, 0x00 */
	u32 rxcr;		/* I2S_RXCR, 0x04 */
	u32 ckr;		/* I2S_CKR, 0x08 */
	u32 fifolr;		/* I2S_FIFOLR, 0x0C */
	u32 dmacr;		/* I2S_DMACR, 0x10 */
	u32 intcr;		/* I2S_INTCR, 0x14 */
	u32 intsr;		/* I2S_INTSR, 0x18 */
	u32 xfer;		/* I2S_XFER, 0x1C */
	u32 clr;		/* I2S_CLR, 0x20 */
	u32 txdr;		/* I2S_TXDR, 0x24 */
	u32 rxdr;		/* I2S_RXDR, 0x28 */
};

enum {
	/* I2S_XFER */
	I2S_RX_TRAN_BIT		= BIT(1),
	I2S_TX_TRAN_BIT		= BIT(0),
	I2S_TRAN_MASK		= 3 << 0,

	/* I2S_TXCKR */
	I2S_MCLK_DIV_SHIFT	= 16,
	I2S_MCLK_DIV_MASK	= (0xff << I2S_MCLK_DIV_SHIFT),

	I2S_RX_SCLK_DIV_SHIFT	= 8,
	I2S_RX_SCLK_DIV_MASK	= 0xff << I2S_RX_SCLK_DIV_SHIFT,
	I2S_TX_SCLK_DIV_SHIFT	= 0,
	I2S_TX_SCLK_DIV_MASK	= 0xff << I2S_TX_SCLK_DIV_SHIFT,

	I2S_DATA_WIDTH_SHIFT	= 0,
	I2S_DATA_WIDTH_MASK	= 0x1f << I2S_DATA_WIDTH_SHIFT,
};

static int rockchip_i2s_init(struct i2s_uc_priv *priv)
{
	struct rk_i2s_regs *regs = (struct rk_i2s_regs *)priv->base_address;
	u32 bps = priv->bitspersample;
	u32 lrf = priv->rfs;
	u32 chn = priv->channels;
	u32 mode = 0;

	clrbits_le32(&regs->xfer, I2S_TX_TRAN_BIT);
	mode = readl(&regs->txcr) & ~0x1f;
	switch (priv->bitspersample) {
	case 16:
	case 24:
		mode |= (priv->bitspersample - 1) << I2S_DATA_WIDTH_SHIFT;
		break;
	default:
		log_err("Invalid sample size input %d\n", priv->bitspersample);
		return -EINVAL;
	}
	writel(mode, &regs->txcr);

	mode = readl(&regs->ckr) & ~I2S_MCLK_DIV_MASK;
	mode |= (lrf / (bps * chn) - 1) << I2S_MCLK_DIV_SHIFT;

	mode &= ~I2S_TX_SCLK_DIV_MASK;
	mode |= (priv->bitspersample * priv->channels - 1) <<
			 I2S_TX_SCLK_DIV_SHIFT;
	writel(mode, &regs->ckr);

	return 0;
}

static int i2s_send_data(struct rk_i2s_regs *regs, u32 *data, uint length)
{
	for (int i = 0; i < min(32u, length); i++)
		writel(*data++, &regs->txdr);

	length -= min(32u, length);

	/* enable both tx and rx */
	setbits_le32(&regs->xfer, I2S_TRAN_MASK);
	while (length) {
		if ((readl(&regs->fifolr) & 0x3f) < 0x20) {
			writel(*data++, &regs->txdr);
			length--;
		}
	}
	while (readl(&regs->fifolr) & 0x3f)
		/* wait until FIFO empty */;
	clrbits_le32(&regs->xfer, I2S_TRAN_MASK);
	writel(0, &regs->clr);

	return 0;
}

static int rockchip_i2s_tx_data(struct udevice *dev, void *data, uint data_size)
{
	struct i2s_uc_priv *priv = dev_get_uclass_priv(dev);
	struct rk_i2s_regs *regs = (struct rk_i2s_regs *)priv->base_address;

	return i2s_send_data(regs, data, data_size / sizeof(u32));
}

static int rockchip_i2s_probe(struct udevice *dev)
{
	struct i2s_uc_priv *priv = dev_get_uclass_priv(dev);
	ulong base;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE) {
		log_debug("Missing i2s base\n");
		return -EINVAL;
	}
	priv->base_address = base;
	priv->id = 1;
	priv->audio_pll_clk = 4800000;
	priv->samplingrate = 48000;
	priv->bitspersample = 16;
	priv->channels = 2;
	priv->rfs = 256;
	priv->bfs = 32;

	return rockchip_i2s_init(priv);
}

static const struct i2s_ops rockchip_i2s_ops = {
	.tx_data	= rockchip_i2s_tx_data,
};

static const struct udevice_id rockchip_i2s_ids[] = {
	{ .compatible = "rockchip,rk3288-i2s" },
	{ }
};

U_BOOT_DRIVER(rockchip_i2s) = {
	.name		= "rockchip_i2s",
	.id		= UCLASS_I2S,
	.of_match	= rockchip_i2s_ids,
	.probe		= rockchip_i2s_probe,
	.ops		= &rockchip_i2s_ops,
};
