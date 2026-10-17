// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <wdt.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitfield.h>

DECLARE_GLOBAL_DATA_PTR;

#define CORE0_POKE_OFFSET_MASK	0xfffffULL

#define WDOG_MODE		GENMASK_ULL(1, 0)
#define WDOG_LEN		GENMASK_ULL(19, 4)
#define WDOG_CNT		GENMASK_ULL(43, 20)

struct octeontx_wdt_data {
	u32 wdog_offset;
	u32 poke_offset;
	int timer_shift;
	bool has_clk;
};

struct octeontx_wdt {
	void __iomem *reg;
	const struct octeontx_wdt_data *data;
	struct clk clk;
};

static int octeontx_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct octeontx_wdt *priv = dev_get_priv(dev);
	u64 clk_rate, val;
	u64 tout_wdog;

	if (priv->data->has_clk) {
		clk_rate = clk_get_rate(&priv->clk);
		if (IS_ERR_VALUE(clk_rate))
			return -EINVAL;
	} else {
		clk_rate = gd->bus_clk;
	}

	/* Watchdog counts in configured cycle steps */
	tout_wdog = (clk_rate * timeout_ms / 1000) >> priv->data->timer_shift;

	/*
	 * We can only specify the upper 16 bits of a 24 bit value.
	 * Round up
	 */
	tout_wdog = (tout_wdog + 0xff) >> 8;

	/* If the timeout overflows the hardware limit, set max */
	if (tout_wdog >= 0x10000)
		tout_wdog = 0xffff;

	val = FIELD_PREP(WDOG_MODE, 0x3) |
		FIELD_PREP(WDOG_LEN, tout_wdog) |
		FIELD_PREP(WDOG_CNT, tout_wdog << 8);
	writeq(val, priv->reg + priv->data->wdog_offset);

	return 0;
}

static int octeontx_wdt_stop(struct udevice *dev)
{
	struct octeontx_wdt *priv = dev_get_priv(dev);

	writeq(0, priv->reg + priv->data->wdog_offset);

	return 0;
}

static int octeontx_wdt_expire_now(struct udevice *dev, ulong flags)
{
	octeontx_wdt_stop(dev);

	/* Start with 100ms timeout to expire immediately */
	octeontx_wdt_start(dev, 100, flags);

	return 0;
}

static int octeontx_wdt_reset(struct udevice *dev)
{
	struct octeontx_wdt *priv = dev_get_priv(dev);

	writeq(~0ULL, priv->reg + priv->data->poke_offset);

	return 0;
}

static int octeontx_wdt_remove(struct udevice *dev)
{
	octeontx_wdt_stop(dev);

	return 0;
}

static int octeontx_wdt_probe(struct udevice *dev)
{
	struct octeontx_wdt *priv = dev_get_priv(dev);
	int ret;

	priv->reg = dev_remap_addr(dev);
	if (!priv->reg)
		return -EINVAL;

	priv->data = (void *)dev_get_driver_data(dev);
	if (!priv->data)
		return -EINVAL;

	/*
	 * Save base register address in reg masking lower 20 bits
	 * as 0xa0000 appears when extracted from the DT
	 */
	priv->reg = (void __iomem *)(((u64)priv->reg &
				      ~CORE0_POKE_OFFSET_MASK));

	if (priv->data->has_clk) {
		ret = clk_get_by_index(dev, 0, &priv->clk);
		if (ret < 0)
			return ret;

		ret = clk_enable(&priv->clk);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct wdt_ops octeontx_wdt_ops = {
	.reset = octeontx_wdt_reset,
	.start = octeontx_wdt_start,
	.stop = octeontx_wdt_stop,
	.expire_now = octeontx_wdt_expire_now,
};

static const struct octeontx_wdt_data octeontx_data = {
	.wdog_offset = 0x40000,
	.poke_offset = 0x50000,
	.timer_shift = 10,
	.has_clk = true,
};

static const struct octeontx_wdt_data octeon_data = {
	.wdog_offset = 0x20000,
	.poke_offset = 0x30000,
	.timer_shift = 10,
	.has_clk = false,
};

static const struct udevice_id octeontx_wdt_ids[] = {
	{ .compatible = "arm,sbsa-gwdt", .data = (ulong)&octeontx_data },
	{ .compatible = "cavium,octeon-7890-ciu3", .data = (ulong)&octeon_data },
	{}
};

U_BOOT_DRIVER(wdt_octeontx) = {
	.name = "wdt_octeontx",
	.id = UCLASS_WDT,
	.of_match = octeontx_wdt_ids,
	.ops = &octeontx_wdt_ops,
	.priv_auto = sizeof(struct octeontx_wdt),
	.probe = octeontx_wdt_probe,
	.remove = octeontx_wdt_remove,
	.flags = DM_FLAG_OS_PREPARE,
};
