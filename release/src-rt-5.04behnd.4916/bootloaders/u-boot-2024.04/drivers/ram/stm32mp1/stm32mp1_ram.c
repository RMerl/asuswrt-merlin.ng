// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_RAM

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <ram.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/printk.h>
#include "stm32mp1_ddr.h"
#include "stm32mp1_ddr_regs.h"

/* DDR subsystem configuration */
struct stm32mp1_ddr_cfg {
	u8 nb_bytes;	/* MEMC_DRAM_DATA_WIDTH */
};

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
	unsigned int idx;

	for (idx = 0; idx < ARRAY_SIZE(clkname); idx++) {
		ret = clk_get_by_name(priv->dev, clkname[idx], &clk);

		if (!ret)
			ret = clk_enable(&clk);

		if (ret) {
			log_err("error for %s : %d\n", clkname[idx], ret);
			return ret;
		}
	}

	priv->clk = clk;
	ddrphy_clk = clk_get_rate(&priv->clk);

	log_debug("DDR: mem_speed (%d kHz), RCC %d kHz\n",
		  mem_speed, (u32)(ddrphy_clk / 1000));
	/* max 10% frequency delta */
	ddr_clk = abs(ddrphy_clk - mem_speed * 1000);
	if (ddr_clk > (mem_speed * 100)) {
		log_err("DDR expected freq %d kHz, current is %d kHz\n",
			mem_speed, (u32)(ddrphy_clk / 1000));
		return -EINVAL;
	}

	return 0;
}

__weak int board_stm32mp1_ddr_config_name_match(struct udevice *dev,
						const char *name)
{
	return 0;	/* Always match */
}

static ofnode stm32mp1_ddr_get_ofnode(struct udevice *dev)
{
	const char *name;
	ofnode node;

	dev_for_each_subnode(node, dev) {
		name = ofnode_get_property(node, "compatible", NULL);

		if (!board_stm32mp1_ddr_config_name_match(dev, name))
			return node;
	}

	return dev_ofnode(dev);
}

static int stm32mp1_ddr_setup(struct udevice *dev)
{
	struct ddr_info *priv = dev_get_priv(dev);
	int ret;
	unsigned int idx;
	struct clk axidcg;
	struct stm32mp1_ddr_config config;
	ofnode node = stm32mp1_ddr_get_ofnode(dev);

#define PARAM(x, y, z)							\
	{	.name = x,						\
		.offset = offsetof(struct stm32mp1_ddr_config, y),	\
		.size = sizeof(config.y) / sizeof(u32),			\
	}

#define CTL_PARAM(x) PARAM("st,ctl-"#x, c_##x, NULL)
#define PHY_PARAM(x) PARAM("st,phy-"#x, p_##x, NULL)

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
		PHY_PARAM(timing)
	};

	config.info.speed = ofnode_read_u32_default(node, "st,mem-speed", 0);
	config.info.size = ofnode_read_u32_default(node, "st,mem-size", 0);
	config.info.name = ofnode_read_string(node, "st,mem-name");
	if (!config.info.name) {
		dev_dbg(dev, "no st,mem-name\n");
		return -EINVAL;
	}
	if (CONFIG_IS_ENABLED(DISPLAY_PRINT))
		printf("RAM: %s\n", config.info.name);

	for (idx = 0; idx < ARRAY_SIZE(param); idx++) {
		ret = ofnode_read_u32_array(node, param[idx].name,
					 (void *)((u32)&config +
						  param[idx].offset),
					 param[idx].size);
		dev_dbg(dev, "%s: %s[0x%x] = %d\n", __func__,
			param[idx].name, param[idx].size, ret);
		if (ret) {
			dev_err(dev, "Cannot read %s, error=%d\n",
				param[idx].name, ret);
			return -EINVAL;
		}
	}

	ret = clk_get_by_name(dev, "axidcg", &axidcg);
	if (ret) {
		dev_dbg(dev, "%s: Cannot found axidcg\n", __func__);
		return -EINVAL;
	}
	clk_disable(&axidcg); /* disable clock gating during init */

	stm32mp1_ddr_init(priv, &config);

	clk_enable(&axidcg); /* enable clock gating */

	/* check size */
	dev_dbg(dev, "get_ram_size(%x, %x)\n",
		(u32)priv->info.base, (u32)STM32_DDR_SIZE);

	priv->info.size = get_ram_size((long *)priv->info.base,
				       STM32_DDR_SIZE);

	dev_dbg(dev, "info.size: %x\n", (u32)priv->info.size);

	/* check memory access for all memory */
	if (config.info.size != priv->info.size) {
		printf("DDR invalid size : 0x%x, expected 0x%x\n",
		       priv->info.size, config.info.size);
		return -EINVAL;
	}
	return 0;
}

static u8 get_data_bus_width(struct stm32mp1_ddrctl *ctl)
{
	u32 reg = readl(&ctl->mstr) & DDRCTRL_MSTR_DATA_BUS_WIDTH_MASK;
	u8 data_bus_width = reg >> DDRCTRL_MSTR_DATA_BUS_WIDTH_SHIFT;

	return data_bus_width;
}

static u8 get_nb_bank(struct stm32mp1_ddrctl *ctl)
{
	/* Count bank address bits */
	u8 bits = 0;
	u32 reg, val;

	reg = readl(&ctl->addrmap1);
	/* addrmap1.addrmap_bank_b1 */
	val = (reg & GENMASK(5, 0)) >> 0;
	if (val <= 31)
		bits++;
	/* addrmap1.addrmap_bank_b2 */
	val = (reg & GENMASK(13, 8)) >> 8;
	if (val <= 31)
		bits++;
	/* addrmap1.addrmap_bank_b3 */
	val = (reg & GENMASK(21, 16)) >> 16;
	if (val <= 31)
		bits++;

	return bits;
}

static u8 get_nb_col(struct stm32mp1_ddrctl *ctl, u8 data_bus_width)
{
	u8 bits;
	u32 reg, val;

	/* Count column address bits, start at 2 for b0 and b1 (fixed) */
	bits = 2;

	reg = readl(&ctl->addrmap2);
	/* addrmap2.addrmap_col_b2 */
	val = (reg & GENMASK(3, 0)) >> 0;
	if (val <= 7)
		bits++;
	/* addrmap2.addrmap_col_b3 */
	val = (reg & GENMASK(11, 8)) >> 8;
	if (val <= 7)
		bits++;
	/* addrmap2.addrmap_col_b4 */
	val = (reg & GENMASK(19, 16)) >> 16;
	if (val <= 7)
		bits++;
	/* addrmap2.addrmap_col_b5 */
	val = (reg & GENMASK(27, 24)) >> 24;
	if (val <= 7)
		bits++;

	reg = readl(&ctl->addrmap3);
	/* addrmap3.addrmap_col_b6 */
	val = (reg & GENMASK(4, 0)) >> 0;
	if (val <= 7)
		bits++;
	/* addrmap3.addrmap_col_b7 */
	val = (reg & GENMASK(12, 8)) >> 8;
	if (val <= 7)
		bits++;
	/* addrmap3.addrmap_col_b8 */
	val = (reg & GENMASK(20, 16)) >> 16;
	if (val <= 7)
		bits++;
	/* addrmap3.addrmap_col_b9 */
	val = (reg & GENMASK(28, 24)) >> 24;
	if (val <= 7)
		bits++;

	reg = readl(&ctl->addrmap4);
	/* addrmap4.addrmap_col_b10 */
	val = (reg & GENMASK(4, 0)) >> 0;
	if (val <= 7)
		bits++;
	/* addrmap4.addrmap_col_b11 */
	val = (reg & GENMASK(12, 8)) >> 8;
	if (val <= 7)
		bits++;

	/*
	 * column bits shift up:
	 * 1 when half the data bus is used (data_bus_width = 1)
	 * 2 when a quarter the data bus is used (data_bus_width = 2)
	 * nothing to do for full data bus (data_bus_width = 0)
	 */
	bits += data_bus_width;

	return bits;
}

static u8 get_nb_row(struct stm32mp1_ddrctl *ctl)
{
	/* Count row address bits */
	u8 bits = 0;
	u32 reg, val;

	reg = readl(&ctl->addrmap5);
	/* addrmap5.addrmap_row_b0 */
	val = (reg & GENMASK(3, 0)) >> 0;
	if (val <= 11)
		bits++;
	/* addrmap5.addrmap_row_b1 */
	val = (reg & GENMASK(11, 8)) >> 8;
	if (val <= 11)
		bits++;
	/* addrmap5.addrmap_row_b2_10 */
	val = (reg & GENMASK(19, 16)) >> 16;
	if (val <= 11)
		bits += 9;
	else
		printf("warning: addrmap5.addrmap_row_b2_10 not supported\n");
	/* addrmap5.addrmap_row_b11 */
	val = (reg & GENMASK(27, 24)) >> 24;
	if (val <= 11)
		bits++;

	reg = readl(&ctl->addrmap6);
	/* addrmap6.addrmap_row_b12 */
	val = (reg & GENMASK(3, 0)) >> 0;
	if (val <= 11)
		bits++;
	/* addrmap6.addrmap_row_b13 */
	val = (reg & GENMASK(11, 8)) >> 8;
	if (val <= 11)
		bits++;
	/* addrmap6.addrmap_row_b14 */
	val = (reg & GENMASK(19, 16)) >> 16;
	if (val <= 11)
		bits++;
	/* addrmap6.addrmap_row_b15 */
	val = (reg & GENMASK(27, 24)) >> 24;
	if (val <= 11)
		bits++;

	if (reg & BIT(31))
		printf("warning: LPDDR3_6GB_12GB is not supported\n");

	return bits;
}

/*
 * stm32mp1_ddr_size
 *
 * Get the current DRAM size from the DDR CTL registers
 *
 * @return: DRAM size
 */
u32 stm32mp1_ddr_size(struct udevice *dev)
{
	u8 nb_bit;
	u32 ddr_size;
	u8 data_bus_width;
	struct ddr_info *priv = dev_get_priv(dev);
	struct stm32mp1_ddrctl *ctl = priv->ctl;
	struct stm32mp1_ddr_cfg *cfg = (struct stm32mp1_ddr_cfg *)dev_get_driver_data(dev);
	const u8 nb_bytes = cfg->nb_bytes;

	data_bus_width = get_data_bus_width(ctl);
	nb_bit = get_nb_bank(ctl) + get_nb_col(ctl, data_bus_width) +
		 get_nb_row(ctl);
	if (nb_bit > 32) {
		nb_bit = 32;
		debug("invalid DDR configuration: %d bits\n", nb_bit);
	}

	ddr_size = (nb_bytes >> data_bus_width) << nb_bit;
	if (ddr_size > STM32_DDR_SIZE) {
		ddr_size = STM32_DDR_SIZE;
		debug("invalid DDR configuration: size = %x\n", ddr_size);
	}

	return ddr_size;
}

static int stm32mp1_ddr_probe(struct udevice *dev)
{
	struct ddr_info *priv = dev_get_priv(dev);
	struct regmap *map;
	int ret;

	priv->dev = dev;

	ret = regmap_init_mem(dev_ofnode(dev), &map);
	if (ret)
		return log_ret(ret);

	priv->ctl = regmap_get_range(map, 0);
	priv->phy = regmap_get_range(map, 1);

	priv->rcc = STM32_RCC_BASE;

	priv->info.base = STM32_DDR_BASE;

	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		priv->info.size = 0;
		ret = stm32mp1_ddr_setup(dev);

		return log_ret(ret);
	}

	priv->info.size = stm32mp1_ddr_size(dev);

	return 0;
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

static const struct stm32mp1_ddr_cfg stm32mp13x_ddr_cfg = {
	.nb_bytes = 2,
};

static const struct stm32mp1_ddr_cfg stm32mp15x_ddr_cfg = {
	.nb_bytes = 4,
};

static const struct udevice_id stm32mp1_ddr_ids[] = {
	{ .compatible = "st,stm32mp1-ddr", .data = (ulong)&stm32mp15x_ddr_cfg},
	{ .compatible = "st,stm32mp13-ddr", .data = (ulong)&stm32mp13x_ddr_cfg},
	{ }
};

U_BOOT_DRIVER(ddr_stm32mp1) = {
	.name = "stm32mp1_ddr",
	.id = UCLASS_RAM,
	.of_match = stm32mp1_ddr_ids,
	.ops = &stm32mp1_ddr_ops,
	.probe = stm32mp1_ddr_probe,
	.priv_auto	= sizeof(struct ddr_info),
};
