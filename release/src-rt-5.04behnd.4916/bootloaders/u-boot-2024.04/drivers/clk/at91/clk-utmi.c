// SPDX-License-Identifier: GPL-2.0+
/*
 * UTMI clock support for AT91 architectures.
 *
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 *
 * Based on drivers/clk/at91/clk-utmi.c from Linux.
 */
#include <asm/processor.h>
#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/clk-provider.h>
#include <linux/clk/at91_pmc.h>
#include <mach/at91_sfr.h>
#include <regmap.h>
#include <syscon.h>

#include "pmc.h"

#define UBOOT_DM_CLK_AT91_UTMI			"at91-utmi-clk"
#define UBOOT_DM_CLK_AT91_SAMA7G5_UTMI		"at91-sama7g5-utmi-clk"

/*
 * The purpose of this clock is to generate a 480 MHz signal. A different
 * rate can't be configured.
 */
#define UTMI_RATE	480000000

struct clk_utmi {
	void __iomem *base;
	struct regmap *regmap_sfr;
	struct clk clk;
};

#define to_clk_utmi(_clk) container_of(_clk, struct clk_utmi, clk)

static inline bool clk_utmi_ready(struct regmap *regmap)
{
	unsigned int status;

	pmc_read(regmap, AT91_PMC_SR, &status);

	return !!(status & AT91_PMC_LOCKU);
}

static int clk_utmi_enable(struct clk *clk)
{
	struct clk_utmi *utmi = to_clk_utmi(clk);
	unsigned int uckr = AT91_PMC_UPLLEN | AT91_PMC_UPLLCOUNT |
			    AT91_PMC_BIASEN;
	unsigned int utmi_ref_clk_freq;
	ulong parent_rate = clk_get_parent_rate(clk);

	/*
	 * If mainck rate is different from 12 MHz, we have to configure the
	 * FREQ field of the SFR_UTMICKTRIM register to generate properly
	 * the utmi clock.
	 */
	switch (parent_rate) {
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
		debug("UTMICK: unsupported mainck rate\n");
		return -EINVAL;
	}

	if (utmi->regmap_sfr) {
		regmap_update_bits(utmi->regmap_sfr, AT91_SFR_UTMICKTRIM,
				   AT91_UTMICKTRIM_FREQ, utmi_ref_clk_freq);
	} else if (utmi_ref_clk_freq) {
		debug("UTMICK: sfr node required\n");
		return -EINVAL;
	}

	pmc_update_bits(utmi->base, AT91_CKGR_UCKR, uckr, uckr);

	while (!clk_utmi_ready(utmi->base)) {
		debug("waiting for utmi...\n");
		cpu_relax();
	}

	return 0;
}

static int clk_utmi_disable(struct clk *clk)
{
	struct clk_utmi *utmi = to_clk_utmi(clk);

	pmc_update_bits(utmi->base, AT91_CKGR_UCKR, AT91_PMC_UPLLEN, 0);

	return 0;
}

static ulong clk_utmi_get_rate(struct clk *clk)
{
	/* UTMI clk rate is fixed. */
	return UTMI_RATE;
}

static const struct clk_ops utmi_ops = {
	.enable = clk_utmi_enable,
	.disable = clk_utmi_disable,
	.get_rate = clk_utmi_get_rate,
};

struct clk *at91_clk_register_utmi(void __iomem *base, struct udevice *dev,
				   const char *name, const char *parent_name)
{
	struct udevice *syscon;
	struct clk_utmi *utmi;
	struct clk *clk;
	int ret;

	if (!base || !dev || !name || !parent_name)
		return ERR_PTR(-EINVAL);

	ret = uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
					   "regmap-sfr", &syscon);
	if (ret)
		return ERR_PTR(ret);

	utmi = kzalloc(sizeof(*utmi), GFP_KERNEL);
	if (!utmi)
		return ERR_PTR(-ENOMEM);

	utmi->base = base;
	utmi->regmap_sfr = syscon_get_regmap(syscon);
	if (!utmi->regmap_sfr) {
		kfree(utmi);
		return ERR_PTR(-ENODEV);
	}

	clk = &utmi->clk;
	clk->flags = CLK_GET_RATE_NOCACHE;
	ret = clk_register(clk, UBOOT_DM_CLK_AT91_UTMI, name, parent_name);
	if (ret) {
		kfree(utmi);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_utmi_clk) = {
	.name = UBOOT_DM_CLK_AT91_UTMI,
	.id = UCLASS_CLK,
	.ops = &utmi_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static int clk_utmi_sama7g5_enable(struct clk *clk)
{
	struct clk_utmi *utmi = to_clk_utmi(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	unsigned int val;

	switch (parent_rate) {
	case 16000000:
		val = 0;
		break;
	case 20000000:
		val = 2;
		break;
	case 24000000:
		val = 3;
		break;
	case 32000000:
		val = 5;
		break;
	default:
		debug("UTMICK: unsupported main_xtal rate\n");
		return -EINVAL;
	}

	pmc_write(utmi->base, AT91_PMC_XTALF, val);

	return 0;
}

static const struct clk_ops sama7g5_utmi_ops = {
	.enable = clk_utmi_sama7g5_enable,
	.get_rate = clk_utmi_get_rate,
};

struct clk *at91_clk_sama7g5_register_utmi(void __iomem *base,
		const char *name, const char *parent_name)
{
	struct clk_utmi *utmi;
	struct clk *clk;
	int ret;

	if (!base || !name || !parent_name)
		return ERR_PTR(-EINVAL);

	utmi = kzalloc(sizeof(*utmi), GFP_KERNEL);
	if (!utmi)
		return ERR_PTR(-ENOMEM);

	utmi->base = base;

	clk = &utmi->clk;
	ret = clk_register(clk, UBOOT_DM_CLK_AT91_SAMA7G5_UTMI, name,
			   parent_name);
	if (ret) {
		kfree(utmi);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_sama7g5_utmi_clk) = {
	.name = UBOOT_DM_CLK_AT91_SAMA7G5_UTMI,
	.id = UCLASS_CLK,
	.ops = &sama7g5_utmi_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
