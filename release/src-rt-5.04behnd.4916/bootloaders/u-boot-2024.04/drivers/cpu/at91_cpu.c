// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <div64.h>
#include <linux/clk-provider.h>

struct at91_cpu_plat {
	const char *name;
	ulong cpufreq_mhz;
	ulong mckfreq_mhz;
	ulong xtalfreq_mhz;
};

extern char *get_cpu_name(void);

const char *at91_cpu_get_name(void)
{
	return get_cpu_name();
}

int at91_cpu_get_desc(const struct udevice *dev, char *buf, int size)
{
	struct at91_cpu_plat *plat = dev_get_plat(dev);

	snprintf(buf, size, "%s\n"
		 "Crystal frequency: %8lu MHz\n"
		 "CPU clock        : %8lu MHz\n"
		 "Master clock     : %8lu MHz\n",
		 plat->name, plat->xtalfreq_mhz, plat->cpufreq_mhz,
		 plat->mckfreq_mhz);

	return 0;
}

static int at91_cpu_get_info(const struct udevice *dev, struct cpu_info *info)
{
	struct at91_cpu_plat *plat = dev_get_plat(dev);

	info->cpu_freq = plat->cpufreq_mhz * 1000000;
	info->features = BIT(CPU_FEAT_L1_CACHE);

	return 0;
}

static int at91_cpu_get_count(const struct udevice *dev)
{
	return 1;
}

static int at91_cpu_get_vendor(const struct udevice *dev,  char *buf, int size)
{
	snprintf(buf, size, "Microchip Technology Inc.");

	return 0;
}

static const struct cpu_ops at91_cpu_ops = {
	.get_desc	= at91_cpu_get_desc,
	.get_info	= at91_cpu_get_info,
	.get_count	= at91_cpu_get_count,
	.get_vendor	= at91_cpu_get_vendor,
};

static const struct udevice_id at91_cpu_ids[] = {
	{ .compatible = "arm,cortex-a7" },
	{ .compatible = "arm,arm926ej-s" },
	{ /* Sentinel. */ }
};

static int at91_cpu_probe(struct udevice *dev)
{
	struct at91_cpu_plat *plat = dev_get_plat(dev);
	struct clk clk;
	ulong rate;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	rate  = clk_get_rate(&clk);
	if (!rate)
		return -ENOTSUPP;
	plat->cpufreq_mhz = DIV_ROUND_CLOSEST_ULL(rate, 1000000);

	ret = clk_get_by_index(dev, 1, &clk);
	if (ret)
		return ret;

	rate = clk_get_rate(&clk);
	if (!rate)
		return -ENOTSUPP;
	plat->mckfreq_mhz = DIV_ROUND_CLOSEST_ULL(rate, 1000000);

	ret = clk_get_by_index(dev, 2, &clk);
	if (ret)
		return ret;

	rate = clk_get_rate(&clk);
	if (!rate)
		return -ENOTSUPP;
	plat->xtalfreq_mhz = DIV_ROUND_CLOSEST_ULL(rate, 1000000);

	plat->name = get_cpu_name();

	return 0;
}

U_BOOT_DRIVER(cpu_at91_drv) = {
	.name		= "at91-cpu",
	.id		= UCLASS_CPU,
	.of_match	= at91_cpu_ids,
	.ops		= &at91_cpu_ops,
	.probe		= at91_cpu_probe,
	.plat_auto	= sizeof(struct at91_cpu_plat),
	.flags		= DM_FLAG_PRE_RELOC,
};
