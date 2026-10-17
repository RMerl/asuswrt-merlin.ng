// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Amit Singh Tomar <amittomer25@gmail.com>
 *
 * Driver for SD/MMC controller present on Actions Semi S700/S900 SoC, based
 * on Linux Driver "drivers/mmc/host/owl-mmc.c".
 *
 * Though, there is a bit (BSEL, BUS or DMA Special Channel Selection) that
 * controls the data transfer from SDx_DAT register either using CPU AHB Bus
 * or DMA channel, but seems like, it only works correctly using external DMA
 * channel, and those special bits used in this driver is picked from vendor
 * source exclusively for MMC/SD.
 */
#include <common.h>
#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <mmc.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/iopoll.h>

/*
 * SDC registers
 */
#define OWL_REG_SD_EN			0x0000
#define OWL_REG_SD_CTL			0x0004
#define OWL_REG_SD_STATE		0x0008
#define OWL_REG_SD_CMD			0x000c
#define OWL_REG_SD_ARG			0x0010
#define OWL_REG_SD_RSPBUF0		0x0014
#define OWL_REG_SD_RSPBUF1		0x0018
#define OWL_REG_SD_RSPBUF2		0x001c
#define OWL_REG_SD_RSPBUF3		0x0020
#define OWL_REG_SD_RSPBUF4		0x0024
#define OWL_REG_SD_DAT			0x0028
#define OWL_REG_SD_BLK_SIZE		0x002c
#define OWL_REG_SD_BLK_NUM		0x0030
#define OWL_REG_SD_BUF_SIZE		0x0034

/* SD_EN Bits */
#define OWL_SD_EN_RANE			BIT(31)
#define OWL_SD_EN_RESE			BIT(10)
#define OWL_SD_ENABLE			BIT(7)
#define OWL_SD_EN_BSEL			BIT(6)
#define OWL_SD_EN_DATAWID(x)		(((x) & 0x3) << 0)
#define OWL_SD_EN_DATAWID_MASK		0x03

/* SD_CTL Bits */
#define OWL_SD_CTL_TOUTEN		BIT(31)
#define OWL_SD_CTL_DELAY_MSK		GENMASK(23, 16)
#define OWL_SD_CTL_RDELAY(x)		(((x) & 0xf) << 20)
#define OWL_SD_CTL_WDELAY(x)		(((x) & 0xf) << 16)
#define OWL_SD_CTL_TS			BIT(7)
#define OWL_SD_CTL_LBE			BIT(6)
#define OWL_SD_CTL_TM(x)		(((x) & 0xf) << 0)

#define OWL_SD_DELAY_LOW_CLK		0x0f
#define OWL_SD_DELAY_MID_CLK		0x0a
#define OWL_SD_RDELAY_HIGH		0x08
#define OWL_SD_WDELAY_HIGH		0x09

/* SD_STATE Bits */
#define OWL_SD_STATE_DAT0S		BIT(7)
#define OWL_SD_STATE_CLNR		BIT(4)
#define OWL_SD_STATE_CRC7ER		BIT(0)

#define OWL_MMC_OCR			(MMC_VDD_32_33 | MMC_VDD_33_34 | \
					 MMC_VDD_165_195)

#define DATA_TRANSFER_TIMEOUT		3000000
#define DMA_TRANSFER_TIMEOUT		5000000

/*
 * Simple DMA transfer operations defines for MMC/SD card
 */
#define SD_DMA_CHANNEL(base, channel)	((base) + 0x100 + 0x100 * (channel))

#define DMA_MODE			0x0000
#define DMA_SOURCE			0x0004
#define DMA_DESTINATION			0x0008
#define DMA_FRAME_LEN			0x000C
#define DMA_FRAME_CNT			0x0010
#define DMA_START			0x0024

/* DMAx_MODE */
#define DMA_MODE_ST(x)			(((x) & 0x3) << 8)
#define DMA_MODE_ST_DEV			DMA_MODE_ST(0)
#define DMA_MODE_DT(x)			(((x) & 0x3) << 10)
#define DMA_MODE_DT_DCU			DMA_MODE_DT(2)
#define DMA_MODE_SAM(x)			(((x) & 0x3) << 16)
#define DMA_MODE_SAM_CONST		DMA_MODE_SAM(0)
#define DMA_MODE_DAM(x)			(((x) & 0x3) << 18)
#define DMA_MODE_DAM_INC		DMA_MODE_DAM(1)

#define DMA_ENABLE			0x1

struct owl_mmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct owl_mmc_priv {
	void *reg_base;
	void *dma_channel;
	struct clk clk;
	unsigned int clock;	  /* Current clock */
	unsigned int dma_drq;	  /* Trigger Source */
};

static void owl_dma_config(struct owl_mmc_priv *priv, unsigned int src,
			   unsigned int dst, unsigned int len)
{
	unsigned int mode = priv->dma_drq;

	/* Set Source and Destination adderess mode */
	mode |= (DMA_MODE_ST_DEV | DMA_MODE_SAM_CONST | DMA_MODE_DT_DCU |
			DMA_MODE_DAM_INC);

	writel(mode, SD_DMA_CHANNEL(priv->dma_channel, 0) + DMA_MODE);
	writel(src, SD_DMA_CHANNEL(priv->dma_channel, 0)  + DMA_SOURCE);
	writel(dst, SD_DMA_CHANNEL(priv->dma_channel, 0)  + DMA_DESTINATION);
	writel(len, SD_DMA_CHANNEL(priv->dma_channel, 0)  + DMA_FRAME_LEN);
	writel(0x1, SD_DMA_CHANNEL(priv->dma_channel, 0)  + DMA_FRAME_CNT);
}

static void owl_mmc_prepare_data(struct owl_mmc_priv *priv,
				 struct mmc_data *data)
{
	unsigned int total;
	u32 buf = 0;

	setbits_le32(priv->reg_base + OWL_REG_SD_EN, OWL_SD_EN_BSEL);

	writel(data->blocks, priv->reg_base + OWL_REG_SD_BLK_NUM);
	writel(data->blocksize, priv->reg_base + OWL_REG_SD_BLK_SIZE);
	total = data->blocksize * data->blocks;

	if (total < 512)
		writel(total, priv->reg_base + OWL_REG_SD_BUF_SIZE);
	else
		writel(512, priv->reg_base + OWL_REG_SD_BUF_SIZE);

	/* DMA STOP */
	writel(0x0, SD_DMA_CHANNEL(priv->dma_channel, 0) + DMA_START);

	if (data) {
		if (data->flags == MMC_DATA_READ) {
			buf = (ulong) (data->dest);
			owl_dma_config(priv, (ulong) priv->reg_base +
				       OWL_REG_SD_DAT, buf, total);
			invalidate_dcache_range(buf, buf + total);
		} else {
			buf = (ulong) (data->src);
			owl_dma_config(priv, buf, (ulong) priv->reg_base +
				       OWL_REG_SD_DAT, total);
			flush_dcache_range(buf, buf + total);
		}
		/* DMA START */
		writel(0x1, SD_DMA_CHANNEL(priv->dma_channel, 0) + DMA_START);
	}
}

static int owl_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			    struct mmc_data *data)
{
	struct owl_mmc_priv *priv = dev_get_priv(dev);
	unsigned int cmd_rsp_mask, mode, reg;
	int ret;

	setbits_le32(priv->reg_base + OWL_REG_SD_EN, OWL_SD_ENABLE);

	/* setup response */
	mode = 0;
	if (cmd->resp_type != MMC_RSP_NONE)
		cmd_rsp_mask = OWL_SD_STATE_CLNR | OWL_SD_STATE_CRC7ER;
	if (cmd->resp_type == MMC_RSP_R1) {
		if (data) {
			if (data->flags == MMC_DATA_READ)
				mode |= OWL_SD_CTL_TM(4);
			else
				mode |= OWL_SD_CTL_TM(5);
		} else
			mode |= OWL_SD_CTL_TM(1);
	} else if (cmd->resp_type == MMC_RSP_R2) {
			mode = OWL_SD_CTL_TM(2);
	} else if (cmd->resp_type == MMC_RSP_R1b) {
			mode = OWL_SD_CTL_TM(3);
	} else if (cmd->resp_type == MMC_RSP_R3) {
			cmd_rsp_mask = OWL_SD_STATE_CLNR;
			mode = OWL_SD_CTL_TM(1);
	}

	mode |= (readl(priv->reg_base + OWL_REG_SD_CTL) & (0xff << 16));

	/* setup command */
	writel(cmd->cmdidx, priv->reg_base + OWL_REG_SD_CMD);
	writel(cmd->cmdarg, priv->reg_base + OWL_REG_SD_ARG);

	/* Set LBE to send clk at the end of last read block */
	if (data)
		mode |= (OWL_SD_CTL_TS | OWL_SD_CTL_LBE | 0xE4000000);
	else
		mode |= OWL_SD_CTL_TS;

	if (data)
		owl_mmc_prepare_data(priv, data);

	/* Start transfer */
	writel(mode, priv->reg_base + OWL_REG_SD_CTL);

	ret = readl_poll_timeout(priv->reg_base + OWL_REG_SD_CTL, reg,
				 !(reg & OWL_SD_CTL_TS), DATA_TRANSFER_TIMEOUT);

	if (ret == -ETIMEDOUT) {
		debug("error: transferred data timeout\n");
		return ret;
	}

	reg = readl(priv->reg_base + OWL_REG_SD_STATE) & cmd_rsp_mask;
	if (cmd->resp_type & MMC_RSP_PRESENT) {
		if (reg & OWL_SD_STATE_CLNR) {
			printf("Error CMD_NO_RSP\n");
			return -1;
		}

		if (reg & OWL_SD_STATE_CRC7ER) {
			printf("Error CMD_RSP_CRC\n");
			return -1;
		}

		if (cmd->resp_type & MMC_RSP_136) {
			cmd->response[3] = readl(priv->reg_base + OWL_REG_SD_RSPBUF0);
			cmd->response[2] = readl(priv->reg_base + OWL_REG_SD_RSPBUF1);
			cmd->response[1] = readl(priv->reg_base + OWL_REG_SD_RSPBUF2);
			cmd->response[0] = readl(priv->reg_base + OWL_REG_SD_RSPBUF3);
		} else {
			u32 rsp[2];

			rsp[0] = readl(priv->reg_base + OWL_REG_SD_RSPBUF0);
			rsp[1] = readl(priv->reg_base + OWL_REG_SD_RSPBUF1);
			cmd->response[0] = rsp[1] << 24 | rsp[0] >> 8;
			cmd->response[1] = rsp[1] >> 8;
		}
	}

	if (data) {
		ret = readl_poll_timeout(SD_DMA_CHANNEL(priv->dma_channel, 0) + DMA_START,
					 reg, !(reg & DMA_ENABLE), DMA_TRANSFER_TIMEOUT);

		if (ret == -ETIMEDOUT) {
			debug("error: DMA transfer timeout\n");
			return ret;
		}

		/* DMA STOP */
		writel(0x0, SD_DMA_CHANNEL(priv->dma_channel, 0) + DMA_START);
		/* Transmission STOP */
		while (readl(priv->reg_base + OWL_REG_SD_CTL) & OWL_SD_CTL_TS)
			clrbits_le32(priv->reg_base + OWL_REG_SD_CTL,
					OWL_SD_CTL_TS);
	}

	return 0;
}

static int owl_mmc_clk_set(struct owl_mmc_priv *priv, int rate)
{
	u32 reg, wdelay, rdelay;

	reg = readl(priv->reg_base + OWL_REG_SD_CTL);
	reg &= ~OWL_SD_CTL_DELAY_MSK;

	/* Set RDELAY and WDELAY based on the clock */
	if (rate <= 1000000)
		rdelay = wdelay = OWL_SD_DELAY_LOW_CLK;
	else if ((rate > 1000000) && (rate <= 26000000))
		rdelay = wdelay = OWL_SD_DELAY_MID_CLK;
	else if ((rate > 26000000) && (rate <= 52000000)) {
		rdelay = OWL_SD_RDELAY_HIGH;
		wdelay = OWL_SD_WDELAY_HIGH;
	} else {
		debug("SD clock rate not supported\n");
		return -EINVAL;
	}

	writel(reg | OWL_SD_CTL_RDELAY(rdelay) | OWL_SD_CTL_WDELAY(wdelay),
	       priv->reg_base + OWL_REG_SD_CTL);

	return 0;
}

static int owl_mmc_set_ios(struct udevice *dev)
{
	struct owl_mmc_priv *priv = dev_get_priv(dev);
	struct owl_mmc_plat *plat = dev_get_plat(dev);
	struct mmc *mmc = &plat->mmc;
	u32 reg, ret;

	if (mmc->clock != priv->clock) {
		priv->clock = mmc->clock;
		ret = owl_mmc_clk_set(priv, mmc->clock);
		if (IS_ERR_VALUE(ret))
			return ret;

		ret = clk_set_rate(&priv->clk, mmc->clock);
		if (IS_ERR_VALUE(ret))
			return ret;
	}

	if (mmc->clk_disable)
		ret = clk_disable(&priv->clk);
	else
		ret = clk_enable(&priv->clk);
	if (ret)
		return ret;

	/* Set the Bus width */
	reg = readl(priv->reg_base + OWL_REG_SD_EN);
	reg &= ~OWL_SD_EN_DATAWID_MASK;
	if (mmc->bus_width == 8)
		reg |= OWL_SD_EN_DATAWID(2);
	else if (mmc->bus_width == 4)
		reg |= OWL_SD_EN_DATAWID(1);

	writel(reg, priv->reg_base + OWL_REG_SD_EN);

	return 0;
}

static const struct dm_mmc_ops owl_mmc_ops = {
	.send_cmd       = owl_mmc_send_cmd,
	.set_ios        = owl_mmc_set_ios,
};

static int owl_mmc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct owl_mmc_plat *plat = dev_get_plat(dev);
	struct owl_mmc_priv *priv = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	struct ofnode_phandle_args args;
	int ret;
	fdt_addr_t addr;

	cfg->name = dev->name;
	cfg->voltages = OWL_MMC_OCR;
	cfg->f_min = 400000;
	cfg->f_max = 52000000;
	cfg->b_max = 512;
	cfg->host_caps = MMC_MODE_HS | MMC_MODE_HS_52MHz;

	ret = mmc_of_parse(dev, cfg);
	if (ret)
		return ret;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->reg_base = (void *)addr;

	ret = dev_read_phandle_with_args(dev, "dmas", "#dma-cells", 0, 0,
					 &args);
	if (ret)
		return ret;

	priv->dma_channel = (void *)ofnode_get_addr(args.node);
	priv->dma_drq	  = args.args[0];

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret) {
		debug("clk_get_by_index() failed: %d\n", ret);
		return ret;
	}

	upriv->mmc = &plat->mmc;

	return 0;
}

static int owl_mmc_bind(struct udevice *dev)
{
	struct owl_mmc_plat *plat = dev_get_plat(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id owl_mmc_ids[] = {
	{ .compatible = "actions,s700-mmc" },
	{ .compatible = "actions,owl-mmc" },
	{ }
};

U_BOOT_DRIVER(owl_mmc_drv) = {
	.name           = "owl_mmc",
	.id             = UCLASS_MMC,
	.of_match       = owl_mmc_ids,
	.bind           = owl_mmc_bind,
	.probe          = owl_mmc_probe,
	.ops            = &owl_mmc_ops,
	.plat_auto      = sizeof(struct owl_mmc_plat),
	.priv_auto      = sizeof(struct owl_mmc_priv),
};
