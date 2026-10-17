// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Jaehoon Chung <jh80.chung@samsung.com>
 * Portions Copyright 2011-2019 NVIDIA Corporation
 * Portions Copyright 2021 Tianrui Wei
 * This file is adapted from tegra_mmc.c
 * Tianrui Wei <tianrui-wei@outlook.com>
 */

#include <asm/gpio.h>
#include <asm/io.h>
#include <common.h>
#include <div64.h>
#include <dm.h>
#include <errno.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/sizes.h>
#include <log.h>
#include <mmc.h>


#define PITON_MMC_DUMMY_F_MAX 20000000
#define PITON_MMC_DUMMY_F_MIN 10000000
#define PITON_MMC_DUMMY_CAPACITY SZ_4G << 3
#define PITON_MMC_DUMMY_B_MAX SZ_4G

struct piton_mmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct piton_mmc_priv {
	void __iomem *base_addr;
};

static int piton_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
							  struct mmc_data *data)
{
	if (!data)
		return 0;

	struct piton_mmc_priv *priv = dev_get_priv(dev);
	u32 *buff, *start_addr, *write_src;
	size_t byte_cnt, start_block;

	buff = (u32 *)data->dest;
	write_src = (u32 *)data->src;
	start_block = cmd->cmdarg;
	start_addr = priv->base_addr + start_block;

	/* if there is a read */
	for (byte_cnt = data->blocks * data->blocksize; byte_cnt;
				 byte_cnt -= sizeof(u32)) {
		if (data->flags & MMC_DATA_READ) {
			*buff++ = readl(start_addr++);
		}
		else if (data->flags & MMC_DATA_WRITE) {
			writel(*write_src++,start_addr++);
		}
	}
	return 0;
}

static int piton_mmc_ofdata_to_platdata(struct udevice *dev)
{
	struct piton_mmc_priv *priv = dev_get_priv(dev);
	struct piton_mmc_plat *plat = dev_get_plat(dev);
	struct mmc_config *cfg;
	struct mmc *mmc;
	struct blk_desc *bdesc;

	priv->base_addr = dev_read_addr_ptr(dev);
	cfg = &plat->cfg;
	cfg->name = "PITON MMC";
	cfg->host_caps = MMC_MODE_8BIT;
	cfg->f_max = PITON_MMC_DUMMY_F_MAX;
	cfg->f_min = PITON_MMC_DUMMY_F_MIN;
	cfg->voltages = MMC_VDD_21_22;

	mmc = &plat->mmc;
	mmc->read_bl_len = MMC_MAX_BLOCK_LEN;
	mmc->capacity_user = PITON_MMC_DUMMY_CAPACITY;
	mmc->capacity_user *= mmc->read_bl_len;
	mmc->capacity_boot = 0;
	mmc->capacity_rpmb = 0;
	for (int i = 0; i < 4; i++)
		mmc->capacity_gp[i] = 0;
	mmc->capacity = PITON_MMC_DUMMY_CAPACITY;
	mmc->has_init = 1;

	bdesc = mmc_get_blk_desc(mmc);
	bdesc->lun = 0;
	bdesc->hwpart = 0;
	bdesc->type = 0;
	bdesc->blksz = mmc->read_bl_len;
	bdesc->log2blksz = LOG2(bdesc->blksz);
	bdesc->lba = lldiv(mmc->capacity, mmc->read_bl_len);

	return 0;
}

static int piton_mmc_getcd(struct udevice *dev)
{
	return 1;
}

static const struct dm_mmc_ops piton_mmc_ops = {
	.send_cmd = piton_mmc_send_cmd,
	.get_cd = piton_mmc_getcd,
};

static int piton_mmc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct piton_mmc_plat *plat = dev_get_plat(dev);
	struct mmc_config *cfg = &plat->cfg;

	cfg->name = dev->name;
	upriv->mmc = &plat->mmc;
	upriv->mmc->has_init = 1;
	upriv->mmc->capacity = PITON_MMC_DUMMY_CAPACITY;
	upriv->mmc->read_bl_len = MMC_MAX_BLOCK_LEN;
	return 0;
}

static int piton_mmc_bind(struct udevice *dev)
{
	struct piton_mmc_plat *plat = dev_get_plat(dev);
	struct mmc_config *cfg = &plat->cfg;

	cfg->name = dev->name;
	cfg->host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_8BIT;
	cfg->voltages = MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34;
	cfg->f_min = PITON_MMC_DUMMY_F_MIN;
	cfg->f_max = PITON_MMC_DUMMY_F_MAX;
	cfg->b_max = MMC_MAX_BLOCK_LEN;

	return mmc_bind(dev, &plat->mmc, cfg);
}

static const struct udevice_id piton_mmc_ids[] = {
	{.compatible = "openpiton,piton-mmc"},
	{/* sentinel */}
};

U_BOOT_DRIVER(piton_mmc_drv) = {
	.name = "piton_mmc",
	.id = UCLASS_MMC,
	.of_match = piton_mmc_ids,
	.of_to_plat = piton_mmc_ofdata_to_platdata,
	.bind = piton_mmc_bind,
	.probe = piton_mmc_probe,
	.ops = &piton_mmc_ops,
	.plat_auto = sizeof(struct piton_mmc_plat),
	.priv_auto = sizeof(struct piton_mmc_priv),
};
