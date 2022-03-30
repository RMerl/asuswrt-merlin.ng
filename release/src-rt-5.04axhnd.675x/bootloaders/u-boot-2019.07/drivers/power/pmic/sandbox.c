// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2015 Samsung Electronics
 *  Przemyslaw Marczak  <p.marczak@samsung.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/sandbox_pmic.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = SANDBOX_OF_LDO_PREFIX, .driver = SANDBOX_LDO_DRIVER },
	{ .prefix = SANDBOX_OF_BUCK_PREFIX, .driver = SANDBOX_BUCK_DRIVER },
	{ },
};

static int sandbox_pmic_reg_count(struct udevice *dev)
{
	return SANDBOX_PMIC_REG_COUNT;
}

static int sandbox_pmic_write(struct udevice *dev, uint reg,
			      const uint8_t *buff, int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int sandbox_pmic_read(struct udevice *dev, uint reg,
			     uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int sandbox_pmic_bind(struct udevice *dev)
{
	if (!pmic_bind_children(dev, dev_ofnode(dev), pmic_children_info))
		pr_err("%s:%d PMIC: %s - no child found!", __func__, __LINE__,
							  dev->name);

	/* Always return success for this device - allows for PMIC I/O */
	return 0;
}

static struct dm_pmic_ops sandbox_pmic_ops = {
	.reg_count = sandbox_pmic_reg_count,
	.read = sandbox_pmic_read,
	.write = sandbox_pmic_write,
};

static const struct udevice_id sandbox_pmic_ids[] = {
	{ .compatible = "sandbox,pmic" },
	{ }
};

U_BOOT_DRIVER(sandbox_pmic) = {
	.name = "sandbox_pmic",
	.id = UCLASS_PMIC,
	.of_match = sandbox_pmic_ids,
	.bind = sandbox_pmic_bind,
	.ops = &sandbox_pmic_ops,
};
