// SPDX-License-Identifier: GPL-2.0+
/*
 * Intel Broadwell I2S driver
 *
 * Copyright 2019 Google LLC
 *
 * Modified from dc i2s/broadwell/broadwell.c
 */

#define LOG_CATEGORY UCLASS_I2S

#include <common.h>
#include <dm.h>
#include <i2s.h>
#include <asm/io.h>
#include "broadwell_i2s.h"

enum {
	BDW_SHIM_START_ADDRESS = 0xfb000,
	BDW_SSP0_START_ADDRESS = 0xfc000,
	BDW_SSP1_START_ADDRESS = 0xfd000,
};

struct broadwell_i2s_priv {
	enum frame_sync_rel_timing_t rel_timing;
	enum frame_sync_pol_t sfrm_polarity;
	enum end_transfer_state_t end_transfer_state;
	enum clock_mode_t sclk_mode;
	uint sclk_dummy_stop;	/* 0-31 */
	uint sclk_frame_width;	/* 1-38 */
	struct i2s_shim_regs *shim;
	struct broadwell_i2s_regs *regs;
};

static void init_shim_csr(struct broadwell_i2s_priv *priv)
{
	/*
	 * Select SSP clock
	 * Turn off low power clock
	 * Set PIO mode
	 * Stall DSP core
	 */
	clrsetbits_le32(&priv->shim->csr,
			SHIM_CS_S0IOCS | SHIM_CS_LPCS | SHIM_CS_DCS_MASK,
			SHIM_CS_S1IOCS | SHIM_CS_SBCS_SSP1_24MHZ |
			SHIM_CS_SBCS_SSP0_24MHZ | SHIM_CS_SDPM_PIO_SSP1 |
			SHIM_CS_SDPM_PIO_SSP0 | SHIM_CS_STALL |
			SHIM_CS_DCS_DSP32_AF32);
}

static void init_shim_clkctl(struct i2s_uc_priv *uc_priv,
			     struct broadwell_i2s_priv *priv)
{
	u32 clkctl = readl(&priv->shim->clkctl);

	/* Set 24Mhz mclk, prevent local clock gating, enable SSP0 clock */
	clkctl &= SHIM_CLKCTL_RESERVED;
	clkctl |= SHIM_CLKCTL_MCLK_24MHZ | SHIM_CLKCTL_DCPLCG;

	/* Enable requested SSP interface */
	if (uc_priv->id)
		clkctl |= SHIM_CLKCTL_SCOE_SSP1 | SHIM_CLKCTL_SFLCGB_SSP1_CGD;
	else
		clkctl |= SHIM_CLKCTL_SCOE_SSP0 | SHIM_CLKCTL_SFLCGB_SSP0_CGD;

	writel(clkctl, &priv->shim->clkctl);
}

static void init_sscr0(struct i2s_uc_priv *uc_priv,
		       struct broadwell_i2s_priv *priv)
{
	u32 sscr0;
	uint scale;

	/* Set data size based on BPS */
	if (uc_priv->bitspersample > 16)
		sscr0 = (uc_priv->bitspersample - 16 - 1) << SSP_SSC0_DSS_SHIFT
			 | SSP_SSC0_EDSS;
	else
		sscr0 = (uc_priv->bitspersample - 1) << SSP_SSC0_DSS_SHIFT;

	/* Set network mode, Stereo PSP frame format */
	sscr0 |= SSP_SSC0_MODE_NETWORK |
		SSP_SSC0_FRDC_STEREO |
		SSP_SSC0_FRF_PSP |
		SSP_SSC0_TIM |
		SSP_SSC0_RIM |
		SSP_SSC0_ECS_PCH |
		SSP_SSC0_NCS_PCH |
		SSP_SSC0_ACS_PCH;

	/* Scale 24MHz MCLK */
	scale = uc_priv->audio_pll_clk / uc_priv->samplingrate / uc_priv->bfs;
	sscr0 |= scale << SSP_SSC0_SCR_SHIFT;

	writel(sscr0, &priv->regs->sscr0);
}

static void init_sscr1(struct broadwell_i2s_priv *priv)
{
	u32 sscr1 = readl(&priv->regs->sscr1);

	sscr1 &= SSP_SSC1_RESERVED;

	/* Set as I2S master */
	sscr1 |= SSP_SSC1_SCLKDIR_MASTER | SSP_SSC1_SCLKDIR_MASTER;

	/* Enable TXD tristate behavior for PCH */
	sscr1 |= SSP_SSC1_TTELP | SSP_SSC1_TTE;

	/* Disable DMA Tx/Rx service request */
	sscr1 |= SSP_SSC1_TSRE | SSP_SSC1_RSRE;

	/* Clock on during transfer */
	sscr1 |= SSP_SSC1_SCFR;

	/* Set FIFO thresholds */
	sscr1 |= SSP_FIFO_SIZE << SSP_SSC1_RFT_SHIFT;
	sscr1 |= SSP_FIFO_SIZE << SSP_SSC1_TFT_SHIFT;

	/* Disable interrupts */
	sscr1 &= ~(SSP_SSC1_EBCEI | SSP_SSC1_TINTE | SSP_SSC1_PINTE);
	sscr1 &= ~(SSP_SSC1_LBM | SSP_SSC1_RWOT);

	writel(sscr1, &priv->regs->sscr1);
}

static void init_sspsp(struct broadwell_i2s_priv *priv)
{
	u32 sspsp = readl(&priv->regs->sspsp);

	sspsp &= SSP_PSP_RESERVED;
	sspsp |= priv->sclk_mode << SSP_PSP_SCMODE_SHIFT;
	sspsp |= (priv->sclk_dummy_stop << SSP_PSP_DMYSTOP_SHIFT) &
			SSP_PSP_DMYSTOP_MASK;
	sspsp |= (priv->sclk_dummy_stop >> 2 << SSP_PSP_EDYMSTOP_SHIFT) &
			SSP_PSP_EDMYSTOP_MASK;
	sspsp |= priv->sclk_frame_width << SSP_PSP_SFRMWDTH_SHIFT;

	/* Frame Sync Relative Timing */
	if (priv->rel_timing == NEXT_FRMS_AFTER_END_OF_T4)
		sspsp |= SSP_PSP_FSRT;
	else
		sspsp &= ~SSP_PSP_FSRT;

	/* Serial Frame Polarity */
	if (priv->sfrm_polarity == SSP_FRMS_ACTIVE_HIGH)
		sspsp |= SSP_PSP_SFRMP;
	else
		sspsp &= ~SSP_PSP_SFRMP;

	/* End Data Transfer State */
	if (priv->end_transfer_state == SSP_END_TRANSFER_STATE_LOW)
		sspsp &= ~SSP_PSP_ETDS;
	else
		sspsp |= SSP_PSP_ETDS;

	writel(sspsp, &priv->regs->sspsp);
}

static void init_ssp_time_slot(struct broadwell_i2s_priv *priv)
{
	writel(3, &priv->regs->sstsa);
	writel(3, &priv->regs->ssrsa);
}

static int bdw_i2s_init(struct udevice *dev)
{
	struct i2s_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	struct broadwell_i2s_priv *priv = dev_get_priv(dev);

	init_shim_csr(priv);
	init_shim_clkctl(uc_priv, priv);
	init_sscr0(uc_priv, priv);
	init_sscr1(priv);
	init_sspsp(priv);
	init_ssp_time_slot(priv);

	return 0;
}

static void bdw_i2s_enable(struct broadwell_i2s_priv *priv)
{
	setbits_le32(&priv->regs->sscr0, SSP_SSC0_SSE);
	setbits_le32(&priv->regs->sstsa, SSP_SSTSA_EN);
}

static void bdw_i2s_disable(struct broadwell_i2s_priv *priv)
{
	clrbits_le32(&priv->regs->sstsa, SSP_SSTSA_EN);
	clrbits_le32(&priv->regs->sstsa, SSP_SSTSA_EN);
}

static int broadwell_i2s_tx_data(struct udevice *dev, void *data,
				 uint data_size)
{
	struct broadwell_i2s_priv *priv = dev_get_priv(dev);
	u32 *ptr = data;

	log_debug("data=%p, data_size=%x\n", data, data_size);
	if (data_size < SSP_FIFO_SIZE) {
		log_err("Invalid I2S data size\n");
		return -ENODATA;
	}

	/* Enable I2S interface */
	bdw_i2s_enable(priv);

	/* Transfer data */
	while (data_size > 0) {
		ulong start = timer_get_us() + 100000;

		/* Write data if transmit FIFO has room */
		if (readl(&priv->regs->sssr) & SSP_SSS_TNF) {
			writel(*ptr++, &priv->regs->ssdr);
			data_size -= sizeof(*ptr);
		} else {
			if ((long)(timer_get_us() - start) > 0) {
				/* Disable I2S interface */
				bdw_i2s_disable(priv);
				log_debug("I2S Transfer Timeout\n");
				return -ETIMEDOUT;
			}
		}
	}

	/* Disable I2S interface */
	bdw_i2s_disable(priv);
	log_debug("done\n");

	return 0;
}

static int broadwell_i2s_probe(struct udevice *dev)
{
	struct i2s_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	struct broadwell_i2s_priv *priv = dev_get_priv(dev);
	struct udevice *adsp = dev_get_parent(dev);
	u32 bar0, offset;
	int ret;

	bar0 = dm_pci_read_bar32(adsp, 0);
	if (!bar0) {
		log_debug("Cannot read adsp bar0\n");
		return -EINVAL;
	}
	offset = dev_read_addr_index(dev, 0);
	if (offset == FDT_ADDR_T_NONE) {
		log_debug("Cannot read address index 0\n");
		return -EINVAL;
	}
	uc_priv->base_address = bar0 + offset;

	/*
	 * Hard-code these values. If other settings are required we can add
	 * this to the device tree.
	 */
	uc_priv->rfs = 64;
	uc_priv->bfs = 32;
	uc_priv->audio_pll_clk = 24 * 1000 * 1000;
	uc_priv->samplingrate = 48000;
	uc_priv->bitspersample = 16;
	uc_priv->channels = 2;
	uc_priv->id = 0;

	priv->shim = (struct i2s_shim_regs *)uc_priv->base_address;
	priv->sfrm_polarity = SSP_FRMS_ACTIVE_LOW;
	priv->end_transfer_state = SSP_END_TRANSFER_STATE_LOW;
	priv->sclk_mode = SCLK_MODE_DDF_DSR_ISL;
	priv->rel_timing = NEXT_FRMS_WITH_LSB_PREVIOUS_FRM;
	priv->sclk_dummy_stop = 0;
	priv->sclk_frame_width = 31;

	offset = dev_read_addr_index(dev, 1 + uc_priv->id);
	if (offset == FDT_ADDR_T_NONE) {
		log_debug("Cannot read address index %d\n", 1 + uc_priv->id);
		return -EINVAL;
	}
	log_debug("bar0=%x, uc_priv->base_address=%x, offset=%x\n", bar0,
		  uc_priv->base_address, offset);
	priv->regs = (struct broadwell_i2s_regs *)(bar0 + offset);

	ret = bdw_i2s_init(dev);
	if (ret)
		return ret;

	return 0;
}

static const struct i2s_ops broadwell_i2s_ops = {
	.tx_data	= broadwell_i2s_tx_data,
};

static const struct udevice_id broadwell_i2s_ids[] = {
	{ .compatible = "intel,broadwell-i2s" },
	{ }
};

U_BOOT_DRIVER(broadwell_i2s) = {
	.name		= "broadwell_i2s",
	.id		= UCLASS_I2S,
	.of_match	= broadwell_i2s_ids,
	.probe		= broadwell_i2s_probe,
	.ops		= &broadwell_i2s_ops,
	.priv_auto_alloc_size	= sizeof(struct broadwell_i2s_priv),
};
