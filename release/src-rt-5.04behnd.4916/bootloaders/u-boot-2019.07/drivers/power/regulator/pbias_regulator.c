// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Texas Instruments Incorporated, <www.ti.com>
 * Jean-Jacques Hiblot <jjhiblot@ti.com>
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <regmap.h>
#include <syscon.h>
#include <linux/bitops.h>
#include <linux/ioport.h>
#include <dm/read.h>
#ifdef CONFIG_MMC_OMAP36XX_PINS
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#endif

struct pbias_reg_info {
	u32 enable;
	u32 enable_mask;
	u32 disable_val;
	u32 vmode;
	unsigned int enable_time;
	char *name;
};

struct pbias_priv {
	struct regmap *regmap;
	int offset;
};

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "pbias", .driver = "pbias_regulator"},
	{ },
};

static int pbias_write(struct udevice *dev, uint reg, const uint8_t *buff,
		       int len)
{
	struct pbias_priv *priv = dev_get_priv(dev);
	u32 val = *(u32 *)buff;

	if (len != 4)
		return -EINVAL;

	return regmap_write(priv->regmap, priv->offset, val);
}

static int pbias_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	struct pbias_priv *priv = dev_get_priv(dev);

	if (len != 4)
		return -EINVAL;

	return regmap_read(priv->regmap, priv->offset, (u32 *)buff);
}

static int pbias_ofdata_to_platdata(struct udevice *dev)
{
	struct pbias_priv *priv = dev_get_priv(dev);
	struct udevice *syscon;
	struct regmap *regmap;
	struct resource res;
	int err;

	err = uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
					   "syscon", &syscon);
	if (err) {
		pr_err("%s: unable to find syscon device (%d)\n", __func__,
		      err);
		return err;
	}

	regmap = syscon_get_regmap(syscon);
	if (IS_ERR(regmap)) {
		pr_err("%s: unable to find regmap (%ld)\n", __func__,
		      PTR_ERR(regmap));
		return PTR_ERR(regmap);
	}
	priv->regmap = regmap;

	err = dev_read_resource(dev, 0, &res);
	if (err) {
		pr_err("%s: unable to find offset (%d)\n", __func__, err);
		return err;
	}
	priv->offset = res.start;

	return 0;
}

static int pbias_bind(struct udevice *dev)
{
	int children;

	children = pmic_bind_children(dev, dev->node, pmic_children_info);
	if (!children)
		debug("%s: %s - no child found\n", __func__, dev->name);

	return 0;
}

static struct dm_pmic_ops pbias_ops = {
	.read = pbias_read,
	.write = pbias_write,
};

static const struct udevice_id pbias_ids[] = {
	{ .compatible = "ti,pbias-dra7" },
	{ .compatible = "ti,pbias-omap2" },
	{ .compatible = "ti,pbias-omap3" },
	{ .compatible = "ti,pbias-omap4" },
	{ .compatible = "ti,pbias-omap5" },
	{ }
};

U_BOOT_DRIVER(pbias_pmic) = {
	.name = "pbias_pmic",
	.id = UCLASS_PMIC,
	.of_match = pbias_ids,
	.bind = pbias_bind,
	.ops = &pbias_ops,
	.ofdata_to_platdata = pbias_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct pbias_priv),
};

static const struct pbias_reg_info pbias_mmc_omap2430 = {
	.enable = BIT(1),
	.enable_mask = BIT(1),
	.vmode = BIT(0),
	.disable_val = 0,
	.enable_time = 100,
	.name = "pbias_mmc_omap2430"
};

static const struct pbias_reg_info pbias_sim_omap3 = {
	.enable = BIT(9),
	.enable_mask = BIT(9),
	.vmode = BIT(8),
	.enable_time = 100,
	.name = "pbias_sim_omap3"
};

static const struct pbias_reg_info pbias_mmc_omap4 = {
	.enable = BIT(26) | BIT(22),
	.enable_mask = BIT(26) | BIT(25) | BIT(22),
	.disable_val = BIT(25),
	.vmode = BIT(21),
	.enable_time = 100,
	.name = "pbias_mmc_omap4"
};

static const struct pbias_reg_info pbias_mmc_omap5 = {
	.enable = BIT(27) | BIT(26),
	.enable_mask = BIT(27) | BIT(25) | BIT(26),
	.disable_val = BIT(25),
	.vmode = BIT(21),
	.enable_time = 100,
	.name = "pbias_mmc_omap5"
};

static const struct pbias_reg_info *pbias_reg_infos[] = {
	&pbias_mmc_omap5,
	&pbias_mmc_omap4,
	&pbias_sim_omap3,
	&pbias_mmc_omap2430,
	NULL
};

static int pbias_regulator_probe(struct udevice *dev)
{
	const struct pbias_reg_info **p = pbias_reg_infos;
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	while (*p) {
		int rc;

		rc = dev_read_stringlist_search(dev, "regulator-name",
						(*p)->name);
		if (rc >= 0) {
			debug("found regulator %s\n", (*p)->name);
			break;
		} else if (rc != -ENODATA) {
			return rc;
		}
		p++;
	}
	if (!*p) {
		int i = 0;
		const char *s;

		debug("regulator ");
		while (dev_read_string_index(dev, "regulator-name", i++, &s) >= 0)
			debug("%s'%s' ", (i > 1) ? ", " : "", s);
		debug("%s not supported\n", (i > 2) ? "are" : "is");
		return -EINVAL;
	}

	uc_pdata->type = REGULATOR_TYPE_OTHER;
	dev->priv = (void *)*p;

	return 0;
}

static int pbias_regulator_get_value(struct udevice *dev)
{
	const struct pbias_reg_info *p = dev_get_priv(dev);
	int rc;
	u32 reg;

	rc = pmic_read(dev->parent, 0, (uint8_t *)&reg, sizeof(reg));
	if (rc)
		return rc;

	debug("%s voltage id %s\n", p->name,
	      (reg & p->vmode) ? "3.0v" : "1.8v");
	return (reg & p->vmode) ? 3000000 : 1800000;
}

static int pbias_regulator_set_value(struct udevice *dev, int uV)
{
	const struct pbias_reg_info *p = dev_get_priv(dev);
	int rc, ret;
	u32 reg;
#ifdef CONFIG_MMC_OMAP36XX_PINS
	u32 wkup_ctrl = readl(OMAP34XX_CTRL_WKUP_CTRL);
#endif

	rc = pmic_read(dev->parent, 0, (uint8_t *)&reg, sizeof(reg));
	if (rc)
		return rc;

	if (uV == 3300000)
		reg |= p->vmode;
	else if (uV == 1800000)
		reg &= ~p->vmode;
	else
		return -EINVAL;

	debug("Setting %s voltage to %s\n", p->name,
	      (reg & p->vmode) ? "3.0v" : "1.8v");

#ifdef CONFIG_MMC_OMAP36XX_PINS
	if (get_cpu_family() == CPU_OMAP36XX) {
		/* Disable extended drain IO before changing PBIAS */
		wkup_ctrl &= ~OMAP34XX_CTRL_WKUP_CTRL_GPIO_IO_PWRDNZ;
		writel(wkup_ctrl, OMAP34XX_CTRL_WKUP_CTRL);
	}
#endif
	ret = pmic_write(dev->parent, 0, (uint8_t *)&reg, sizeof(reg));
#ifdef CONFIG_MMC_OMAP36XX_PINS
	if (get_cpu_family() == CPU_OMAP36XX) {
		/* Enable extended drain IO after changing PBIAS */
		writel(wkup_ctrl |
				OMAP34XX_CTRL_WKUP_CTRL_GPIO_IO_PWRDNZ,
				OMAP34XX_CTRL_WKUP_CTRL);
	}
#endif
	return ret;
}

static int pbias_regulator_get_enable(struct udevice *dev)
{
	const struct pbias_reg_info *p = dev_get_priv(dev);
	int rc;
	u32 reg;

	rc = pmic_read(dev->parent, 0, (uint8_t *)&reg, sizeof(reg));
	if (rc)
		return rc;

	debug("%s id %s\n", p->name,
	      (reg & p->enable_mask) == (p->disable_val) ? "on" : "off");

	return (reg & p->enable_mask) == (p->disable_val);
}

static int pbias_regulator_set_enable(struct udevice *dev, bool enable)
{
	const struct pbias_reg_info *p = dev_get_priv(dev);
	int rc;
	u32 reg;
#ifdef CONFIG_MMC_OMAP36XX_PINS
	u32 wkup_ctrl = readl(OMAP34XX_CTRL_WKUP_CTRL);
#endif

	debug("Turning %s %s\n", enable ? "on" : "off", p->name);

#ifdef CONFIG_MMC_OMAP36XX_PINS
	if (get_cpu_family() == CPU_OMAP36XX) {
		/* Disable extended drain IO before changing PBIAS */
		wkup_ctrl &= ~OMAP34XX_CTRL_WKUP_CTRL_GPIO_IO_PWRDNZ;
		writel(wkup_ctrl, OMAP34XX_CTRL_WKUP_CTRL);
	}
#endif

	rc = pmic_read(dev->parent, 0, (uint8_t *)&reg, sizeof(reg));
	if (rc)
		return rc;

	reg &= ~p->enable_mask;
	if (enable)
		reg |= p->enable;
	else
		reg |= p->disable_val;

	rc = pmic_write(dev->parent, 0, (uint8_t *)&reg, sizeof(reg));

#ifdef CONFIG_MMC_OMAP36XX_PINS
	if (get_cpu_family() == CPU_OMAP36XX) {
		/* Enable extended drain IO after changing PBIAS */
		writel(wkup_ctrl |
				OMAP34XX_CTRL_WKUP_CTRL_GPIO_IO_PWRDNZ,
				OMAP34XX_CTRL_WKUP_CTRL);
	}
#endif

	if (rc)
		return rc;

	if (enable)
		udelay(p->enable_time);

	return 0;
}

static const struct dm_regulator_ops pbias_regulator_ops = {
	.get_value  = pbias_regulator_get_value,
	.set_value  = pbias_regulator_set_value,
	.get_enable = pbias_regulator_get_enable,
	.set_enable = pbias_regulator_set_enable,
};

U_BOOT_DRIVER(pbias_regulator) = {
	.name = "pbias_regulator",
	.id = UCLASS_REGULATOR,
	.ops = &pbias_regulator_ops,
	.probe = pbias_regulator_probe,
};
