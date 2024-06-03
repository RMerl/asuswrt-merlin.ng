// SPDX-License-Identifier: GPL-2.0+159
/*
 * Take from dc tegra_ahub.c
 *
 * Copyright 2018 Google LLC
 */

#define LOG_CATEGORY UCLASS_MISC

#include <common.h>
#include <dm.h>
#include <i2s.h>
#include <misc.h>
#include <asm/io.h>
#include <asm/arch-tegra/tegra_ahub.h>
#include <asm/arch-tegra/tegra_i2s.h>
#include "tegra_i2s_priv.h"

struct tegra_ahub_priv {
	struct apbif_regs *apbif_regs;
	struct xbar_regs *xbar_regs;
	u32 full_mask;
	int capacity_words;  /* FIFO capacity in words */

	/*
	 * This is unset intially, but is set by tegra_ahub_ioctl() called
	 * from the misc_ioctl() in tegra_sound_probe()
	 */
	struct udevice *i2s;
	struct udevice *dma;
};

static int tegra_ahub_xbar_enable_i2s(struct xbar_regs *regs, int i2s_id)
{
	/*
	 * Enables I2S as the receiver of APBIF by writing APBIF_TX0 (0x01) to
	 * the rx0 register
	 */
	switch (i2s_id) {
	case 0:
		writel(1, &regs->i2s0_rx0);
		break;
	case 1:
		writel(1, &regs->i2s1_rx0);
		break;
	case 2:
		writel(1, &regs->i2s2_rx0);
		break;
	case 3:
		writel(1, &regs->i2s3_rx0);
		break;
	case 4:
		writel(1, &regs->i2s4_rx0);
		break;
	default:
		log_err("Invalid I2S component id: %d\n", i2s_id);
		return -EINVAL;
	}
	return 0;
}

static int tegra_ahub_apbif_is_full(struct udevice *dev)
{
	struct tegra_ahub_priv *priv = dev_get_priv(dev);

	return readl(&priv->apbif_regs->apbdma_live_stat) & priv->full_mask;
}

/**
 * tegra_ahub_wait_for_space() - Wait for space in the FIFO
 *
 * @return 0 if OK, -ETIMEDOUT if no space was available in time
 */
static int tegra_ahub_wait_for_space(struct udevice *dev)
{
	int i = 100000;
	ulong start;

	/* Busy-wait initially, since this should take almost no time */
	while (i--) {
		if (!tegra_ahub_apbif_is_full(dev))
			return 0;
	}

	/* Failed, so do a slower loop for 100ms */
	start = get_timer(0);
	while (tegra_ahub_apbif_is_full(dev)) {
		if (get_timer(start) > 100)
			return -ETIMEDOUT;
	}

	return 0;
}

static int tegra_ahub_apbif_send(struct udevice *dev, int offset,
				 const void *buf, int len)
{
	struct tegra_ahub_priv *priv = dev_get_priv(dev);
	const u32 *data = (const u32 *)buf;
	ssize_t written = 0;

	if (len % sizeof(*data)) {
		log_err("Data size (%zd) must be aligned to %zd.\n", len,
			sizeof(*data));
		return -EFAULT;
	}
	while (written < len) {
		int ret = tegra_ahub_wait_for_space(dev);

		if (ret)
			return ret;

		writel(*data++, &priv->apbif_regs->channel0_txfifo);
		written += sizeof(*data);
	}

	return written;
}

static void tegra_ahub_apbif_set_cif(struct udevice *dev, u32 value)
{
	struct tegra_ahub_priv *priv = dev_get_priv(dev);

	writel(value, &priv->apbif_regs->channel0_cif_tx0_ctrl);
}

static void tegra_ahub_apbif_enable_channel0(struct udevice *dev,
					     int fifo_threshold)
{
	struct tegra_ahub_priv *priv = dev_get_priv(dev);

	u32 ctrl = TEGRA_AHUB_CHANNEL_CTRL_TX_PACK_EN |
			TEGRA_AHUB_CHANNEL_CTRL_TX_PACK_16 |
			TEGRA_AHUB_CHANNEL_CTRL_TX_EN;

	fifo_threshold--; /* fifo_threshold starts from 1 */
	ctrl |= (fifo_threshold << TEGRA_AHUB_CHANNEL_CTRL_TX_THRESHOLD_SHIFT);
	writel(ctrl, &priv->apbif_regs->channel0_ctrl);
}

static u32 tegra_ahub_get_cif(bool is_receive, uint channels,
			      uint bits_per_sample, uint fifo_threshold)
{
	uint audio_bits = (bits_per_sample >> 2) - 1;
	u32 val;

	channels--;  /* Channels in CIF starts from 1 */
	fifo_threshold--;  /* FIFO threshold starts from 1 */
	/* Assume input and output are always using same channel / bits */
	val = channels << TEGRA_AUDIOCIF_CTRL_AUDIO_CHANNELS_SHIFT |
	      channels << TEGRA_AUDIOCIF_CTRL_CLIENT_CHANNELS_SHIFT |
	      audio_bits << TEGRA_AUDIOCIF_CTRL_AUDIO_BITS_SHIFT |
	      audio_bits << TEGRA_AUDIOCIF_CTRL_CLIENT_BITS_SHIFT |
	      fifo_threshold << TEGRA_AUDIOCIF_CTRL_FIFO_THRESHOLD_SHIFT |
	      (is_receive ? TEGRA_AUDIOCIF_DIRECTION_RX <<
			    TEGRA_AUDIOCIF_CTRL_DIRECTION_SHIFT : 0);

	return val;
}

static int tegra_ahub_enable(struct udevice *dev)
{
	struct tegra_ahub_priv *priv = dev_get_priv(dev);
	struct i2s_uc_priv *uc_priv = dev_get_uclass_priv(priv->i2s);
	u32 cif_ctrl = 0;
	int ret;

	/* We use APBIF channel0 as a sender */
	priv->full_mask = TEGRA_AHUB_APBDMA_LIVE_STATUS_CH0_TX_CIF_FIFO_FULL;
	priv->capacity_words = 8;

	/*
	 * FIFO is inactive until (fifo_threshold) of words are sent. For
	 * better performance, we want to set it to half of capacity.
	 */
	u32 fifo_threshold = priv->capacity_words / 2;

	/*
	 * Setup audio client interface (ACIF): APBIF (channel0) as sender and
	 * I2S as receiver
	 */
	cif_ctrl = tegra_ahub_get_cif(true, uc_priv->channels,
				      uc_priv->bitspersample, fifo_threshold);
	tegra_i2s_set_cif_tx_ctrl(priv->i2s, cif_ctrl);

	cif_ctrl = tegra_ahub_get_cif(false, uc_priv->channels,
				      uc_priv->bitspersample, fifo_threshold);
	tegra_ahub_apbif_set_cif(dev, cif_ctrl);
	tegra_ahub_apbif_enable_channel0(dev, fifo_threshold);

	ret = tegra_ahub_xbar_enable_i2s(priv->xbar_regs, uc_priv->id);
	if (ret)
		return ret;
	log_debug("ahub: channels=%d, bitspersample=%d, cif_ctrl=%x, fifo_threshold=%d, id=%d\n",
		  uc_priv->channels, uc_priv->bitspersample, cif_ctrl,
		  fifo_threshold, uc_priv->id);

	return 0;
}

static int tegra_ahub_ioctl(struct udevice *dev, unsigned long request,
			    void *buf)
{
	struct tegra_ahub_priv *priv = dev_get_priv(dev);

	if (request != AHUB_MISCOP_SET_I2S)
		return -ENOSYS;

	priv->i2s = *(struct udevice **)buf;
	log_debug("i2s set to '%s'\n", priv->i2s->name);

	return tegra_ahub_enable(dev);
}

static int tegra_ahub_probe(struct udevice *dev)
{
	struct tegra_ahub_priv *priv = dev_get_priv(dev);
	ulong addr;

	addr = dev_read_addr_index(dev, 0);
	if (addr == FDT_ADDR_T_NONE) {
		log_debug("Invalid apbif address\n");
		return -EINVAL;
	}
	priv->apbif_regs = (struct apbif_regs *)addr;

	addr = dev_read_addr_index(dev, 1);
	if (addr == FDT_ADDR_T_NONE) {
		log_debug("Invalid xbar address\n");
		return -EINVAL;
	}
	priv->xbar_regs = (struct xbar_regs *)addr;
	log_debug("ahub apbif_regs=%p, xbar_regs=%p\n", priv->apbif_regs,
		  priv->xbar_regs);

	return 0;
}

static struct misc_ops tegra_ahub_ops = {
	.write		= tegra_ahub_apbif_send,
	.ioctl		= tegra_ahub_ioctl,
};

static const struct udevice_id tegra_ahub_ids[] = {
	{ .compatible = "nvidia,tegra124-ahub" },
	{ }
};

U_BOOT_DRIVER(tegra_ahub) = {
	.name		= "tegra_ahub",
	.id		= UCLASS_MISC,
	.of_match	= tegra_ahub_ids,
	.ops		= &tegra_ahub_ops,
	.probe		= tegra_ahub_probe,
	.priv_auto_alloc_size	= sizeof(struct tegra_ahub_priv),
};
