// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 * Author: Gabriel Fernandez <gabriel.fernandez@foss.st.com> for STMicroelectronics.
 */

#define LOG_CATEGORY UCLASS_CLK

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/clk-provider.h>
#include "clk-stm32-core.h"

int stm32_rcc_init(struct udevice *dev,
		   const struct stm32_clock_match_data *data)
{
	int i;
	u8 *cpt;
	struct stm32mp_rcc_priv *priv = dev_get_priv(dev);
	fdt_addr_t base = dev_read_addr(dev->parent);
	const struct clk_stm32_clock_data *clock_data = data->clock_data;

	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = (void __iomem *)base;

	/* allocate the counter of user for internal RCC gates, common for several user */
	cpt = kzalloc(clock_data->num_gates, GFP_KERNEL);
	if (!cpt)
		return -ENOMEM;

	priv->gate_cpt = cpt;

	priv->data = clock_data;

	for (i = 0; i < data->num_clocks; i++) {
		const struct clock_config *cfg = &data->tab_clocks[i];
		struct clk *clk = ERR_PTR(-ENOENT);

		if (data->check_security && data->check_security(priv->base, cfg))
			continue;

		if (cfg->setup) {
			clk = cfg->setup(dev, cfg);
			clk->id = cfg->id;
		} else {
			dev_err(dev, "failed to register clock %s\n", cfg->name);
			return -ENOENT;
		}
	}

	return 0;
}

ulong clk_stm32_get_rate_by_name(const char *name)
{
	struct udevice *dev;

	if (!uclass_get_device_by_name(UCLASS_CLK, name, &dev)) {
		struct clk *clk = dev_get_clk_ptr(dev);

		return clk_get_rate(clk);
	}

	return 0;
}

const struct clk_ops stm32_clk_ops = {
	.enable = ccf_clk_enable,
	.disable = ccf_clk_disable,
	.get_rate = ccf_clk_get_rate,
	.set_rate = ccf_clk_set_rate,
};

#define RCC_MP_ENCLRR_OFFSET	4

static void clk_stm32_gate_set_state(void __iomem *base,
				     const struct clk_stm32_clock_data *data,
				     u8 *cpt, u16 gate_id, int enable)
{
	const struct stm32_gate_cfg *gate_cfg = &data->gates[gate_id];
	void __iomem *addr = base + gate_cfg->reg_off;
	u8 set_clr = gate_cfg->set_clr ? RCC_MP_ENCLRR_OFFSET : 0;

	if (enable) {
		if (cpt[gate_id]++ > 0)
			return;

		if (set_clr)
			writel(BIT(gate_cfg->bit_idx), addr);
		else
			writel(readl(addr) | BIT(gate_cfg->bit_idx), addr);
	} else {
		if (--cpt[gate_id] > 0)
			return;

		if (set_clr)
			writel(BIT(gate_cfg->bit_idx), addr + set_clr);
		else
			writel(readl(addr) & ~BIT(gate_cfg->bit_idx), addr);
	}
}

static int clk_stm32_gate_enable(struct clk *clk)
{
	struct clk_stm32_gate *stm32_gate = to_clk_stm32_gate(clk);
	struct stm32mp_rcc_priv *priv = stm32_gate->priv;

	clk_stm32_gate_set_state(priv->base, priv->data, priv->gate_cpt,
				 stm32_gate->gate_id, 1);

	return 0;
}

static int clk_stm32_gate_disable(struct clk *clk)
{
	struct clk_stm32_gate *stm32_gate = to_clk_stm32_gate(clk);
	struct stm32mp_rcc_priv *priv = stm32_gate->priv;

	clk_stm32_gate_set_state(priv->base, priv->data, priv->gate_cpt,
				 stm32_gate->gate_id, 0);

	return 0;
}

static const struct clk_ops clk_stm32_gate_ops = {
	.enable = clk_stm32_gate_enable,
	.disable = clk_stm32_gate_disable,
	.get_rate = clk_generic_get_rate,
};

#define UBOOT_DM_CLK_STM32_GATE "clk_stm32_gate"

U_BOOT_DRIVER(clk_stm32_gate) = {
	.name	= UBOOT_DM_CLK_STM32_GATE,
	.id	= UCLASS_CLK,
	.ops	= &clk_stm32_gate_ops,
};

struct clk *clk_stm32_gate_register(struct udevice *dev,
				    const struct clock_config *cfg)
{
	struct stm32mp_rcc_priv *priv = dev_get_priv(dev);
	struct stm32_clk_gate_cfg *clk_cfg = cfg->clock_cfg;
	struct clk_stm32_gate *stm32_gate;
	struct clk *clk;
	int ret;

	stm32_gate = kzalloc(sizeof(*stm32_gate), GFP_KERNEL);
	if (!stm32_gate)
		return ERR_PTR(-ENOMEM);

	stm32_gate->priv = priv;
	stm32_gate->gate_id = clk_cfg->gate_id;

	clk = &stm32_gate->clk;
	clk->flags = cfg->flags;

	ret = clk_register(clk, UBOOT_DM_CLK_STM32_GATE,
			   cfg->name, cfg->parent_name);
	if (ret) {
		kfree(stm32_gate);
		return ERR_PTR(ret);
	}

	return clk;
}

struct clk *
clk_stm32_register_composite(struct udevice *dev,
			     const struct clock_config *cfg)
{
	struct stm32_clk_composite_cfg *composite = cfg->clock_cfg;
	const char *const *parent_names;
	int num_parents;
	struct clk *clk = ERR_PTR(-ENOMEM);
	struct clk_mux *mux = NULL;
	struct clk_stm32_gate *gate = NULL;
	struct clk_divider *div = NULL;
	struct clk *mux_clk = NULL;
	const struct clk_ops *mux_ops = NULL;
	struct clk *gate_clk = NULL;
	const struct clk_ops *gate_ops = NULL;
	struct clk *div_clk = NULL;
	const struct clk_ops *div_ops = NULL;
	struct stm32mp_rcc_priv *priv = dev_get_priv(dev);
	const struct clk_stm32_clock_data *data = priv->data;

	if  (composite->mux_id != NO_STM32_MUX) {
		const struct stm32_mux_cfg *mux_cfg;

		mux = kzalloc(sizeof(*mux), GFP_KERNEL);
		if (!mux)
			goto fail;

		mux_cfg = &data->muxes[composite->mux_id];

		mux->reg = priv->base + mux_cfg->reg_off;
		mux->shift = mux_cfg->shift;
		mux->mask = BIT(mux_cfg->width) - 1;
		mux->num_parents = mux_cfg->num_parents;
		mux->flags = 0;
		mux->parent_names = mux_cfg->parent_names;

		mux_clk = &mux->clk;
		mux_ops = &clk_mux_ops;

		parent_names = mux_cfg->parent_names;
		num_parents = mux_cfg->num_parents;
	} else {
		parent_names = &cfg->parent_name;
		num_parents = 1;
	}

	if  (composite->div_id != NO_STM32_DIV) {
		const struct stm32_div_cfg *div_cfg;

		div = kzalloc(sizeof(*div), GFP_KERNEL);
		if (!div)
			goto fail;

		div_cfg = &data->dividers[composite->div_id];

		div->reg = priv->base + div_cfg->reg_off;
		div->shift = div_cfg->shift;
		div->width = div_cfg->width;
		div->width = div_cfg->width;
		div->flags = div_cfg->div_flags;
		div->table = div_cfg->table;

		div_clk = &div->clk;
		div_ops = &clk_divider_ops;
	}

	if  (composite->gate_id != NO_STM32_GATE) {
		gate = kzalloc(sizeof(*gate), GFP_KERNEL);
		if (!gate)
			goto fail;

		gate->priv = priv;
		gate->gate_id = composite->gate_id;

		gate_clk = &gate->clk;
		gate_ops = &clk_stm32_gate_ops;
	}

	clk = clk_register_composite(NULL, cfg->name,
				     parent_names, num_parents,
				     mux_clk, mux_ops,
				     div_clk, div_ops,
				     gate_clk, gate_ops,
				     cfg->flags);
	if (IS_ERR(clk))
		goto fail;

	return clk;

fail:
	kfree(gate);
	kfree(div);
	kfree(mux);
	return ERR_CAST(clk);
}
