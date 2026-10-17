// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 *
 * This driver supports the Security Fuse Processor device found on some
 * Layerscape processors. At the moment, we only support a few processors.
 * This driver was written with reference to the Layerscape SDK User
 * Guide [1] and the ATF SFP driver [2].
 *
 * [1] https://docs.nxp.com/bundle/GUID-487B2E69-BB19-42CB-AC38-7EF18C0FE3AE/page/GUID-27FC40AD-3321-4A82-B29E-7BB49EE94F23.html
 * [2] https://source.codeaurora.org/external/qoriq/qoriq-components/atf/tree/drivers/nxp/sfp?h=github.com/master
 */

#define LOG_CATEGORY UCLASS_MISC
#include <common.h>
#include <clk.h>
#include <fuse.h>
#include <misc.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/read.h>
#include <linux/bitfield.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

#define SFP_INGR	0x20
#define SFP_SVHESR	0x24
#define SFP_SFPCR	0x28

#define SFP_START	0x200
#define SFP_END		0x284
#define SFP_SIZE	(SFP_END - SFP_START + 4)

#define SFP_INGR_ERR	BIT(8)
#define SFP_INGR_INST	GENMASK(7, 0)

#define SFP_INGR_READFB	0x01
#define SFP_INGR_PROGFB	0x02

#define SFP_SFPCR_PPW	GENMASK(15, 0)

enum ls2_sfp_ioctl {
	LS2_SFP_IOCTL_READ,
	LS2_SFP_IOCTL_PROG,
};

/**
 * struct ls2_sfp_priv - private data for LS2 SFP
 * @base: Base address of SFP
 * @supply: The (optional) supply for TA_PROG_SFP
 * @programmed: Whether we've already programmed the fuses since the last
 *              reset. The SFP has a *very* limited amount of programming
 *              cycles (two to six, depending on the model), so we try and
 *              prevent accidentally performing additional programming
 *              cycles.
 * @dirty: Whether the mirror registers have been written to (overridden)
 *         since we've last read the fuses (either as part of the reset
 *         process or using a READFB instruction). There is a much larger,
 *         but still finite, limit on the number of SFP read cycles (around
 *         300,000), so we try and minimize reads as well.
 */
struct ls2_sfp_priv {
	void __iomem *base;
	struct udevice *supply;
	bool programmed, dirty;
};

static u32 ls2_sfp_readl(struct ls2_sfp_priv *priv, ulong off)
{
	u32 val = be32_to_cpu(readl(priv->base + off));

	log_debug("%08x = readl(%p)\n", val, priv->base + off);
	return val;
}

static void ls2_sfp_writel(struct ls2_sfp_priv *priv, ulong val, ulong off)
{
	log_debug("writel(%08lx, %p)\n", val, priv->base + off);
	writel(cpu_to_be32(val), priv->base + off);
}

static bool ls2_sfp_validate(struct udevice *dev, int offset, int size)
{
	if (offset < 0 || size < 0) {
		dev_notice(dev, "size and offset must be positive\n");
		return false;
	}

	if (offset & 3 || size & 3) {
		dev_notice(dev, "size and offset must be multiples of 4\n");
		return false;
	}

	if (offset + size > SFP_SIZE) {
		dev_notice(dev, "size + offset must be <= %#x\n", SFP_SIZE);
		return false;
	}

	return true;
}

static int ls2_sfp_read(struct udevice *dev, int offset, void *buf_bytes,
			int size)
{
	int i;
	struct ls2_sfp_priv *priv = dev_get_priv(dev);
	u32 *buf = buf_bytes;

	if (!ls2_sfp_validate(dev, offset, size))
		return -EINVAL;

	for (i = 0; i < size; i += 4)
		buf[i >> 2] = ls2_sfp_readl(priv, SFP_START + offset + i);

	return size;
}

static int ls2_sfp_write(struct udevice *dev, int offset,
			 const void *buf_bytes, int size)
{
	int i;
	struct ls2_sfp_priv *priv = dev_get_priv(dev);
	const u32 *buf = buf_bytes;

	if (!ls2_sfp_validate(dev, offset, size))
		return -EINVAL;

	for (i = 0; i < size; i += 4)
		ls2_sfp_writel(priv, buf[i >> 2], SFP_START + offset + i);

	priv->dirty = true;
	return size;
}

static int ls2_sfp_check_secret(struct udevice *dev)
{
	struct ls2_sfp_priv *priv = dev_get_priv(dev);
	u32 svhesr = ls2_sfp_readl(priv, SFP_SVHESR);

	if (svhesr) {
		dev_warn(dev, "secret value hamming error not zero: %08x\n",
			 svhesr);
		return -EIO;
	}
	return 0;
}

static int ls2_sfp_transaction(struct ls2_sfp_priv *priv, ulong inst)
{
	u32 ingr;

	ls2_sfp_writel(priv, inst, SFP_INGR);

	do {
		ingr = ls2_sfp_readl(priv, SFP_INGR);
	} while (FIELD_GET(SFP_INGR_INST, ingr));

	return FIELD_GET(SFP_INGR_ERR, ingr) ? -EIO : 0;
}

static int ls2_sfp_ioctl(struct udevice *dev, unsigned long request, void *buf)
{
	int ret;
	struct ls2_sfp_priv *priv = dev_get_priv(dev);

	switch (request) {
	case LS2_SFP_IOCTL_READ:
		if (!priv->dirty) {
			dev_dbg(dev, "ignoring read request, since fuses are not dirty\n");
			return 0;
		}

		ret = ls2_sfp_transaction(priv, SFP_INGR_READFB);
		if (ret) {
			dev_err(dev, "error reading fuses\n");
			return ret;
		}

		ls2_sfp_check_secret(dev);
		priv->dirty = false;
		return 0;
	case LS2_SFP_IOCTL_PROG:
		if (priv->programmed) {
			dev_warn(dev, "fuses already programmed\n");
			return -EPERM;
		}

		ret = ls2_sfp_check_secret(dev);
		if (ret)
			return ret;

		if (priv->supply) {
			ret = regulator_set_enable(priv->supply, true);
			if (ret)
				return ret;
		}

		ret = ls2_sfp_transaction(priv, SFP_INGR_PROGFB);
		priv->programmed = true;
		if (priv->supply)
			regulator_set_enable(priv->supply, false);

		if (ret)
			dev_err(dev, "error programming fuses\n");
		return ret;
	default:
		dev_dbg(dev, "unknown ioctl %lu\n", request);
		return -EINVAL;
	}
}

static const struct misc_ops ls2_sfp_ops = {
	.read = ls2_sfp_read,
	.write = ls2_sfp_write,
	.ioctl = ls2_sfp_ioctl,
};

static int ls2_sfp_probe(struct udevice *dev)
{
	int ret;
	struct clk clk;
	struct ls2_sfp_priv *priv = dev_get_priv(dev);
	ulong rate;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base) {
		dev_dbg(dev, "could not read register base\n");
		return -EINVAL;
	}

	ret = device_get_supply_regulator(dev, "ta-sfp-prog-supply", &priv->supply);
	if (ret && ret != -ENODEV && ret != -ENOSYS) {
		dev_dbg(dev, "problem getting supply (err %d)\n", ret);
		return ret;
	}

	ret = clk_get_by_name(dev, "sfp", &clk);
	if (ret == -ENOSYS) {
		rate = gd->bus_clk / 4;
	} else if (ret) {
		dev_dbg(dev, "could not get clock (err %d)\n", ret);
		return ret;
	} else {
		ret = clk_enable(&clk);
		if (ret) {
			dev_dbg(dev, "could not enable clock (err %d)\n", ret);
			return ret;
		}

		rate = clk_get_rate(&clk);
		if (!rate || IS_ERR_VALUE(rate)) {
			ret = rate ? rate : -ENOENT;
			dev_dbg(dev, "could not get clock rate (err %d)\n",
				ret);
			return ret;
		}
	}

	/* sfp clock in MHz * 12 */
	ls2_sfp_writel(priv, FIELD_PREP(SFP_SFPCR_PPW, rate * 12 / 1000000),
		       SFP_SFPCR);

	ls2_sfp_check_secret(dev);
	return 0;
}

static const struct udevice_id ls2_sfp_ids[] = {
	{ .compatible = "fsl,ls1021a-sfp" },
	{ }
};

U_BOOT_DRIVER(ls2_sfp) = {
	.name		= "ls2_sfp",
	.id		= UCLASS_MISC,
	.of_match	= ls2_sfp_ids,
	.probe		= ls2_sfp_probe,
	.ops		= &ls2_sfp_ops,
	.priv_auto	= sizeof(struct ls2_sfp_priv),
};

static int ls2_sfp_device(struct udevice **dev)
{
	int ret = uclass_get_device_by_driver(UCLASS_MISC,
					      DM_DRIVER_GET(ls2_sfp), dev);

	if (ret)
		log_debug("device not found (err %d)\n", ret);
	return ret;
}

int fuse_read(u32 bank, u32 word, u32 *val)
{
	int ret;
	struct udevice *dev;

	ret = ls2_sfp_device(&dev);
	if (ret)
		return ret;

	ret = misc_ioctl(dev, LS2_SFP_IOCTL_READ, NULL);
	if (ret)
		return ret;

	ret = misc_read(dev, word << 2, val, sizeof(*val));
	return ret < 0 ? ret : 0;
}

int fuse_sense(u32 bank, u32 word, u32 *val)
{
	int ret;
	struct udevice *dev;

	ret = ls2_sfp_device(&dev);
	if (ret)
		return ret;

	ret = misc_read(dev, word << 2, val, sizeof(*val));
	return ret < 0 ? ret : 0;
}

int fuse_prog(u32 bank, u32 word, u32 val)
{
	int ret;
	struct udevice *dev;

	ret = ls2_sfp_device(&dev);
	if (ret)
		return ret;

	ret = misc_write(dev, word << 2, &val, sizeof(val));
	if (ret < 0)
		return ret;

	return misc_ioctl(dev, LS2_SFP_IOCTL_PROG, NULL);
}

int fuse_override(u32 bank, u32 word, u32 val)
{
	int ret;
	struct udevice *dev;

	ret = ls2_sfp_device(&dev);
	if (ret)
		return ret;

	ret = misc_write(dev, word << 2, &val, sizeof(val));
	return ret < 0 ? ret : 0;
}
