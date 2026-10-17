// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Vaisala Oyj. All rights reserved.
 */

#include <common.h>
#include <bootcount.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/ioport.h>
#include <regmap.h>
#include <syscon.h>

#define BYTES_TO_BITS(bytes) ((bytes) << 3)
#define GEN_REG_MASK(val_size, val_addr)                                       \
	(GENMASK(BYTES_TO_BITS(val_size) - 1, 0)                               \
	 << (!!((val_addr) == 0x02) * BYTES_TO_BITS(2)))
#define GET_DEFAULT_VALUE(val_size)                                            \
	(CONFIG_SYS_BOOTCOUNT_MAGIC >>                                         \
	 (BYTES_TO_BITS((sizeof(u32) - (val_size)))))

/**
 * struct bootcount_syscon_priv - driver's private data
 *
 * @regmap: syscon regmap
 * @reg_addr: register address used to store the bootcount value
 * @size: size of the bootcount value (2 or 4 bytes)
 * @magic: magic used to validate/save the bootcount value
 * @magic_mask: magic value bitmask
 * @reg_mask: mask used to identify the location of the bootcount value
 * in the register when 2 bytes length is used
 * @shift: value used to extract the botcount value from the register
 */
struct bootcount_syscon_priv {
	struct regmap *regmap;
	fdt_addr_t reg_addr;
	fdt_size_t size;
	u32 magic;
	u32 magic_mask;
	u32 reg_mask;
	int shift;
};

static int bootcount_syscon_set(struct udevice *dev, const u32 val)
{
	struct bootcount_syscon_priv *priv = dev_get_priv(dev);
	u32 regval;

	if ((val & priv->magic_mask) != 0)
		return -EINVAL;

	regval = (priv->magic & priv->magic_mask) | (val & ~priv->magic_mask);

	if (priv->size == 2) {
		regval &= 0xffff;
		regval |= (regval & 0xffff) << BYTES_TO_BITS(priv->size);
	}

	debug("%s: Prepare to write reg value: 0x%08x with register mask: 0x%08x\n",
	      __func__, regval, priv->reg_mask);

	return regmap_update_bits(priv->regmap, priv->reg_addr, priv->reg_mask,
				  regval);
}

static int bootcount_syscon_get(struct udevice *dev, u32 *val)
{
	struct bootcount_syscon_priv *priv = dev_get_priv(dev);
	u32 regval;
	int ret;

	ret = regmap_read(priv->regmap, priv->reg_addr, &regval);
	if (ret)
		return ret;

	regval &= priv->reg_mask;
	regval >>= priv->shift;

	if ((regval & priv->magic_mask) == (priv->magic & priv->magic_mask)) {
		*val = regval & ~priv->magic_mask;
	} else {
		dev_err(dev, "%s: Invalid bootcount magic\n", __func__);
		return -EINVAL;
	}

	debug("%s: Read bootcount value: 0x%08x from regval: 0x%08x\n",
	      __func__, *val, regval);
	return 0;
}

static int bootcount_syscon_of_to_plat(struct udevice *dev)
{
	struct bootcount_syscon_priv *priv = dev_get_priv(dev);
	fdt_addr_t bootcount_offset;
	fdt_size_t reg_size;

	priv->regmap = syscon_regmap_lookup_by_phandle(dev, "syscon");
	if (IS_ERR(priv->regmap)) {
		dev_err(dev, "%s: Unable to find regmap (%ld)\n", __func__,
			PTR_ERR(priv->regmap));
		return PTR_ERR(priv->regmap);
	}

	priv->reg_addr = dev_read_addr_size_name(dev, "syscon_reg", &reg_size);
	if (priv->reg_addr == FDT_ADDR_T_NONE) {
		dev_err(dev, "%s: syscon_reg address not found\n", __func__);
		return -EINVAL;
	}
	if (reg_size != 4) {
		dev_err(dev, "%s: Unsupported register size: %pa\n", __func__,
			&reg_size);
		return -EINVAL;
	}

	bootcount_offset = dev_read_addr_size_name(dev, "offset", &priv->size);
	if (bootcount_offset == FDT_ADDR_T_NONE) {
		dev_err(dev, "%s: offset configuration not found\n", __func__);
		return -EINVAL;
	}
	if (bootcount_offset + priv->size > reg_size) {
		dev_err(dev,
			"%s: Bootcount value doesn't fit in the reserved space\n",
			__func__);
		return -EINVAL;
	}
	if (priv->size != 2 && priv->size != 4) {
		dev_err(dev,
			"%s: Driver supports only 2 and 4 bytes bootcount size\n",
			__func__);
		return -EINVAL;
	}

	priv->magic = GET_DEFAULT_VALUE(priv->size);
	priv->magic_mask = GENMASK(BYTES_TO_BITS(priv->size) - 1,
				   BYTES_TO_BITS(priv->size >> 1));
	priv->shift = !!(bootcount_offset == 0x02) * BYTES_TO_BITS(priv->size);
	priv->reg_mask = GEN_REG_MASK(priv->size, bootcount_offset);

	return 0;
}

static const struct bootcount_ops bootcount_syscon_ops = {
	.get = bootcount_syscon_get,
	.set = bootcount_syscon_set,
};

static const struct udevice_id bootcount_syscon_ids[] = {
	{ .compatible = "u-boot,bootcount-syscon" },
	{}
};

U_BOOT_DRIVER(bootcount_syscon) = {
	.name = "bootcount-syscon",
	.id = UCLASS_BOOTCOUNT,
	.of_to_plat = bootcount_syscon_of_to_plat,
	.priv_auto = sizeof(struct bootcount_syscon_priv),
	.of_match = bootcount_syscon_ids,
	.ops = &bootcount_syscon_ops,
};
