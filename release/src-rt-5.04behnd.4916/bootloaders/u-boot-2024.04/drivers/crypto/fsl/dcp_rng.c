// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * RNG driver for Freescale RNGC
 *
 * Copyright 2022 NXP
 *
 * Based on RNGC driver in drivers/char/hw_random/imx-rngc.c in Linux
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <rng.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <dm/root.h>
#include <linux/delay.h>
#include <linux/kernel.h>

#define DCP_RNG_MAX_FIFO_STORE_SIZE	4
#define RNGC_VER_ID			0x0
#define RNGC_COMMAND			0x4
#define RNGC_CONTROL			0x8
#define RNGC_STATUS			0xC
#define RNGC_ERROR			0x10
#define RNGC_FIFO			0x14

/* the fields in the ver id register */
#define RNGC_TYPE_SHIFT			28

/* the rng_type field */
#define RNGC_TYPE_RNGB			0x1
#define RNGC_TYPE_RNGC			0x2

#define RNGC_CMD_CLR_ERR		0x20
#define RNGC_CMD_SEED			0x2

#define RNGC_CTRL_AUTO_SEED		0x10

#define RNGC_STATUS_ERROR		0x10000
#define RNGC_STATUS_FIFO_LEVEL_MASK	0xf00
#define RNGC_STATUS_FIFO_LEVEL_SHIFT	8
#define RNGC_STATUS_SEED_DONE		0x20
#define RNGC_STATUS_ST_DONE		0x10

#define RNGC_ERROR_STATUS_STAT_ERR	0x8

#define RNGC_TIMEOUT			3000000U /* 3 sec */

struct imx_rngc_priv {
	unsigned long base;
};

static int rngc_read(struct udevice *dev, void *data, size_t len)
{
	struct imx_rngc_priv *priv = dev_get_priv(dev);
	u8 buffer[DCP_RNG_MAX_FIFO_STORE_SIZE];
	u32 status, level;
	size_t size;

	while (len) {
		status = readl(priv->base + RNGC_STATUS);

		/* is there some error while reading this random number? */
		if (status & RNGC_STATUS_ERROR)
			break;
		/* how many random numbers are in FIFO? [0-16] */
		level = (status & RNGC_STATUS_FIFO_LEVEL_MASK) >>
			RNGC_STATUS_FIFO_LEVEL_SHIFT;

		if (level) {
			/* retrieve a random number from FIFO */
			*(u32 *)buffer = readl(priv->base + RNGC_FIFO);
			size = min(len, sizeof(u32));
			memcpy(data, buffer, size);
			data += size;
			len -= size;
		}
	}

	return len ? -EIO : 0;
}

static int rngc_init(struct imx_rngc_priv *priv)
{
	u32 cmd, ctrl, status, err_reg = 0;
	unsigned long long timeval = 0;
	unsigned long long timeout = RNGC_TIMEOUT;

	/* clear error */
	cmd = readl(priv->base + RNGC_COMMAND);
	writel(cmd | RNGC_CMD_CLR_ERR, priv->base + RNGC_COMMAND);

	/* create seed, repeat while there is some statistical error */
	do {
		/* seed creation */
		cmd = readl(priv->base + RNGC_COMMAND);
		writel(cmd | RNGC_CMD_SEED, priv->base + RNGC_COMMAND);

		udelay(1);
		timeval += 1;

		status = readl(priv->base + RNGC_STATUS);
		err_reg = readl(priv->base + RNGC_ERROR);

		if (status & (RNGC_STATUS_SEED_DONE | RNGC_STATUS_ST_DONE))
			break;

		if (timeval > timeout) {
			debug("rngc timed out\n");
			return -ETIMEDOUT;
		}
	} while (err_reg == RNGC_ERROR_STATUS_STAT_ERR);

	if (err_reg)
		return -EIO;

	/*
	 * enable automatic seeding, the rngc creates a new seed automatically
	 * after serving 2^20 random 160-bit words
	 */
	ctrl = readl(priv->base + RNGC_CONTROL);
	ctrl |= RNGC_CTRL_AUTO_SEED;
	writel(ctrl, priv->base + RNGC_CONTROL);
	return 0;
}

static int rngc_probe(struct udevice *dev)
{
	struct imx_rngc_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;
	u32 ver_id;
	u8  rng_type;
	int ret;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE) {
		ret = -EINVAL;
		goto err;
	}

	priv->base = addr;
	ver_id = readl(priv->base + RNGC_VER_ID);
	rng_type = ver_id >> RNGC_TYPE_SHIFT;
	/*
	 * This driver supports only RNGC and RNGB. (There's a different
	 * driver for RNGA.)
	 */
	if (rng_type != RNGC_TYPE_RNGC && rng_type != RNGC_TYPE_RNGB) {
		ret = -ENODEV;
		goto err;
	}

	ret = rngc_init(priv);
	if (ret)
		goto err;

	return 0;

err:
	printf("%s error = %d\n", __func__, ret);
	return ret;
}

static const struct dm_rng_ops rngc_ops = {
	.read = rngc_read,
};

static const struct udevice_id rngc_dt_ids[] = {
	{ .compatible = "fsl,imx25-rngb" },
	{ }
};

U_BOOT_DRIVER(dcp_rng) = {
	.name = "dcp_rng",
	.id = UCLASS_RNG,
	.of_match = rngc_dt_ids,
	.ops = &rngc_ops,
	.probe = rngc_probe,
	.priv_auto  = sizeof(struct imx_rngc_priv),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
