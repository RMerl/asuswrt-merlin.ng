// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 BayLibre, SAS.
 */

#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <reset.h>
#include <wdt.h>
#include <asm/io.h>
#include <linux/bitops.h>

#define GXBB_WDT_CTRL_REG			0x0
#define GXBB_WDT_TCNT_REG			0x8
#define GXBB_WDT_RSET_REG			0xc

#define GXBB_WDT_CTRL_SYS_RESET_NOW		BIT(26)
#define GXBB_WDT_CTRL_CLKDIV_EN			BIT(25)
#define GXBB_WDT_CTRL_CLK_EN			BIT(24)
#define GXBB_WDT_CTRL_EE_RESET			BIT(21)
#define GXBB_WDT_CTRL_EN			BIT(18)

#define GXBB_WDT_CTRL_DIV_MASK			GENMASK(17, 0)
#define GXBB_WDT_TCNT_SETUP_MASK		GENMASK(15, 0)


struct amlogic_wdt_priv {
	void __iomem *reg_base;
};

static int amlogic_wdt_set_timeout(struct udevice *dev, u64 timeout_ms)
{
	struct amlogic_wdt_priv *data = dev_get_priv(dev);

	if (timeout_ms > GXBB_WDT_TCNT_SETUP_MASK) {
		dev_warn(dev, "%s: timeout_ms=%llu: maximum watchdog timeout exceeded\n",
		         __func__, timeout_ms);
		timeout_ms = GXBB_WDT_TCNT_SETUP_MASK;
	}

	writel(timeout_ms, data->reg_base + GXBB_WDT_TCNT_REG);

	return 0;
}

static int amlogic_wdt_stop(struct udevice *dev)
{
	struct amlogic_wdt_priv *data = dev_get_priv(dev);

	writel(readl(data->reg_base + GXBB_WDT_CTRL_REG) & ~GXBB_WDT_CTRL_EN,
	       data->reg_base + GXBB_WDT_CTRL_REG);

	return 0;
}

static int amlogic_wdt_start(struct udevice *dev, u64 time_ms, ulong flags)
{
	struct amlogic_wdt_priv *data = dev_get_priv(dev);

	writel(readl(data->reg_base + GXBB_WDT_CTRL_REG) | GXBB_WDT_CTRL_EN,
	       data->reg_base + GXBB_WDT_CTRL_REG);

	return amlogic_wdt_set_timeout(dev, time_ms);
}

static int amlogic_wdt_reset(struct udevice *dev)
{
	struct amlogic_wdt_priv *data = dev_get_priv(dev);

	writel(0, data->reg_base + GXBB_WDT_RSET_REG);

	return 0;
}

static int amlogic_wdt_expire_now(struct udevice *dev, ulong flags)
{
	struct amlogic_wdt_priv *data = dev_get_priv(dev);

	writel(0, data->reg_base + GXBB_WDT_CTRL_SYS_RESET_NOW);

	return 0;
}

static int amlogic_wdt_probe(struct udevice *dev)
{
	struct amlogic_wdt_priv *data = dev_get_priv(dev);
	int ret;

	data->reg_base = dev_remap_addr(dev);
	if (!data->reg_base)
		return -EINVAL;

	struct clk clk;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	/* Setup with 1ms timebase */
	writel(((clk_get_rate(&clk) / 1000) & GXBB_WDT_CTRL_DIV_MASK) |
	       GXBB_WDT_CTRL_EE_RESET |
	       GXBB_WDT_CTRL_CLK_EN |
	       GXBB_WDT_CTRL_CLKDIV_EN,
	       data->reg_base + GXBB_WDT_CTRL_REG);

	return 0;
}

static const struct wdt_ops amlogic_wdt_ops = {
	.start = amlogic_wdt_start,
	.reset = amlogic_wdt_reset,
	.stop = amlogic_wdt_stop,
	.expire_now = amlogic_wdt_expire_now,
};

static const struct udevice_id amlogic_wdt_ids[] = {
	{ .compatible = "amlogic,meson-gxbb-wdt" },
	{}
};

U_BOOT_DRIVER(amlogic_wdt) = {
	.name = "amlogic_wdt",
	.id = UCLASS_WDT,
	.of_match = amlogic_wdt_ids,
	.priv_auto = sizeof(struct amlogic_wdt_priv),
	.probe = amlogic_wdt_probe,
	.ops = &amlogic_wdt_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
