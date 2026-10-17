// SPDX-License-Identifier: GPL-2.0+
/*
 * Compatible code for non CCF AT91 platforms.
 *
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 */
#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/util.h>
#include <mach/at91_pmc.h>
#include <mach/at91_sfr.h>
#include <regmap.h>
#include <syscon.h>

#include "pmc.h"

DECLARE_GLOBAL_DATA_PTR;

struct pmc_plat {
	struct at91_pmc *reg_base;
	struct regmap *regmap_sfr;
};

static const struct udevice_id at91_pmc_match[] = {
	{ .compatible = "atmel,at91rm9200-pmc" },
	{ .compatible = "atmel,at91sam9260-pmc" },
	{ .compatible = "atmel,at91sam9g45-pmc" },
	{ .compatible = "atmel,at91sam9n12-pmc" },
	{ .compatible = "atmel,at91sam9x5-pmc" },
	{ .compatible = "atmel,sama5d3-pmc" },
	{ .compatible = "atmel,sama5d2-pmc" },
	{}
};

U_BOOT_DRIVER(at91_pmc) = {
	.name = "at91-pmc",
	.id = UCLASS_SIMPLE_BUS,
	.of_match = at91_pmc_match,
};

static int at91_pmc_core_probe(struct udevice *dev)
{
	struct pmc_plat *plat = dev_get_plat(dev);

	dev = dev_get_parent(dev);

	plat->reg_base = dev_read_addr_ptr(dev);

	return 0;
}

/**
 * at91_clk_sub_device_bind() - for the at91 clock driver
 * Recursively bind its children as clk devices.
 *
 * @return: 0 on success, or negative error code on failure
 */
int at91_clk_sub_device_bind(struct udevice *dev, const char *drv_name)
{
	const void *fdt = gd->fdt_blob;
	int offset = dev_of_offset(dev);
	bool pre_reloc_only = !(gd->flags & GD_FLG_RELOC);
	const char *name;
	int ret;

	for (offset = fdt_first_subnode(fdt, offset);
	     offset > 0;
	     offset = fdt_next_subnode(fdt, offset)) {
		if (pre_reloc_only &&
		    !ofnode_pre_reloc(offset_to_ofnode(offset)))
			continue;
		/*
		 * If this node has "compatible" property, this is not
		 * a clock sub-node, but a normal device. skip.
		 */
		fdt_get_property(fdt, offset, "compatible", &ret);
		if (ret >= 0)
			continue;

		if (ret != -FDT_ERR_NOTFOUND)
			return ret;

		name = fdt_get_name(fdt, offset, NULL);
		if (!name)
			return -EINVAL;
		ret = device_bind_driver_to_node(dev, drv_name, name,
					offset_to_ofnode(offset), NULL);
		if (ret)
			return ret;
	}

	return 0;
}

int at91_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	int periph;

	if (args->args_count) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	periph = fdtdec_get_uint(gd->fdt_blob, dev_of_offset(clk->dev), "reg",
				 -1);
	if (periph < 0)
		return -EINVAL;

	clk->id = periph;

	return 0;
}

int at91_clk_probe(struct udevice *dev)
{
	struct udevice *dev_periph_container, *dev_pmc;
	struct pmc_plat *plat = dev_get_plat(dev);

	dev_periph_container = dev_get_parent(dev);
	dev_pmc = dev_get_parent(dev_periph_container);

	plat->reg_base = dev_read_addr_ptr(dev_pmc);

	return 0;
}

/* SCKC specific code. */
static const struct udevice_id at91_sckc_match[] = {
	{ .compatible = "atmel,at91sam9x5-sckc" },
	{}
};

U_BOOT_DRIVER(at91_sckc) = {
	.name = "at91-sckc",
	.id = UCLASS_SIMPLE_BUS,
	.of_match = at91_sckc_match,
};

/* Slow clock specific code. */
static int at91_slow_clk_enable(struct clk *clk)
{
	return 0;
}

static ulong at91_slow_clk_get_rate(struct clk *clk)
{
	return CFG_SYS_AT91_SLOW_CLOCK;
}

static struct clk_ops at91_slow_clk_ops = {
	.enable = at91_slow_clk_enable,
	.get_rate = at91_slow_clk_get_rate,
};

static const struct udevice_id at91_slow_clk_match[] = {
	{ .compatible = "atmel,at91sam9x5-clk-slow" },
	{}
};

U_BOOT_DRIVER(at91_slow_clk) = {
	.name = "at91-slow-clk",
	.id = UCLASS_CLK,
	.of_match = at91_slow_clk_match,
	.ops = &at91_slow_clk_ops,
};

/* Master clock specific code. */
static ulong at91_master_clk_get_rate(struct clk *clk)
{
	return gd->arch.mck_rate_hz;
}

static struct clk_ops at91_master_clk_ops = {
	.get_rate = at91_master_clk_get_rate,
};

static const struct udevice_id at91_master_clk_match[] = {
	{ .compatible = "atmel,at91rm9200-clk-master" },
	{ .compatible = "atmel,at91sam9x5-clk-master" },
	{}
};

U_BOOT_DRIVER(at91_master_clk) = {
	.name = "at91-master-clk",
	.id = UCLASS_CLK,
	.of_match = at91_master_clk_match,
	.ops = &at91_master_clk_ops,
};

/* Main osc clock specific code. */
static int main_osc_clk_enable(struct clk *clk)
{
	struct pmc_plat *plat = dev_get_plat(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;

	if (readl(&pmc->sr) & AT91_PMC_MOSCSELS)
		return 0;

	return -EINVAL;
}

static ulong main_osc_clk_get_rate(struct clk *clk)
{
	return gd->arch.main_clk_rate_hz;
}

static struct clk_ops main_osc_clk_ops = {
	.enable = main_osc_clk_enable,
	.get_rate = main_osc_clk_get_rate,
};

static int main_osc_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id main_osc_clk_match[] = {
	{ .compatible = "atmel,at91sam9x5-clk-main" },
	{}
};

U_BOOT_DRIVER(at91sam9x5_main_osc_clk) = {
	.name = "at91sam9x5-main-osc-clk",
	.id = UCLASS_CLK,
	.of_match = main_osc_clk_match,
	.probe = main_osc_clk_probe,
	.plat_auto	= sizeof(struct pmc_plat),
	.ops = &main_osc_clk_ops,
};

/* PLLA clock specific code. */
static int plla_clk_enable(struct clk *clk)
{
	struct pmc_plat *plat = dev_get_plat(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;

	if (readl(&pmc->sr) & AT91_PMC_LOCKA)
		return 0;

	return -EINVAL;
}

static ulong plla_clk_get_rate(struct clk *clk)
{
	return gd->arch.plla_rate_hz;
}

static struct clk_ops plla_clk_ops = {
	.enable = plla_clk_enable,
	.get_rate = plla_clk_get_rate,
};

static int plla_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id plla_clk_match[] = {
	{ .compatible = "atmel,sama5d3-clk-pll" },
	{}
};

U_BOOT_DRIVER(at91_plla_clk) = {
	.name = "at91-plla-clk",
	.id = UCLASS_CLK,
	.of_match = plla_clk_match,
	.probe = plla_clk_probe,
	.plat_auto	= sizeof(struct pmc_plat),
	.ops = &plla_clk_ops,
};

/* PLLA DIV clock specific code. */
static int at91_plladiv_clk_enable(struct clk *clk)
{
	return 0;
}

static ulong at91_plladiv_clk_get_rate(struct clk *clk)
{
	struct pmc_plat *plat = dev_get_plat(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct clk source;
	ulong clk_rate;
	int ret;

	ret = clk_get_by_index(clk->dev, 0, &source);
	if (ret)
		return -EINVAL;

	clk_rate = clk_get_rate(&source);
	if (readl(&pmc->mckr) & AT91_PMC_MCKR_PLLADIV_2)
		clk_rate /= 2;

	return clk_rate;
}

static ulong at91_plladiv_clk_set_rate(struct clk *clk, ulong rate)
{
	struct pmc_plat *plat = dev_get_plat(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct clk source;
	ulong parent_rate;
	int ret;

	ret = clk_get_by_index(clk->dev, 0, &source);
	if (ret)
		return -EINVAL;

	parent_rate = clk_get_rate(&source);
	if ((parent_rate != rate) && ((parent_rate) / 2 != rate))
		return -EINVAL;

	if (parent_rate != rate) {
		writel((readl(&pmc->mckr) | AT91_PMC_MCKR_PLLADIV_2),
		       &pmc->mckr);
	}

	return 0;
}

static struct clk_ops at91_plladiv_clk_ops = {
	.enable = at91_plladiv_clk_enable,
	.get_rate = at91_plladiv_clk_get_rate,
	.set_rate = at91_plladiv_clk_set_rate,
};

static int at91_plladiv_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id at91_plladiv_clk_match[] = {
	{ .compatible = "atmel,at91sam9x5-clk-plldiv" },
	{}
};

U_BOOT_DRIVER(at91_plladiv_clk) = {
	.name = "at91-plladiv-clk",
	.id = UCLASS_CLK,
	.of_match = at91_plladiv_clk_match,
	.probe = at91_plladiv_clk_probe,
	.plat_auto	= sizeof(struct pmc_plat),
	.ops = &at91_plladiv_clk_ops,
};

/* System clock specific code. */
#define SYSTEM_MAX_ID		31

/**
 * at91_system_clk_bind() - for the system clock driver
 * Recursively bind its children as clk devices.
 *
 * @return: 0 on success, or negative error code on failure
 */
static int at91_system_clk_bind(struct udevice *dev)
{
	return at91_clk_sub_device_bind(dev, "system-clk");
}

static const struct udevice_id at91_system_clk_match[] = {
	{ .compatible = "atmel,at91rm9200-clk-system" },
	{}
};

U_BOOT_DRIVER(at91_system_clk) = {
	.name = "at91-system-clk",
	.id = UCLASS_MISC,
	.of_match = at91_system_clk_match,
	.bind = at91_system_clk_bind,
};

static inline int is_pck(int id)
{
	return (id >= 8) && (id <= 15);
}

static ulong system_clk_get_rate(struct clk *clk)
{
	struct clk clk_dev;
	int ret;

	ret = clk_get_by_index(clk->dev, 0, &clk_dev);
	if (ret)
		return -EINVAL;

	return clk_get_rate(&clk_dev);
}

static ulong system_clk_set_rate(struct clk *clk, ulong rate)
{
	struct clk clk_dev;
	int ret;

	ret = clk_get_by_index(clk->dev, 0, &clk_dev);
	if (ret)
		return -EINVAL;

	return clk_set_rate(&clk_dev, rate);
}

static int system_clk_enable(struct clk *clk)
{
	struct pmc_plat *plat = dev_get_plat(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	u32 mask;

	if (clk->id > SYSTEM_MAX_ID)
		return -EINVAL;

	mask = BIT(clk->id);

	writel(mask, &pmc->scer);

	/**
	 * For the programmable clocks the Ready status in the PMC
	 * status register should be checked after enabling.
	 * For other clocks this is unnecessary.
	 */
	if (!is_pck(clk->id))
		return 0;

	while (!(readl(&pmc->sr) & mask))
		;

	return 0;
}

static struct clk_ops system_clk_ops = {
	.of_xlate = at91_clk_of_xlate,
	.get_rate = system_clk_get_rate,
	.set_rate = system_clk_set_rate,
	.enable = system_clk_enable,
};

U_BOOT_DRIVER(system_clk) = {
	.name = "system-clk",
	.id = UCLASS_CLK,
	.probe = at91_clk_probe,
	.plat_auto	= sizeof(struct pmc_plat),
	.ops = &system_clk_ops,
};

/* Peripheral clock specific code. */
#define PERIPHERAL_ID_MIN	2
#define PERIPHERAL_ID_MAX	31
#define PERIPHERAL_MASK(id)	(1 << ((id) & PERIPHERAL_ID_MAX))

enum periph_clk_type {
	CLK_PERIPH_AT91RM9200 = 0,
	CLK_PERIPH_AT91SAM9X5,
};

/**
 * sam9x5_periph_clk_bind() - for the periph clock driver
 * Recursively bind its children as clk devices.
 *
 * @return: 0 on success, or negative error code on failure
 */
static int sam9x5_periph_clk_bind(struct udevice *dev)
{
	return at91_clk_sub_device_bind(dev, "periph-clk");
}

static const struct udevice_id sam9x5_periph_clk_match[] = {
	{
		.compatible = "atmel,at91rm9200-clk-peripheral",
		.data = CLK_PERIPH_AT91RM9200,
	},
	{
		.compatible = "atmel,at91sam9x5-clk-peripheral",
		.data = CLK_PERIPH_AT91SAM9X5,
	},
	{}
};

U_BOOT_DRIVER(sam9x5_periph_clk) = {
	.name = "sam9x5-periph-clk",
	.id = UCLASS_MISC,
	.of_match = sam9x5_periph_clk_match,
	.bind = sam9x5_periph_clk_bind,
};

static int periph_clk_enable(struct clk *clk)
{
	struct pmc_plat *plat = dev_get_plat(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	enum periph_clk_type clk_type;
	void *addr;

	if (clk->id < PERIPHERAL_ID_MIN)
		return -1;

	clk_type = dev_get_driver_data(dev_get_parent(clk->dev));
	if (clk_type == CLK_PERIPH_AT91RM9200) {
		addr = &pmc->pcer;
		if (clk->id > PERIPHERAL_ID_MAX)
			addr = &pmc->pcer1;

		setbits_le32(addr, PERIPHERAL_MASK(clk->id));
	} else {
		writel(clk->id & AT91_PMC_PCR_PID_MASK, &pmc->pcr);
		setbits_le32(&pmc->pcr,
			     AT91_PMC_PCR_CMD_WRITE | AT91_PMC_PCR_EN);
	}

	return 0;
}

static ulong periph_get_rate(struct clk *clk)
{
	struct udevice *dev;
	struct clk clk_dev;
	int ret;

	dev = dev_get_parent(clk->dev);

	ret = clk_get_by_index(dev, 0, &clk_dev);
	if (ret)
		return ret;

	return clk_get_rate(&clk_dev);
}

static struct clk_ops periph_clk_ops = {
	.of_xlate = at91_clk_of_xlate,
	.enable = periph_clk_enable,
	.get_rate = periph_get_rate,
};

U_BOOT_DRIVER(clk_periph) = {
	.name	= "periph-clk",
	.id	= UCLASS_CLK,
	.plat_auto	= sizeof(struct pmc_plat),
	.probe = at91_clk_probe,
	.ops	= &periph_clk_ops,
};

/* UTMI clock specific code. */
#ifdef CONFIG_AT91_UTMI

/*
 * The purpose of this clock is to generate a 480 MHz signal. A different
 * rate can't be configured.
 */
#define UTMI_RATE	480000000

static int utmi_clk_enable(struct clk *clk)
{
	struct pmc_plat *plat = dev_get_plat(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct clk clk_dev;
	ulong clk_rate;
	u32 utmi_ref_clk_freq;
	u32 tmp;
	int err;
	int timeout = 2000000;

	if (readl(&pmc->sr) & AT91_PMC_LOCKU)
		return 0;

	/*
	 * If mainck rate is different from 12 MHz, we have to configure the
	 * FREQ field of the SFR_UTMICKTRIM register to generate properly
	 * the utmi clock.
	 */
	err = clk_get_by_index(clk->dev, 0, &clk_dev);
	if (err)
		return -EINVAL;

	clk_rate = clk_get_rate(&clk_dev);
	switch (clk_rate) {
	case 12000000:
		utmi_ref_clk_freq = 0;
		break;
	case 16000000:
		utmi_ref_clk_freq = 1;
		break;
	case 24000000:
		utmi_ref_clk_freq = 2;
		break;
	/*
	 * Not supported on SAMA5D2 but it's not an issue since MAINCK
	 * maximum value is 24 MHz.
	 */
	case 48000000:
		utmi_ref_clk_freq = 3;
		break;
	default:
		printf("UTMICK: unsupported mainck rate\n");
		return -EINVAL;
	}

	if (plat->regmap_sfr) {
		err = regmap_read(plat->regmap_sfr, AT91_SFR_UTMICKTRIM, &tmp);
		if (err)
			return -EINVAL;

		tmp &= ~AT91_UTMICKTRIM_FREQ;
		tmp |= utmi_ref_clk_freq;
		err = regmap_write(plat->regmap_sfr, AT91_SFR_UTMICKTRIM, tmp);
		if (err)
			return -EINVAL;
	} else if (utmi_ref_clk_freq) {
		printf("UTMICK: sfr node required\n");
		return -EINVAL;
	}

	tmp = readl(&pmc->uckr);
	tmp |= AT91_PMC_UPLLEN |
	       AT91_PMC_UPLLCOUNT |
	       AT91_PMC_BIASEN;
	writel(tmp, &pmc->uckr);

	while ((--timeout) && !(readl(&pmc->sr) & AT91_PMC_LOCKU))
		;
	if (!timeout) {
		printf("UTMICK: timeout waiting for UPLL lock\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static ulong utmi_clk_get_rate(struct clk *clk)
{
	/* UTMI clk rate is fixed. */
	return UTMI_RATE;
}

static struct clk_ops utmi_clk_ops = {
	.enable = utmi_clk_enable,
	.get_rate = utmi_clk_get_rate,
};

static int utmi_clk_of_to_plat(struct udevice *dev)
{
	struct pmc_plat *plat = dev_get_plat(dev);
	struct udevice *syscon;

	uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
				     "regmap-sfr", &syscon);

	if (syscon)
		plat->regmap_sfr = syscon_get_regmap(syscon);

	return 0;
}

static int utmi_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id utmi_clk_match[] = {
	{ .compatible = "atmel,at91sam9x5-clk-utmi" },
	{}
};

U_BOOT_DRIVER(at91sam9x5_utmi_clk) = {
	.name = "at91sam9x5-utmi-clk",
	.id = UCLASS_CLK,
	.of_match = utmi_clk_match,
	.probe = utmi_clk_probe,
	.of_to_plat = utmi_clk_of_to_plat,
	.plat_auto	= sizeof(struct pmc_plat),
	.ops = &utmi_clk_ops,
};

#endif /* CONFIG_AT91_UTMI */

/* H32MX clock specific code. */
#ifdef CONFIG_AT91_H32MX

#define H32MX_MAX_FREQ	90000000

static ulong sama5d4_h32mx_clk_get_rate(struct clk *clk)
{
	struct pmc_plat *plat = dev_get_plat(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	ulong rate = gd->arch.mck_rate_hz;

	if (readl(&pmc->mckr) & AT91_PMC_MCKR_H32MXDIV)
		rate /= 2;

	if (rate > H32MX_MAX_FREQ)
		dev_dbg(clk->dev, "H32MX clock is too fast\n");

	return rate;
}

static struct clk_ops sama5d4_h32mx_clk_ops = {
	.get_rate = sama5d4_h32mx_clk_get_rate,
};

static int sama5d4_h32mx_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id sama5d4_h32mx_clk_match[] = {
	{ .compatible = "atmel,sama5d4-clk-h32mx" },
	{}
};

U_BOOT_DRIVER(sama5d4_h32mx_clk) = {
	.name = "sama5d4-h32mx-clk",
	.id = UCLASS_CLK,
	.of_match = sama5d4_h32mx_clk_match,
	.probe = sama5d4_h32mx_clk_probe,
	.plat_auto	= sizeof(struct pmc_plat),
	.ops = &sama5d4_h32mx_clk_ops,
};

#endif /* CONFIG_AT91_H32MX */

/* Generic clock specific code. */
#ifdef CONFIG_AT91_GENERIC_CLK

#define GENERATED_SOURCE_MAX	6
#define GENERATED_MAX_DIV	255

/**
 * generated_clk_bind() - for the generated clock driver
 * Recursively bind its children as clk devices.
 *
 * @return: 0 on success, or negative error code on failure
 */
static int generated_clk_bind(struct udevice *dev)
{
	return at91_clk_sub_device_bind(dev, "generic-clk");
}

static const struct udevice_id generated_clk_match[] = {
	{ .compatible = "atmel,sama5d2-clk-generated" },
	{}
};

U_BOOT_DRIVER(generated_clk) = {
	.name = "generated-clk",
	.id = UCLASS_MISC,
	.of_match = generated_clk_match,
	.bind = generated_clk_bind,
};

struct generic_clk_priv {
	u32 num_parents;
};

static ulong generic_clk_get_rate(struct clk *clk)
{
	struct pmc_plat *plat = dev_get_plat(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct clk parent;
	u32 tmp, gckdiv;
	u8 clock_source, parent_index;
	int ret;

	writel(clk->id & AT91_PMC_PCR_PID_MASK, &pmc->pcr);
	tmp = readl(&pmc->pcr);
	clock_source = (tmp >> AT91_PMC_PCR_GCKCSS_OFFSET) &
		    AT91_PMC_PCR_GCKCSS_MASK;
	gckdiv = (tmp >> AT91_PMC_PCR_GCKDIV_OFFSET) & AT91_PMC_PCR_GCKDIV_MASK;

	parent_index = clock_source - 1;
	ret = clk_get_by_index(dev_get_parent(clk->dev), parent_index, &parent);
	if (ret)
		return 0;

	return clk_get_rate(&parent) / (gckdiv + 1);
}

static ulong generic_clk_set_rate(struct clk *clk, ulong rate)
{
	struct pmc_plat *plat = dev_get_plat(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct generic_clk_priv *priv = dev_get_priv(clk->dev);
	struct clk parent, best_parent;
	ulong tmp_rate, best_rate = rate, parent_rate;
	int tmp_diff, best_diff = -1;
	u32 div, best_div = 0;
	u8 best_parent_index, best_clock_source = 0;
	u8 i;
	u32 tmp;
	int ret;

	for (i = 0; i < priv->num_parents; i++) {
		ret = clk_get_by_index(dev_get_parent(clk->dev), i, &parent);
		if (ret)
			return ret;

		parent_rate = clk_get_rate(&parent);
		if (IS_ERR_VALUE(parent_rate))
			return parent_rate;

		for (div = 1; div < GENERATED_MAX_DIV + 2; div++) {
			tmp_rate = DIV_ROUND_CLOSEST(parent_rate, div);
			tmp_diff = abs(rate - tmp_rate);

			if (best_diff < 0 || best_diff > tmp_diff) {
				best_rate = tmp_rate;
				best_diff = tmp_diff;

				best_div = div - 1;
				best_parent = parent;
				best_parent_index = i;
				best_clock_source = best_parent_index + 1;
			}

			if (!best_diff || tmp_rate < rate)
				break;
		}

		if (!best_diff)
			break;
	}

	debug("GCK: best parent: %s, best_rate = %ld, best_div = %d\n",
	      best_parent.dev->name, best_rate, best_div);

	ret = clk_enable(&best_parent);
	if (ret)
		return ret;

	writel(clk->id & AT91_PMC_PCR_PID_MASK, &pmc->pcr);
	tmp = readl(&pmc->pcr);
	tmp &= ~(AT91_PMC_PCR_GCKDIV | AT91_PMC_PCR_GCKCSS);
	tmp |= AT91_PMC_PCR_GCKCSS_(best_clock_source) |
	       AT91_PMC_PCR_CMD_WRITE |
	       AT91_PMC_PCR_GCKDIV_(best_div) |
	       AT91_PMC_PCR_GCKEN;
	writel(tmp, &pmc->pcr);

	while (!(readl(&pmc->sr) & AT91_PMC_GCKRDY))
		;

	return 0;
}

static struct clk_ops generic_clk_ops = {
	.of_xlate = at91_clk_of_xlate,
	.get_rate = generic_clk_get_rate,
	.set_rate = generic_clk_set_rate,
};

static int generic_clk_of_to_plat(struct udevice *dev)
{
	struct generic_clk_priv *priv = dev_get_priv(dev);
	u32 cells[GENERATED_SOURCE_MAX];
	u32 num_parents;

	num_parents = fdtdec_get_int_array_count(gd->fdt_blob,
			dev_of_offset(dev_get_parent(dev)), "clocks", cells,
			GENERATED_SOURCE_MAX);

	if (!num_parents)
		return -1;

	priv->num_parents = num_parents;

	return 0;
}

U_BOOT_DRIVER(generic_clk) = {
	.name = "generic-clk",
	.id = UCLASS_CLK,
	.probe = at91_clk_probe,
	.of_to_plat = generic_clk_of_to_plat,
	.priv_auto	= sizeof(struct generic_clk_priv),
	.plat_auto	= sizeof(struct pmc_plat),
	.ops = &generic_clk_ops,
};

#endif /* CONFIG_AT91_GENERIC_CLK */

/* USB clock specific code. */
#ifdef CONFIG_AT91_USB_CLK

#define AT91_USB_CLK_SOURCE_MAX	2
#define AT91_USB_CLK_MAX_DIV	15

struct at91_usb_clk_priv {
	u32 num_clksource;
};

static ulong at91_usb_clk_get_rate(struct clk *clk)
{
	struct pmc_plat *plat = dev_get_plat(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct clk source;
	u32 tmp, usbdiv;
	u8 source_index;
	int ret;

	tmp = readl(&pmc->pcr);
	source_index = (tmp >> AT91_PMC_USB_USBS_OFFSET) &
			AT91_PMC_USB_USBS_MASK;
	usbdiv = (tmp >> AT91_PMC_USB_DIV_OFFSET) & AT91_PMC_USB_DIV_MASK;

	ret = clk_get_by_index(clk->dev, source_index, &source);
	if (ret)
		return 0;

	return clk_get_rate(&source) / (usbdiv + 1);
}

static ulong at91_usb_clk_set_rate(struct clk *clk, ulong rate)
{
	struct pmc_plat *plat = dev_get_plat(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct at91_usb_clk_priv *priv = dev_get_priv(clk->dev);
	struct clk source, best_source;
	ulong tmp_rate, best_rate = rate, source_rate;
	int tmp_diff, best_diff = -1;
	u32 div, best_div = 0;
	u8 best_source_index = 0;
	u8 i;
	u32 tmp;
	int ret;

	for (i = 0; i < priv->num_clksource; i++) {
		ret = clk_get_by_index(clk->dev, i, &source);
		if (ret)
			return ret;

		source_rate = clk_get_rate(&source);
		if (IS_ERR_VALUE(source_rate))
			return source_rate;

		for (div = 1; div < AT91_USB_CLK_MAX_DIV + 2; div++) {
			tmp_rate = DIV_ROUND_CLOSEST(source_rate, div);
			tmp_diff = abs(rate - tmp_rate);

			if (best_diff < 0 || best_diff > tmp_diff) {
				best_rate = tmp_rate;
				best_diff = tmp_diff;

				best_div = div - 1;
				best_source = source;
				best_source_index = i;
			}

			if (!best_diff || tmp_rate < rate)
				break;
		}

		if (!best_diff)
			break;
	}

	debug("AT91 USB: best sourc: %s, best_rate = %ld, best_div = %d\n",
	      best_source.dev->name, best_rate, best_div);

	ret = clk_enable(&best_source);
	if (ret)
		return ret;

	tmp = AT91_PMC_USB_USBS_(best_source_index) |
	      AT91_PMC_USB_DIV_(best_div);
	writel(tmp, &pmc->usb);

	return 0;
}

static struct clk_ops at91_usb_clk_ops = {
	.get_rate = at91_usb_clk_get_rate,
	.set_rate = at91_usb_clk_set_rate,
};

static int at91_usb_clk_of_to_plat(struct udevice *dev)
{
	struct at91_usb_clk_priv *priv = dev_get_priv(dev);
	u32 cells[AT91_USB_CLK_SOURCE_MAX];
	u32 num_clksource;

	num_clksource = fdtdec_get_int_array_count(gd->fdt_blob,
						   dev_of_offset(dev),
						   "clocks", cells,
						   AT91_USB_CLK_SOURCE_MAX);

	if (!num_clksource)
		return -1;

	priv->num_clksource = num_clksource;

	return 0;
}

static int at91_usb_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id at91_usb_clk_match[] = {
	{ .compatible = "atmel,at91sam9x5-clk-usb" },
	{}
};

U_BOOT_DRIVER(at91_usb_clk) = {
	.name = "at91-usb-clk",
	.id = UCLASS_CLK,
	.of_match = at91_usb_clk_match,
	.probe = at91_usb_clk_probe,
	.of_to_plat = at91_usb_clk_of_to_plat,
	.priv_auto	= sizeof(struct at91_usb_clk_priv),
	.plat_auto	= sizeof(struct pmc_plat),
	.ops = &at91_usb_clk_ops,
};

#endif /* CONFIG_AT91_USB_CLK */
