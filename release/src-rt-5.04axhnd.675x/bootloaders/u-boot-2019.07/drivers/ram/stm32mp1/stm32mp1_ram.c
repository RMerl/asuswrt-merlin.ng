// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <ram.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include "stm32mp1_ddr.h"

static const char *const clkname[] = {
	"ddrc1",
	"ddrc2",
	"ddrcapb",
	"ddrphycapb",
	"ddrphyc" /* LAST clock => used for get_rate() */
};

int stm32mp1_ddr_clk_enable(struct ddr_info *priv, uint32_t mem_speed)
{
	unsigned long ddrphy_clk;
	unsigned long ddr_clk;
	struct clk clk;
	int ret;
	int idx;

	for (idx = 0; idx < ARRAY_SIZE(clkname); idx++) {
		ret = clk_get_by_name(priv->dev, clkname[idx], &clk);

		if (!ret)
			ret = clk_enable(&clk);

		if (ret) {
			printf("error for %s : %d\n", clkname[idx], ret);
			return ret;
		}
	}

	priv->clk = clk;
	ddrphy_clk = clk_get_rate(&priv->clk);

	debug("DDR: mem_speed (%d kHz), RCC %d kHz\n",
	      mem_speed, (u32)(ddrphy_clk / 1000));
	/* max 10% frequency delta */
	ddr_clk = abs(ddrphy_clk - mem_speed * 1000);
	if (ddr_clk > (mem_speed * 100)) {
		pr_err("DDR expected freq %d kHz, current is %d kHz\n",
		       mem_speed, (u32)(ddrphy_clk / 1000));
		return -EINVAL;
	}

	return 0;
}

static __maybe_unused int stm32mp1_ddr_setup(struct udevice *dev)
{
	struct ddr_info *priv = dev_get_priv(dev);
	int ret, idx;
	struct clk axidcg;
	struct stm32mp1_ddr_config config;

#define PARAM(x, y) \
	{ x,\
	  offsetof(struct stm32mp1_ddr_config, y),\
	  sizeof(config.y) / sizeof(u32)}

#define CTL_PARAM(x) PARAM("st,ctl-"#x, c_##x)
#define PHY_PARAM(x) PARAM("st,phy-"#x, p_##x)

	const struct {
		const char *name; /* name in DT */
		const u32 offset; /* offset in config struct */
		const u32 size;   /* size of parameters */
	} param[] = {
		CTL_PARAM(reg),
		CTL_PARAM(timing),
		CTL_PARAM(map),
		CTL_PARAM(perf),
		PHY_PARAM(reg),
		PHY_PARAM(timing),
		PHY_PARAM(cal)
	};

	config.info.speed = dev_read_u32_default(dev, "st,mem-speed", 0);
	config.info.size = dev_read_u32_default(dev, "st,mem-size", 0);
	config.info.name = dev_read_string(dev, "st,mem-name");
	if (!config.info.name) {
		debug("%s: no st,mem-name\n", __func__);
		return -EINVAL;
	}
	printf("RAM: %s\n", config.info.name);

	for (idx = 0; idx < ARRAY_SIZE(param); idx++) {
		ret = dev_read_u32_array(dev, param[idx].name,
					 (void *)((u32)&config +
						  param[idx].offset),
					 param[idx].size);
		debug("%s: %s[0x%x] = %d\n", __func__,
		      param[idx].name, param[idx].size, ret);
		if (ret) {
			pr_err("%s: Cannot read %s, error=%d\n",
			       __func__, param[idx].name, ret);
			return -EINVAL;
		}
	}

	ret = clk_get_by_name(dev, "axidcg", &axidcg);
	if (ret) {
		debug("%s: Cannot found axidcg\n", __func__);
		return -EINVAL;
	}
	clk_disable(&axidcg); /* disable clock gating during init */

	stm32mp1_ddr_init(priv, &config);

	clk_enable(&axidcg); /* enable clock gating */

	/* check size */
	debug("%s : get_ram_size(%x, %x)\n", __func__,
	      (u32)priv->info.base, (u32)STM32_DDR_SIZE);

	priv->info.size = get_ram_size((long *)priv->info.base,
				       STM32_DDR_SIZE);

	debug("%s : %x\n", __func__, (u32)priv->info.size);

	/* check memory access for all memory */
	if (config.info.size != priv->info.size) {
		printf("DDR invalid size : 0x%x, expected 0x%x\n",
		       priv->info.size, config.info.size);
		return -EINVAL;
	}
	return 0;
}

static int stm32mp1_ddr_probe(struct udevice *dev)
{
	struct ddr_info *priv = dev_get_priv(dev);
	struct regmap *map;
	int ret;

	debug("STM32MP1 DDR probe\n");
	priv->dev = dev;

	ret = regmap_init_mem(dev_ofnode(dev), &map);
	if (ret)
		return ret;

	priv->ctl = regmap_get_range(map, 0);
	priv->phy = regmap_get_range(map, 1);

	priv->rcc = STM32_RCC_BASE;

	priv->info.base = STM32_DDR_BASE;

#if !defined(CONFIG_STM32MP1_TRUSTED) && \
	(!defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD))
	priv->info.size = 0;
	return stm32mp1_ddr_setup(dev);
#else
	priv->info.size = dev_read_u32_default(dev, "st,mem-size", 0);
	return 0;
#endif
}

static int stm32mp1_ddr_get_info(struct udevice *dev, struct ram_info *info)
{
	struct ddr_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops stm32mp1_ddr_ops = {
	.get_info = stm32mp1_ddr_get_info,
};

static const struct udevice_id stm32mp1_ddr_ids[] = {
	{ .compatible = "st,stm32mp1-ddr" },
	{ }
};

U_BOOT_DRIVER(ddr_stm32mp1) = {
	.name = "stm32mp1_ddr",
	.id = UCLASS_RAM,
	.of_match = stm32mp1_ddr_ids,
	.ops = &stm32mp1_ddr_ops,
	.probe = stm32mp1_ddr_probe,
	.priv_auto_alloc_size = sizeof(struct ddr_info),
};
