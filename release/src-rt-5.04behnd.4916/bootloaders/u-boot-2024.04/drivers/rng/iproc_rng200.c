// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2020, Matthias Brugger <mbrugger@suse.com>
 *
 * Driver for Raspberry Pi hardware random number generator
 */

#include <common.h>
#include <dm.h>
#include <linux/delay.h>
#include <rng.h>
#include <asm/io.h>

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

struct iproc_rng200_plat {
	void __iomem *base;
};

static void iproc_rng200_enable(struct iproc_rng200_plat *pdata, bool enable)
{
	void __iomem *rng_base = pdata->base;
	u32 val;

	val = readl(rng_base + RNG_CTRL_OFFSET);
	val &= ~RNG_CTRL_RNG_RBGEN_MASK;
	if (enable)
		val |= RNG_CTRL_RNG_RBGEN_ENABLE;
	else
		val &= ~RNG_CTRL_RNG_RBGEN_ENABLE;

	writel(val, rng_base + RNG_CTRL_OFFSET);
}

static void iproc_rng200_restart(struct iproc_rng200_plat *pdata)
{
	void __iomem *rng_base = pdata->base;
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
	struct iproc_rng200_plat *priv = dev_get_plat(dev);
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
	struct iproc_rng200_plat *priv = dev_get_plat(dev);

	iproc_rng200_enable(priv, true);

	return 0;
}

static int iproc_rng200_remove(struct udevice *dev)
{
	struct iproc_rng200_plat *priv = dev_get_plat(dev);

	iproc_rng200_enable(priv, false);

	return 0;
}

static int iproc_rng200_of_to_plat(struct udevice *dev)
{
	struct iproc_rng200_plat *pdata = dev_get_plat(dev);

	pdata->base = devfdt_map_physmem(dev, sizeof(void *));
	if (!pdata->base)
		return -ENODEV;

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
	.priv_auto = sizeof(struct iproc_rng200_plat),
	.of_to_plat = iproc_rng200_of_to_plat,
};
