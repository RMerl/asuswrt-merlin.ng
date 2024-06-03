// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2020, Matthias Brugger <mbrugger@suse.com>
 *
 * Driver for Raspberry Pi hardware random number generator
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <linux/delay.h>
#include <rng.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/ioport.h>

#define usleep_range(a, b) udelay((b))

#define RNG_CTRL_OFFSET					0x00
#define RNG_CTRL_RNG_RBGEN_MASK				0x00001FFF
#define RNG_CTRL_RNG_RBGEN_ENABLE			0x00000001
#define RNG_CTRL_RNG_RBGEN_DISABLE			0x00000000

#define RNG_SOFT_RESET_OFFSET				0x04
#define RNG_SOFT_RESET					0x00000001

#define RBG_SOFT_RESET_OFFSET				0x08
#define RBG_SOFT_RESET					0x00000001

#define RNG_INT_STATUS_OFFSET				0x18
#define RNG_INT_STATUS_MASTER_FAIL_LOCKOUT_IRQ_MASK	0x80000000
#define RNG_INT_STATUS_NIST_FAIL_IRQ_MASK		0x00000020

#define RNG_FIFO_DATA_OFFSET				0x20

#define RNG_FIFO_COUNT_OFFSET				0x24
#define RNG_FIFO_COUNT_RNG_FIFO_COUNT_MASK		0x000000FF

#define RNG_FIFO_DATA_BIT_MAX		(16*32) 
#define RNG_FIFO_DATA_MAX		(RNG_FIFO_DATA_BIT_MAX/8) 
#define RNG_FIFO_CNT_EMPTY_SHIFT	31
#define RNG_FIFO_CNT_FULL_SHIFT		30

struct iproc_rng200_platdata {
	fdt_addr_t base;
};

static void iproc_rng200_enable(struct iproc_rng200_platdata *pdata, bool enable)
{
	fdt_addr_t rng_base = pdata->base;
	u32 val;

	val = readl(rng_base + RNG_CTRL_OFFSET);
	val &= ~RNG_CTRL_RNG_RBGEN_MASK;
	if (enable)
		val |= RNG_CTRL_RNG_RBGEN_ENABLE;
	else
		val &= ~RNG_CTRL_RNG_RBGEN_ENABLE;

	writel(val, rng_base + RNG_CTRL_OFFSET);
}

static void iproc_rng200_restart(struct iproc_rng200_platdata *pdata)
{
	fdt_addr_t rng_base = pdata->base;
	u32 val;

	iproc_rng200_enable(pdata, false);

	/* Clear all interrupt status */
	writel(0xFFFFFFFFUL, rng_base + RNG_INT_STATUS_OFFSET);

	/* Reset RNG and RBG */
	val = readl(rng_base + RBG_SOFT_RESET_OFFSET);
	val |= RBG_SOFT_RESET;
	writel(val, rng_base + RBG_SOFT_RESET_OFFSET);

	val = readl(rng_base + RNG_SOFT_RESET_OFFSET);
	val |= RNG_SOFT_RESET;
	writel(val, rng_base + RNG_SOFT_RESET_OFFSET);

	val = readl(rng_base + RNG_SOFT_RESET_OFFSET);
	val &= ~RNG_SOFT_RESET;
	writel(val, rng_base + RNG_SOFT_RESET_OFFSET);

	val = readl(rng_base + RBG_SOFT_RESET_OFFSET);
	val &= ~RBG_SOFT_RESET;
	writel(val, rng_base + RBG_SOFT_RESET_OFFSET);

	iproc_rng200_enable(pdata, true);
}

static int iproc_rng200_read(struct udevice *dev, void *data, size_t len)
{
	struct iproc_rng200_platdata *priv = dev_get_platdata(dev);
	char *buf = (char *)data;
	u32 num_remaining = len;
	u32 status;

	#define MAX_RESETS_PER_READ	1
	u32 num_resets = 0;

	while (num_remaining > 0) {

		/* Is RNG sane? If not, reset it. */
		status = readl(priv->base + RNG_INT_STATUS_OFFSET);
		if ((status & (RNG_INT_STATUS_MASTER_FAIL_LOCKOUT_IRQ_MASK |
			RNG_INT_STATUS_NIST_FAIL_IRQ_MASK)) != 0) {

			if (num_resets >= MAX_RESETS_PER_READ)
				return len - num_remaining;

			iproc_rng200_restart(priv);
			num_resets++;
		}

		/* Are there any random numbers available? */
		if ((readl(priv->base + RNG_FIFO_COUNT_OFFSET) &
				RNG_FIFO_COUNT_RNG_FIFO_COUNT_MASK) > 0) {

			if (num_remaining >= sizeof(u32)) {
				/* Buffer has room to store entire word */
				*(u32 *)buf = readl(priv->base +
							RNG_FIFO_DATA_OFFSET);
				buf += sizeof(u32);
				num_remaining -= sizeof(u32);
			} else {
				/* Buffer can only store partial word */
				u32 rnd_number = readl(priv->base +
							RNG_FIFO_DATA_OFFSET);
				memcpy(buf, &rnd_number, num_remaining);
				buf += num_remaining;
				num_remaining = 0;
			}

		} else {
			/* Can wait, give others chance to run */
			usleep_range(min(num_remaining * 10, 500U), 500);
		}
	}

	return 0;
}

static int iproc_rng200_probe(struct udevice *dev)
{
	struct iproc_rng200_platdata *priv = dev_get_platdata(dev);

	priv->base = (uintptr_t)dev_remap_addr(dev);
	if (!priv->base) {
		dev_err(dev, "can't get rng regs !\n");
		return -EINVAL;
	}

	iproc_rng200_enable(priv, true);

	return 0;
}

static int iproc_rng200_remove(struct udevice *dev)
{
	struct iproc_rng200_platdata *priv = dev_get_platdata(dev);

	iproc_rng200_enable(priv, false);

	return 0;
}

static const struct dm_rng_ops iproc_rng200_ops = {
	.read = iproc_rng200_read,
};

static const struct udevice_id iproc_rng200_rng_match[] = {
	{ .compatible = "brcm,bcm2711-rng200", },
	{ .compatible = "brcm,iproc-rng200", },
	{},
};

U_BOOT_DRIVER(iproc_rng200_rng) = {
	.name = "iproc_rng200-rng",
	.id = UCLASS_RNG,
	.of_match = iproc_rng200_rng_match,
	.ops = &iproc_rng200_ops,
	.probe = iproc_rng200_probe,
	.remove = iproc_rng200_remove,
	.platdata_auto_alloc_size = sizeof(struct iproc_rng200_platdata),
};
