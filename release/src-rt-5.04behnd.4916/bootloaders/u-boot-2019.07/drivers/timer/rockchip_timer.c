// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <dm.h>
#include <dm/ofnode.h>
#include <mapmem.h>
#include <asm/arch-rockchip/timer.h>
#include <dt-structs.h>
#include <timer.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(OF_PLATDATA)
struct rockchip_timer_plat {
	struct dtd_rockchip_rk3368_timer dtd;
};
#endif

/* Driver private data. Contains timer id. Could be either 0 or 1. */
struct rockchip_timer_priv {
	struct rk_timer *timer;
};

static inline int64_t rockchip_timer_get_curr_value(struct rk_timer *timer)
{
	uint64_t timebase_h, timebase_l;
	uint64_t cntr;

	timebase_l = readl(&timer->timer_curr_value0);
	timebase_h = readl(&timer->timer_curr_value1);

	cntr = timebase_h << 32 | timebase_l;
	return cntr;
}

#if CONFIG_IS_ENABLED(BOOTSTAGE)
ulong timer_get_boot_us(void)
{
	uint64_t  ticks = 0;
	uint32_t  rate;
	uint64_t  us;
	int ret;

	ret = dm_timer_init();

	if (!ret) {
		/* The timer is available */
		rate = timer_get_rate(gd->timer);
		timer_get_count(gd->timer, &ticks);
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	} else if (ret == -EAGAIN) {
		/* We have been called so early that the DM is not ready,... */
		ofnode node = offset_to_ofnode(-1);
		struct rk_timer *timer = NULL;

		/*
		 * ... so we try to access the raw timer, if it is specified
		 * via the tick-timer property in /chosen.
		 */
		node = ofnode_get_chosen_node("tick-timer");
		if (!ofnode_valid(node)) {
			debug("%s: no /chosen/tick-timer\n", __func__);
			return 0;
		}

		timer = (struct rk_timer *)ofnode_get_addr(node);

		/* This timer is down-counting */
		ticks = ~0uLL - rockchip_timer_get_curr_value(timer);
		if (ofnode_read_u32(node, "clock-frequency", &rate)) {
			debug("%s: could not read clock-frequency\n", __func__);
			return 0;
		}
#endif
	} else {
		return 0;
	}

	us = (ticks * 1000) / rate;
	return us;
}
#endif

static int rockchip_timer_get_count(struct udevice *dev, u64 *count)
{
	struct rockchip_timer_priv *priv = dev_get_priv(dev);
	uint64_t cntr = rockchip_timer_get_curr_value(priv->timer);

	/* timers are down-counting */
	*count = ~0ull - cntr;
	return 0;
}

static int rockchip_clk_ofdata_to_platdata(struct udevice *dev)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rockchip_timer_priv *priv = dev_get_priv(dev);

	priv->timer = dev_read_addr_ptr(dev);
	if (!priv->timer)
		return -ENOENT;
#endif

	return 0;
}

static int rockchip_timer_start(struct udevice *dev)
{
	struct rockchip_timer_priv *priv = dev_get_priv(dev);
	const uint64_t reload_val = ~0uLL;
	const uint32_t reload_val_l = reload_val & 0xffffffff;
	const uint32_t reload_val_h = reload_val >> 32;

	/* don't reinit, if the timer is already running and set up */
	if ((readl(&priv->timer->timer_ctrl_reg) & 1) == 1 &&
	    (readl(&priv->timer->timer_load_count0) == reload_val_l) &&
	    (readl(&priv->timer->timer_load_count1) == reload_val_h))
		return 0;

	/* disable timer and reset all control */
	writel(0, &priv->timer->timer_ctrl_reg);
	/* write reload value */
	writel(reload_val_l, &priv->timer->timer_load_count0);
	writel(reload_val_h, &priv->timer->timer_load_count1);
	/* enable timer */
	writel(1, &priv->timer->timer_ctrl_reg);

	return 0;
}

static int rockchip_timer_probe(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct rockchip_timer_priv *priv = dev_get_priv(dev);
	struct rockchip_timer_plat *plat = dev_get_platdata(dev);

	priv->timer = map_sysmem(plat->dtd.reg[0], plat->dtd.reg[1]);
	uc_priv->clock_rate = plat->dtd.clock_frequency;
#endif

	return rockchip_timer_start(dev);
}

static const struct timer_ops rockchip_timer_ops = {
	.get_count = rockchip_timer_get_count,
};

static const struct udevice_id rockchip_timer_ids[] = {
	{ .compatible = "rockchip,rk3188-timer" },
	{ .compatible = "rockchip,rk3288-timer" },
	{ .compatible = "rockchip,rk3368-timer" },
	{}
};

U_BOOT_DRIVER(rockchip_rk3368_timer) = {
	.name	= "rockchip_rk3368_timer",
	.id	= UCLASS_TIMER,
	.of_match = rockchip_timer_ids,
	.probe = rockchip_timer_probe,
	.ops	= &rockchip_timer_ops,
	.priv_auto_alloc_size = sizeof(struct rockchip_timer_priv),
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	.platdata_auto_alloc_size = sizeof(struct rockchip_timer_plat),
#endif
	.ofdata_to_platdata = rockchip_clk_ofdata_to_platdata,
};
