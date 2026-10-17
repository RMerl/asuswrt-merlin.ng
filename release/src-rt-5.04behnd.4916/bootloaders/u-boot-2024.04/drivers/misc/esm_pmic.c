// SPDX-License-Identifier: GPL-2.0+
/*
 * PMIC Error Signal Monitor driver
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *      Tero Kristo <t-kristo@ti.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <power/pmic.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>

#define INT_ESM_REG		0x6c
#define INT_ESM_MASK		0x3f

#define ESM_MCU_START_REG	0x8f

#define ESM_MCU_START		BIT(0)

#define ESM_MCU_MODE_CFG_REG	0x92

#define ESM_MCU_EN		BIT(6)
#define ESM_MCU_ENDRV		BIT(5)

#define ESM_MCU_MASK_REG	0x59
#define ESM_MCU_MASK		0x7

/**
 * pmic_esm_probe: configures and enables PMIC ESM functionality
 *
 * Configures ESM PMIC support and enables it.
 */
static int pmic_esm_probe(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_write(dev->parent, INT_ESM_REG, INT_ESM_MASK);
	if (ret) {
		dev_err(dev, "clearing ESM irqs failed: %d\n", ret);
		return ret;
	}

	ret = pmic_reg_write(dev->parent, ESM_MCU_MODE_CFG_REG,
			     ESM_MCU_EN | ESM_MCU_ENDRV);
	if (ret) {
		dev_err(dev, "setting ESM mode failed: %d\n", ret);
		return ret;
	}

	ret = pmic_reg_write(dev->parent, ESM_MCU_MASK_REG, ESM_MCU_MASK);
	if (ret) {
		dev_err(dev, "clearing ESM masks failed: %d\n", ret);
		return ret;
	}

	ret = pmic_reg_write(dev->parent, ESM_MCU_START_REG, ESM_MCU_START);
	if (ret) {
		dev_err(dev, "starting ESM failed: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id pmic_esm_ids[] = {
	{ .compatible = "ti,tps659413-esm" },
	{}
};

U_BOOT_DRIVER(pmic_esm) = {
	.name = "esm_pmic",
	.of_match = pmic_esm_ids,
	.id = UCLASS_MISC,
	.probe = pmic_esm_probe,
};
