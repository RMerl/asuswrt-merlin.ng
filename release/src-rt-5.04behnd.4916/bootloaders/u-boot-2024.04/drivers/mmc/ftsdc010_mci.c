// SPDX-License-Identifier: GPL-2.0+
/*
 * Faraday MMC/SD Host Controller
 *
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * Copyright 2018 Andes Technology, Inc.
 * Author: Rick Chen (rick@andestech.com)
 */

#include <common.h>
#include <clk.h>
#include <log.h>
#include <malloc.h>
#include <part.h>
#include <mmc.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <asm/byteorder.h>
#include <faraday/ftsdc010.h>
#include "ftsdc010_mci.h"
#include <dm.h>
#include <dt-structs.h>
#include <errno.h>
#include <mapmem.h>
#include <pwrseq.h>
#include <syscon.h>
#include <linux/err.h>

#define CFG_CMD_TIMEOUT (CONFIG_SYS_HZ >> 2) /* 250 ms */
#define CFG_RST_TIMEOUT CONFIG_SYS_HZ /* 1 sec reset timeout */

#if CONFIG_IS_ENABLED(OF_PLATDATA)
struct ftsdc010 {
	fdt32_t		bus_width;
	bool		cap_mmc_highspeed;
	bool		cap_sd_highspeed;
	fdt32_t		clock_freq_min_max[2];
	struct phandle_2_cell	clocks[4];
	fdt32_t		fifo_depth;
	fdt32_t		reg[2];
};
#endif

struct ftsdc010_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct ftsdc010 dtplat;
#endif
	struct mmc_config cfg;
	struct mmc mmc;
};

struct ftsdc_priv {
	struct clk clk;
	struct ftsdc010_chip chip;
	int fifo_depth;
	bool fifo_mode;
	u32 minmax[2];
};

static inline int ftsdc010_send_cmd(struct mmc *mmc, struct mmc_cmd *mmc_cmd)
{
	struct ftsdc010_chip *chip = mmc->priv;
	struct ftsdc010_mmc __iomem *regs = chip->regs;
	int ret = -ETIMEDOUT;
	uint32_t ts, st;
	uint32_t cmd   = FTSDC010_CMD_IDX(mmc_cmd->cmdidx);
	uint32_t arg   = mmc_cmd->cmdarg;
	uint32_t flags = mmc_cmd->resp_type;

	cmd |= FTSDC010_CMD_CMD_EN;

	if (chip->acmd) {
		cmd |= FTSDC010_CMD_APP_CMD;
		chip->acmd = 0;
	}

	if (flags & MMC_RSP_PRESENT)
		cmd |= FTSDC010_CMD_NEED_RSP;

	if (flags & MMC_RSP_136)
		cmd |= FTSDC010_CMD_LONG_RSP;

	writel(FTSDC010_STATUS_RSP_MASK | FTSDC010_STATUS_CMD_SEND,
		&regs->clr);
	writel(arg, &regs->argu);
	writel(cmd, &regs->cmd);

	if (!(flags & (MMC_RSP_PRESENT | MMC_RSP_136))) {
		for (ts = get_timer(0); get_timer(ts) < CFG_CMD_TIMEOUT; ) {
			if (readl(&regs->status) & FTSDC010_STATUS_CMD_SEND) {
				writel(FTSDC010_STATUS_CMD_SEND, &regs->clr);
				ret = 0;
				break;
			}
		}
	} else {
		st = 0;
		for (ts = get_timer(0); get_timer(ts) < CFG_CMD_TIMEOUT; ) {
			st = readl(&regs->status);
			writel(st & FTSDC010_STATUS_RSP_MASK, &regs->clr);
			if (st & FTSDC010_STATUS_RSP_MASK)
				break;
		}
		if (st & FTSDC010_STATUS_RSP_CRC_OK) {
			if (flags & MMC_RSP_136) {
				mmc_cmd->response[0] = readl(&regs->rsp3);
				mmc_cmd->response[1] = readl(&regs->rsp2);
				mmc_cmd->response[2] = readl(&regs->rsp1);
				mmc_cmd->response[3] = readl(&regs->rsp0);
			} else {
				mmc_cmd->response[0] = readl(&regs->rsp0);
			}
			ret = 0;
		} else {
			debug("ftsdc010: rsp err (cmd=%d, st=0x%x)\n",
				mmc_cmd->cmdidx, st);
		}
	}

	if (ret) {
		debug("ftsdc010: cmd timeout (op code=%d)\n",
			mmc_cmd->cmdidx);
	} else if (mmc_cmd->cmdidx == MMC_CMD_APP_CMD) {
		chip->acmd = 1;
	}

	return ret;
}

static void ftsdc010_clkset(struct mmc *mmc, uint32_t rate)
{
	struct ftsdc010_chip *chip = mmc->priv;
	struct ftsdc010_mmc __iomem *regs = chip->regs;
	uint32_t div;

	for (div = 0; div < 0x7f; ++div) {
		if (rate >= chip->sclk / (2 * (div + 1)))
			break;
	}
	chip->rate = chip->sclk / (2 * (div + 1));

	writel(FTSDC010_CCR_CLK_DIV(div), &regs->ccr);

	if (IS_SD(mmc)) {
		setbits_le32(&regs->ccr, FTSDC010_CCR_CLK_SD);

		if (chip->rate > 25000000)
			setbits_le32(&regs->ccr, FTSDC010_CCR_CLK_HISPD);
		else
			clrbits_le32(&regs->ccr, FTSDC010_CCR_CLK_HISPD);
	}
}

static int ftsdc010_wait(struct ftsdc010_mmc __iomem *regs, uint32_t mask)
{
	int ret = -ETIMEDOUT;
	uint32_t st, timeout = 10000000;
	while (timeout--) {
		st = readl(&regs->status);
		if (!(st & mask))
			continue;
		writel(st & mask, &regs->clr);
		ret = 0;
		break;
	}

	if (ret){
		debug("ftsdc010: wait st(0x%x) timeout\n", mask);
	}

	return ret;
}

/*
 * u-boot mmc api
 */
static int ftsdc010_request(struct udevice *dev, struct mmc_cmd *cmd,
	struct mmc_data *data)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	int ret = -EOPNOTSUPP;
	uint32_t len = 0;
	struct ftsdc010_chip *chip = mmc->priv;
	struct ftsdc010_mmc __iomem *regs = chip->regs;

	if (data && (data->flags & MMC_DATA_WRITE) && chip->wprot) {
		printf("ftsdc010: the card is write protected!\n");
		return ret;
	}

	if (data) {
		uint32_t dcr;

		len = data->blocksize * data->blocks;

		/* 1. data disable + fifo reset */
		dcr = 0;
#ifdef CONFIG_FTSDC010_SDIO
		dcr |= FTSDC010_DCR_FIFO_RST;
#endif
		writel(dcr, &regs->dcr);

		/* 2. clear status register */
		writel(FTSDC010_STATUS_DATA_MASK | FTSDC010_STATUS_FIFO_URUN
			| FTSDC010_STATUS_FIFO_ORUN, &regs->clr);

		/* 3. data timeout (1 sec) */
		writel(chip->rate, &regs->dtr);

		/* 4. data length (bytes) */
		writel(len, &regs->dlr);

		/* 5. data enable */
		dcr = (ffs(data->blocksize) - 1) | FTSDC010_DCR_DATA_EN;
		if (data->flags & MMC_DATA_WRITE)
			dcr |= FTSDC010_DCR_DATA_WRITE;
		writel(dcr, &regs->dcr);
	}

	ret = ftsdc010_send_cmd(mmc, cmd);
	if (ret) {
		printf("ftsdc010: CMD%d failed\n", cmd->cmdidx);
		return ret;
	}

	if (!data)
		return ret;

	if (data->flags & MMC_DATA_WRITE) {
		const uint8_t *buf = (const uint8_t *)data->src;

		while (len > 0) {
			int wlen;

			/* wait for tx ready */
			ret = ftsdc010_wait(regs, FTSDC010_STATUS_FIFO_URUN);
			if (ret)
				break;

			/* write bytes to ftsdc010 */
			for (wlen = 0; wlen < len && wlen < chip->fifo; ) {
				writel(*(uint32_t *)buf, &regs->dwr);
				buf  += 4;
				wlen += 4;
			}

			len -= wlen;
		}

	} else {
		uint8_t *buf = (uint8_t *)data->dest;

		while (len > 0) {
			int rlen;

			/* wait for rx ready */
			ret = ftsdc010_wait(regs, FTSDC010_STATUS_FIFO_ORUN);
			if (ret)
				break;

			/* fetch bytes from ftsdc010 */
			for (rlen = 0; rlen < len && rlen < chip->fifo; ) {
				*(uint32_t *)buf = readl(&regs->dwr);
				buf  += 4;
				rlen += 4;
			}

			len -= rlen;
		}

	}

	if (!ret) {
		ret = ftsdc010_wait(regs,
			FTSDC010_STATUS_DATA_END | FTSDC010_STATUS_DATA_CRC_OK);
	}

	return ret;
}

static int ftsdc010_set_ios(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	struct ftsdc010_chip *chip = mmc->priv;
	struct ftsdc010_mmc __iomem *regs = chip->regs;

	ftsdc010_clkset(mmc, mmc->clock);

	clrbits_le32(&regs->bwr, FTSDC010_BWR_MODE_MASK);
	switch (mmc->bus_width) {
	case 4:
		setbits_le32(&regs->bwr, FTSDC010_BWR_MODE_4BIT);
		break;
	case 8:
		setbits_le32(&regs->bwr, FTSDC010_BWR_MODE_8BIT);
		break;
	default:
		setbits_le32(&regs->bwr, FTSDC010_BWR_MODE_1BIT);
		break;
	}

	return 0;
}

static int ftsdc010_get_cd(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	struct ftsdc010_chip *chip = mmc->priv;
	struct ftsdc010_mmc __iomem *regs = chip->regs;
	return !(readl(&regs->status) & FTSDC010_STATUS_CARD_DETECT);
}

static int ftsdc010_get_wp(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	struct ftsdc010_chip *chip = mmc->priv;
	struct ftsdc010_mmc __iomem *regs = chip->regs;
	if (readl(&regs->status) & FTSDC010_STATUS_WRITE_PROT) {
		printf("ftsdc010: write protected\n");
		chip->wprot = 1;
	}

	return 0;
}

static int ftsdc010_init(struct mmc *mmc)
{
	struct ftsdc010_chip *chip = mmc->priv;
	struct ftsdc010_mmc __iomem *regs = chip->regs;
	uint32_t ts;

	chip->fifo = (readl(&regs->feature) & 0xff) << 2;

	/* 1. chip reset */
	writel(FTSDC010_CMD_SDC_RST, &regs->cmd);
	for (ts = get_timer(0); get_timer(ts) < CFG_RST_TIMEOUT; ) {
		if (readl(&regs->cmd) & FTSDC010_CMD_SDC_RST)
			continue;
		break;
	}
	if (readl(&regs->cmd) & FTSDC010_CMD_SDC_RST) {
		printf("ftsdc010: reset failed\n");
		return -EOPNOTSUPP;
	}

	/* 2. enter low speed mode (400k card detection) */
	ftsdc010_clkset(mmc, 400000);

	/* 3. interrupt disabled */
	writel(0, &regs->int_mask);

	return 0;
}

static int ftsdc010_probe(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	return ftsdc010_init(mmc);
}

const struct dm_mmc_ops dm_ftsdc010_mmc_ops = {
	.send_cmd	= ftsdc010_request,
	.set_ios	= ftsdc010_set_ios,
	.get_cd		= ftsdc010_get_cd,
	.get_wp		= ftsdc010_get_wp,
};

static void ftsdc_setup_cfg(struct mmc_config *cfg, const char *name, int buswidth,
		     uint caps, u32 max_clk, u32 min_clk)
{
	cfg->name = name;
	cfg->f_min = min_clk;
	cfg->f_max = max_clk;
	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	cfg->host_caps = caps;
	if (buswidth == 8) {
		cfg->host_caps |= MMC_MODE_8BIT;
		cfg->host_caps &= ~MMC_MODE_4BIT;
	} else {
		cfg->host_caps |= MMC_MODE_4BIT;
		cfg->host_caps &= ~MMC_MODE_8BIT;
	}
	cfg->part_type = PART_TYPE_DOS;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;
}

static int ftsdc010_mmc_of_to_plat(struct udevice *dev)
{
	struct ftsdc_priv *priv = dev_get_priv(dev);
	struct ftsdc010_chip *chip = &priv->chip;

	if (CONFIG_IS_ENABLED(OF_REAL)) {
		chip->name = dev->name;
		chip->ioaddr = dev_read_addr_ptr(dev);
		chip->buswidth = dev_read_u32_default(dev, "bus-width", 4);
		chip->priv = dev;
		priv->fifo_depth = dev_read_u32_default(dev, "fifo-depth", 0);
		priv->fifo_mode = dev_read_bool(dev, "fifo-mode");
		if (dev_read_u32_array(dev, "clock-freq-min-max", priv->minmax, 2)) {
			if (dev_read_u32(dev, "max-frequency", &priv->minmax[1]))
				return -EINVAL;

			priv->minmax[0] = 400000;  /* 400 kHz */
		} else {
			debug("%s: 'clock-freq-min-max' property was deprecated.\n",
			      __func__);
		}
	}
	chip->sclk = priv->minmax[1];
	chip->regs = chip->ioaddr;

	return 0;
}

static int ftsdc010_mmc_probe(struct udevice *dev)
{
	struct ftsdc010_plat *plat = dev_get_plat(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct ftsdc_priv *priv = dev_get_priv(dev);
	struct ftsdc010_chip *chip = &priv->chip;
	struct udevice *pwr_dev __maybe_unused;

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	int ret;
	struct ftsdc010 *dtplat = &plat->dtplat;
	chip->name = dev->name;
	chip->ioaddr = map_sysmem(dtplat->reg[0], dtplat->reg[1]);
	chip->buswidth = dtplat->bus_width;
	chip->priv = dev;
	chip->dev_index = 1;
	memcpy(priv->minmax, dtplat->clock_freq_min_max, sizeof(priv->minmax));
	ret = clk_get_by_phandle(dev, dtplat->clocks, &priv->clk);
	if (ret < 0)
		return ret;
#endif

	if (dev_read_bool(dev, "cap-mmc-highspeed") || \
		  dev_read_bool(dev, "cap-sd-highspeed"))
		chip->caps |= MMC_MODE_HS | MMC_MODE_HS_52MHz;

	ftsdc_setup_cfg(&plat->cfg, dev->name, chip->buswidth, chip->caps,
			priv->minmax[1] , priv->minmax[0]);
	chip->mmc = &plat->mmc;
	chip->mmc->priv = &priv->chip;
	chip->mmc->dev = dev;
	upriv->mmc = chip->mmc;
	return ftsdc010_probe(dev);
}

int ftsdc010_mmc_bind(struct udevice *dev)
{
	struct ftsdc010_plat *plat = dev_get_plat(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id ftsdc010_mmc_ids[] = {
	{ .compatible = "andestech,atfsdc010" },
	{ }
};

U_BOOT_DRIVER(ftsdc010_mmc) = {
	.name		= "ftsdc010_mmc",
	.id		= UCLASS_MMC,
	.of_match	= ftsdc010_mmc_ids,
	.of_to_plat = ftsdc010_mmc_of_to_plat,
	.ops		= &dm_ftsdc010_mmc_ops,
	.bind		= ftsdc010_mmc_bind,
	.probe		= ftsdc010_mmc_probe,
	.priv_auto	= sizeof(struct ftsdc_priv),
	.plat_auto	= sizeof(struct ftsdc010_plat),
};
