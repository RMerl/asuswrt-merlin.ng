// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments K3 clock driver
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 *	Tero Kristo <t-kristo@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <soc.h>
#include <clk-uclass.h>
#include <k3-avs.h>
#include "k3-clk.h"

#define PLL_MIN_FREQ	800000000
#define PLL_MAX_FREQ	3200000000UL
#define PLL_MAX_DIV	127

/**
 * struct clk_map - mapping from dev/clk id tuples towards physical clocks
 * @dev_id: device ID for the clock
 * @clk_id: clock ID for the clock
 * @clk: pointer to the registered clock entry for the mapping
 */
struct clk_map {
	u16 dev_id;
	u32 clk_id;
	struct clk *clk;
};

/**
 * struct ti_clk_data - clock controller information structure
 * @map: mapping from dev/clk id tuples to physical clock entries
 * @size: number of entries in the map
 */
struct ti_clk_data {
	struct clk_map *map;
	int size;
};

static ulong osc_freq;

static void clk_add_map(struct ti_clk_data *data, struct clk *clk,
			u32 dev_id, u32 clk_id)
{
	struct clk_map *map;

	debug("%s: added clk=%p, data=%p, dev=%d, clk=%d\n", __func__,
	      clk, data, dev_id, clk_id);
	if (!clk)
		return;

	map = data->map + data->size++;

	map->dev_id = dev_id;
	map->clk_id = clk_id;
	map->clk = clk;
}

static const struct soc_attr ti_k3_soc_clk_data[] = {
#if IS_ENABLED(CONFIG_SOC_K3_J721E)
	{
		.family = "J721E",
		.data = &j721e_clk_platdata,
	},
	{
		.family = "J7200",
		.data = &j7200_clk_platdata,
	},
#elif CONFIG_SOC_K3_J721S2
	{
		.family = "J721S2",
		.data = &j721s2_clk_platdata,
	},
#endif
#ifdef CONFIG_SOC_K3_AM625
	{
		.family = "AM62X",
		.data = &am62x_clk_platdata,
	},
#endif
#ifdef CONFIG_SOC_K3_AM62A7
	{
		.family = "AM62AX",
		.data = &am62ax_clk_platdata,
	},
#endif
	{ /* sentinel */ }
};

static int ti_clk_probe(struct udevice *dev)
{
	struct ti_clk_data *data = dev_get_priv(dev);
	struct clk *clk;
	const char *name;
	const struct clk_data *ti_clk_data;
	int i, j;
	const struct soc_attr *soc_match_data;
	const struct ti_k3_clk_platdata *pdata;

	debug("%s(dev=%p)\n", __func__, dev);

	soc_match_data = soc_device_match(ti_k3_soc_clk_data);
	if (!soc_match_data)
		return -ENODEV;

	pdata = (const struct ti_k3_clk_platdata *)soc_match_data->data;

	data->map = kcalloc(pdata->soc_dev_clk_data_cnt, sizeof(*data->map),
			    GFP_KERNEL);
	data->size = 0;

	for (i = 0; i < pdata->clk_list_cnt; i++) {
		ti_clk_data = &pdata->clk_list[i];

		switch (ti_clk_data->type) {
		case CLK_TYPE_FIXED_RATE:
			name = ti_clk_data->clk.fixed_rate.name;
			clk = clk_register_fixed_rate(NULL,
						      name,
						      ti_clk_data->clk.fixed_rate.rate);
			break;
		case CLK_TYPE_DIV:
			name = ti_clk_data->clk.div.name;
			clk = clk_register_divider(NULL, name,
						   ti_clk_data->clk.div.parent,
						   ti_clk_data->clk.div.flags,
						   map_physmem(ti_clk_data->clk.div.reg, 0, MAP_NOCACHE),
						   ti_clk_data->clk.div.shift,
						   ti_clk_data->clk.div.width,
						   ti_clk_data->clk.div.div_flags);
			break;
		case CLK_TYPE_MUX:
			name = ti_clk_data->clk.mux.name;
			clk = clk_register_mux(NULL, name,
					       ti_clk_data->clk.mux.parents,
					       ti_clk_data->clk.mux.num_parents,
					       ti_clk_data->clk.mux.flags,
					       map_physmem(ti_clk_data->clk.mux.reg, 0, MAP_NOCACHE),
					       ti_clk_data->clk.mux.shift,
					       ti_clk_data->clk.mux.width,
					       0);
			break;
		case CLK_TYPE_PLL:
			name = ti_clk_data->clk.pll.name;
			clk = clk_register_ti_pll(name,
						  ti_clk_data->clk.pll.parent,
						  map_physmem(ti_clk_data->clk.pll.reg, 0, MAP_NOCACHE));

			if (!osc_freq)
				osc_freq = clk_get_rate(clk_get_parent(clk));
			break;
		default:
			name = NULL;
			clk = NULL;
			printf("WARNING: %s has encountered unknown clk type %d\n",
			       __func__, ti_clk_data->type);
		}

		if (clk && ti_clk_data->default_freq)
			clk_set_rate(clk, ti_clk_data->default_freq);

		if (clk && name) {
			for (j = 0; j < pdata->soc_dev_clk_data_cnt; j++) {
				if (!strcmp(name, pdata->soc_dev_clk_data[j].clk_name)) {
					clk_add_map(data, clk, pdata->soc_dev_clk_data[j].dev_id,
						    pdata->soc_dev_clk_data[j].clk_id);
				}
			}
		}
	}

	return 0;
}

static int _clk_cmp(u32 dev_id, u32 clk_id, const struct clk_map *map)
{
	if (map->dev_id == dev_id && map->clk_id == clk_id)
		return 0;
	if (map->dev_id > dev_id ||
	    (map->dev_id == dev_id && map->clk_id > clk_id))
		return -1;
	return 1;
}

static int bsearch(u32 dev_id, u32 clk_id, struct clk_map *map, int num)
{
	int result;
	int idx;

	for (idx = 0; idx < num; idx++) {
		result = _clk_cmp(dev_id, clk_id, &map[idx]);

		if (result == 0)
			return idx;
	}

	return -ENOENT;
}

static int ti_clk_of_xlate(struct clk *clk,
			   struct ofnode_phandle_args *args)
{
	struct ti_clk_data *data = dev_get_priv(clk->dev);
	int idx;

	debug("%s(clk=%p, args_count=%d [0]=%d [1]=%d)\n", __func__, clk,
	      args->args_count, args->args[0], args->args[1]);

	if (args->args_count != 2) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (!data->size)
		return -EPROBE_DEFER;

	idx = bsearch(args->args[0], args->args[1], data->map, data->size);
	if (idx < 0)
		return idx;

	clk->id = idx;

	return 0;
}

static ulong ti_clk_get_rate(struct clk *clk)
{
	struct ti_clk_data *data = dev_get_priv(clk->dev);
	struct clk *clkp = data->map[clk->id].clk;

	return clk_get_rate(clkp);
}

static ulong ti_clk_set_rate(struct clk *clk, ulong rate)
{
	struct ti_clk_data *data = dev_get_priv(clk->dev);
	struct clk *clkp = data->map[clk->id].clk;
	int div = 1;
	ulong child_rate;
	const struct clk_ops *ops;
	ulong new_rate, rem;
	ulong diff, new_diff;
	int freq_scale_up = rate >= ti_clk_get_rate(clk) ? 1 : 0;

	if (IS_ENABLED(CONFIG_K3_AVS0) && freq_scale_up)
		k3_avs_notify_freq(data->map[clk->id].dev_id,
				   data->map[clk->id].clk_id, rate);
	/*
	 * We must propagate rate change to parent if current clock type
	 * does not allow setting it.
	 */
	while (clkp) {
		ops = clkp->dev->driver->ops;
		if (ops->set_rate)
			break;

		/*
		 * Store child rate so we can calculate the clock rate
		 * that must be passed to parent
		 */
		child_rate = clk_get_rate(clkp);
		clkp = clk_get_parent(clkp);
		if (clkp) {
			debug("%s: propagating rate change to parent %s, rate=%u.\n",
			      __func__, clkp->dev->name, (u32)rate / div);
			div *= clk_get_rate(clkp) / child_rate;
		}
	}

	if (!clkp)
		return -ENOSYS;

	child_rate = clk_get_rate(clkp);

	new_rate = clk_set_rate(clkp, rate / div);

	diff = abs(new_rate - rate / div);

	debug("%s: clk=%s, div=%d, rate=%u, new_rate=%u, diff=%u\n", __func__,
	      clkp->dev->name, div, (u32)rate, (u32)new_rate, (u32)diff);

	/*
	 * If the new rate differs by 50% of the target,
	 * modify parent. This handles typical cases where we have a hsdiv
	 * following directly a PLL
	 */

	if (diff > rate / div / 2) {
		ulong pll_tgt;
		int pll_div = 0;

		clk = clkp;

		debug("%s: propagating rate change to parent, rate=%u.\n",
		      __func__, (u32)rate / div);

		clkp = clk_get_parent(clkp);

		if (rate > osc_freq) {
			if (rate > PLL_MAX_FREQ / 2 && rate < PLL_MAX_FREQ) {
				pll_tgt = rate;
				pll_div = 1;
			} else {
				for (pll_div = 2; pll_div < PLL_MAX_DIV; pll_div++) {
					pll_tgt = rate / div * pll_div;
					if (pll_tgt >= PLL_MIN_FREQ && pll_tgt <= PLL_MAX_FREQ)
						break;
				}
			}
		} else {
			pll_tgt = osc_freq;
			pll_div = rate / div / osc_freq;
		}

		debug("%s: pll_tgt=%u, rate=%u, div=%u\n", __func__,
		      (u32)pll_tgt, (u32)rate, pll_div);

		clk_set_rate(clkp, pll_tgt);

		return clk_set_rate(clk, rate / div) * div;
	}

	/*
	 * If the new rate differs by at least 5% of the target,
	 * we must check for rounding error in a divider, so try
	 * set rate with rate + (parent_freq % rate).
	 */

	if (diff > rate / div / 20) {
		u64 parent_freq = clk_get_parent_rate(clkp);

		rem = parent_freq % rate;
		new_rate = clk_set_rate(clkp, (rate / div) + rem);
		new_diff = abs(new_rate - rate / div);

		if (new_diff > diff) {
			new_rate = clk_set_rate(clkp, rate / div);
		} else {
			debug("%s: Using better rate %lu that gives diff %lu\n",
			      __func__, new_rate, new_diff);
		}
	}

	if (IS_ENABLED(CONFIG_K3_AVS0) && !freq_scale_up)
		k3_avs_notify_freq(data->map[clk->id].dev_id,
				   data->map[clk->id].clk_id, rate);

	return new_rate;
}

static int ti_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct ti_clk_data *data = dev_get_priv(clk->dev);
	struct clk *clkp = data->map[clk->id].clk;
	struct clk *parentp = data->map[parent->id].clk;

	return clk_set_parent(clkp, parentp);
}

static int ti_clk_enable(struct clk *clk)
{
	struct ti_clk_data *data = dev_get_priv(clk->dev);
	struct clk *clkp = data->map[clk->id].clk;

	return clk_enable(clkp);
}

static int ti_clk_disable(struct clk *clk)
{
	struct ti_clk_data *data = dev_get_priv(clk->dev);
	struct clk *clkp = data->map[clk->id].clk;

	return clk_disable(clkp);
}

static const struct udevice_id ti_clk_of_match[] = {
	{ .compatible = "ti,k2g-sci-clk" },
	{ /* sentinel */ },
};

static const struct clk_ops ti_clk_ops = {
	.of_xlate = ti_clk_of_xlate,
	.set_rate = ti_clk_set_rate,
	.get_rate = ti_clk_get_rate,
	.enable = ti_clk_enable,
	.disable = ti_clk_disable,
	.set_parent = ti_clk_set_parent,
};

U_BOOT_DRIVER(ti_clk) = {
	.name = "ti-clk",
	.id = UCLASS_CLK,
	.of_match = ti_clk_of_match,
	.probe = ti_clk_probe,
	.priv_auto = sizeof(struct ti_clk_data),
	.ops = &ti_clk_ops,
};
