// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */
#define LOG_CATEGORY UCLASS_I2S

#include <common.h>
#include <dm.h>
#include <i2s.h>
#include <log.h>
#include <misc.h>
#include <sound.h>
#include <asm/io.h>
#include <asm/arch-tegra/tegra_i2s.h>
#include "tegra_i2s_priv.h"

int tegra_i2s_set_cif_tx_ctrl(struct udevice *dev, u32 value)
{
	struct i2s_uc_priv *priv = dev_get_uclass_priv(dev);
	struct i2s_ctlr *regs = (struct i2s_ctlr *)priv->base_address;

	writel(value, &regs->cif_tx_ctrl);

	return 0;
}

static void tegra_i2s_transmit_enable(struct i2s_ctlr *regs, int on)
{
	clrsetbits_le32(&regs->ctrl, I2S_CTRL_XFER_EN_TX,
			on ? I2S_CTRL_XFER_EN_TX : 0);
}

static int i2s_tx_init(struct i2s_uc_priv *pi2s_tx)
{
	struct i2s_ctlr *regs = (struct i2s_ctlr *)pi2s_tx->base_address;
	u32 audio_bits = (pi2s_tx->bitspersample >> 2) - 1;
	u32 ctrl = readl(&regs->ctrl);

	/* Set format to LRCK / Left Low */
	ctrl &= ~(I2S_CTRL_FRAME_FORMAT_MASK | I2S_CTRL_LRCK_MASK);
	ctrl |= I2S_CTRL_FRAME_FORMAT_LRCK;
	ctrl |= I2S_CTRL_LRCK_L_LOW;

	/* Disable all transmission until we are ready to transfer */
	ctrl &= ~(I2S_CTRL_XFER_EN_TX | I2S_CTRL_XFER_EN_RX);

	/* Serve as master */
	ctrl |= I2S_CTRL_MASTER_ENABLE;

	/* Configure audio bits size */
	ctrl &= ~I2S_CTRL_BIT_SIZE_MASK;
	ctrl |= audio_bits << I2S_CTRL_BIT_SIZE_SHIFT;
	writel(ctrl, &regs->ctrl);

	/* Timing in LRCK mode: */
	writel(pi2s_tx->bitspersample, &regs->timing);

	/* I2S mode has [TX/RX]_DATA_OFFSET both set to 1 */
	writel(((1 << I2S_OFFSET_RX_DATA_OFFSET_SHIFT) |
		(1 << I2S_OFFSET_TX_DATA_OFFSET_SHIFT)), &regs->offset);

	/* FSYNC_WIDTH = 2 clocks wide, TOTAL_SLOTS = 2 slots per fsync */
	writel((2 - 1) << I2S_CH_CTRL_FSYNC_WIDTH_SHIFT, &regs->ch_ctrl);

	return 0;
}

static int tegra_i2s_tx_data(struct udevice *dev, void *data, uint data_size)
{
	struct i2s_uc_priv *priv = dev_get_uclass_priv(dev);
	struct i2s_ctlr *regs = (struct i2s_ctlr *)priv->base_address;
	int ret;

	tegra_i2s_transmit_enable(regs, 1);
	ret = misc_write(dev_get_parent(dev), 0, data, data_size);
	tegra_i2s_transmit_enable(regs, 0);
	if (ret < 0)
		return ret;
	else if (ret < data_size)
		return -EIO;

	return 0;
}

static int tegra_i2s_probe(struct udevice *dev)
{
	struct i2s_uc_priv *priv = dev_get_uclass_priv(dev);
	ulong base;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE) {
		debug("%s: Missing i2s base\n", __func__);
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

	return i2s_tx_init(priv);
}

static const struct i2s_ops tegra_i2s_ops = {
	.tx_data	= tegra_i2s_tx_data,
};

static const struct udevice_id tegra_i2s_ids[] = {
	{ .compatible = "nvidia,tegra124-i2s" },
	{ }
};

U_BOOT_DRIVER(tegra_i2s) = {
	.name		= "tegra_i2s",
	.id		= UCLASS_I2S,
	.of_match	= tegra_i2s_ids,
	.probe		= tegra_i2s_probe,
	.ops		= &tegra_i2s_ops,
};
