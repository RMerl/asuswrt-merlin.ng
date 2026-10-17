// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_REGULATOR

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <power/pmic.h>
#include <power/regulator.h>

#define STM32MP_PWR_CR3 0xc
#define STM32MP_PWR_CR3_USB33DEN BIT(24)
#define STM32MP_PWR_CR3_USB33RDY BIT(26)
#define STM32MP_PWR_CR3_REG18DEN BIT(28)
#define STM32MP_PWR_CR3_REG18RDY BIT(29)
#define STM32MP_PWR_CR3_REG11DEN BIT(30)
#define STM32MP_PWR_CR3_REG11RDY BIT(31)

struct stm32mp_pwr_reg_info {
	u32 enable;
	u32 ready;
	char *name;
};

struct stm32mp_pwr_priv {
	fdt_addr_t base;
};

static int stm32mp_pwr_write(struct udevice *dev, uint reg,
			     const uint8_t *buff, int len)
{
	struct stm32mp_pwr_priv *priv = dev_get_priv(dev);
	u32 val = *(u32 *)buff;

	if (len != 4)
		return -EINVAL;

	writel(val, priv->base + STM32MP_PWR_CR3);

	return 0;
}

static int stm32mp_pwr_read(struct udevice *dev, uint reg, uint8_t *buff,
			    int len)
{
	struct stm32mp_pwr_priv *priv = dev_get_priv(dev);

	if (len != 4)
		return -EINVAL;

	*(u32 *)buff = readl(priv->base + STM32MP_PWR_CR3);

	return 0;
}

static int stm32mp_pwr_of_to_plat(struct udevice *dev)
{
	struct stm32mp_pwr_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct pmic_child_info pwr_children_info[] = {
	{ .prefix = "reg", .driver = "stm32mp_pwr_regulator"},
	{ .prefix = "usb", .driver = "stm32mp_pwr_regulator"},
	{ },
};

static int stm32mp_pwr_bind(struct udevice *dev)
{
	int children;

	children = pmic_bind_children(dev, dev_ofnode(dev), pwr_children_info);
	if (!children)
		dev_dbg(dev, "no child found\n");

	return 0;
}

static struct dm_pmic_ops stm32mp_pwr_ops = {
	.read = stm32mp_pwr_read,
	.write = stm32mp_pwr_write,
};

static const struct udevice_id stm32mp_pwr_ids[] = {
	{ .compatible = "st,stm32mp1,pwr-reg" },
	{ }
};

U_BOOT_DRIVER(stm32mp_pwr_pmic) = {
	.name = "stm32mp_pwr_pmic",
	.id = UCLASS_PMIC,
	.of_match = stm32mp_pwr_ids,
	.bind = stm32mp_pwr_bind,
	.ops = &stm32mp_pwr_ops,
	.of_to_plat = stm32mp_pwr_of_to_plat,
	.priv_auto	= sizeof(struct stm32mp_pwr_priv),
};

static const struct stm32mp_pwr_reg_info stm32mp_pwr_reg11 = {
	.enable = STM32MP_PWR_CR3_REG11DEN,
	.ready = STM32MP_PWR_CR3_REG11RDY,
	.name = "reg11"
};

static const struct stm32mp_pwr_reg_info stm32mp_pwr_reg18 = {
	.enable = STM32MP_PWR_CR3_REG18DEN,
	.ready = STM32MP_PWR_CR3_REG18RDY,
	.name = "reg18"
};

static const struct stm32mp_pwr_reg_info stm32mp_pwr_usb33 = {
	.enable = STM32MP_PWR_CR3_USB33DEN,
	.ready = STM32MP_PWR_CR3_USB33RDY,
	.name = "usb33"
};

static const struct stm32mp_pwr_reg_info *stm32mp_pwr_reg_infos[] = {
	&stm32mp_pwr_reg11,
	&stm32mp_pwr_reg18,
	&stm32mp_pwr_usb33,
	NULL
};

static int stm32mp_pwr_regulator_probe(struct udevice *dev)
{
	const struct stm32mp_pwr_reg_info **p = stm32mp_pwr_reg_infos;
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);

	while (*p) {
		int rc;

		rc = dev_read_stringlist_search(dev, "regulator-name",
						(*p)->name);
		if (rc >= 0) {
			dev_dbg(dev, "found regulator %s\n", (*p)->name);
			break;
		} else if (rc != -ENODATA) {
			return rc;
		}
		p++;
	}
	if (!*p) {
		int i = 0;
		const char *s;

		dev_dbg(dev, "regulator ");
		while (dev_read_string_index(dev, "regulator-name",
					     i++, &s) >= 0)
			dev_dbg(dev, "%s'%s' ", (i > 1) ? ", " : "", s);
		dev_dbg(dev, "%s not supported\n", (i > 2) ? "are" : "is");
		return -EINVAL;
	}

	uc_pdata->type = REGULATOR_TYPE_FIXED;
	dev_set_priv(dev, (void *)*p);

	return 0;
}

static int stm32mp_pwr_regulator_set_value(struct udevice *dev, int uV)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	if (!uc_pdata)
		return -ENXIO;

	if (uc_pdata->min_uV != uV) {
		dev_dbg(dev, "Invalid uV=%d for: %s\n", uV, uc_pdata->name);
		return -EINVAL;
	}

	return 0;
}

static int stm32mp_pwr_regulator_get_value(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	if (!uc_pdata)
		return -ENXIO;

	if (uc_pdata->min_uV != uc_pdata->max_uV) {
		dev_dbg(dev, "Invalid constraints for: %s\n", uc_pdata->name);
		return -EINVAL;
	}

	return uc_pdata->min_uV;
}

static int stm32mp_pwr_regulator_get_enable(struct udevice *dev)
{
	const struct stm32mp_pwr_reg_info *p = dev_get_priv(dev);
	int rc;
	u32 reg;

	rc = pmic_read(dev->parent, 0, (uint8_t *)&reg, sizeof(reg));
	if (rc)
		return rc;

	dev_dbg(dev, "%s id %s\n", p->name, (reg & p->enable) ? "on" : "off");

	return (reg & p->enable) != 0;
}

static int stm32mp_pwr_regulator_set_enable(struct udevice *dev, bool enable)
{
	const struct stm32mp_pwr_reg_info *p = dev_get_priv(dev);
	int rc;
	u32 reg;
	u32 time_start;

	dev_dbg(dev, "Turning %s %s\n", enable ? "on" : "off", p->name);

	rc = pmic_read(dev->parent, 0, (uint8_t *)&reg, sizeof(reg));
	if (rc)
		return rc;

	/* if regulator is already in the wanted state, nothing to do */
	if (!!(reg & p->enable) == enable)
		return 0;

	reg &= ~p->enable;
	if (enable)
		reg |= p->enable;

	rc = pmic_write(dev->parent, 0, (uint8_t *)&reg, sizeof(reg));
	if (rc)
		return rc;

	if (!enable)
		return 0;

	/* waiting ready for enable */
	time_start = get_timer(0);
	while (1) {
		rc = pmic_read(dev->parent, 0, (uint8_t *)&reg, sizeof(reg));
		if (rc)
			return rc;
		if (reg & p->ready)
			break;
		if (get_timer(time_start) > CONFIG_SYS_HZ) {
			dev_dbg(dev, "%s: timeout\n", p->name);
			return -ETIMEDOUT;
		}
	}
	return 0;
}

static const struct dm_regulator_ops stm32mp_pwr_regulator_ops = {
	.set_value  = stm32mp_pwr_regulator_set_value,
	.get_value  = stm32mp_pwr_regulator_get_value,
	.get_enable = stm32mp_pwr_regulator_get_enable,
	.set_enable = stm32mp_pwr_regulator_set_enable,
};

U_BOOT_DRIVER(stm32mp_pwr_regulator) = {
	.name = "stm32mp_pwr_regulator",
	.id = UCLASS_REGULATOR,
	.ops = &stm32mp_pwr_regulator_ops,
	.probe = stm32mp_pwr_regulator_probe,
};
