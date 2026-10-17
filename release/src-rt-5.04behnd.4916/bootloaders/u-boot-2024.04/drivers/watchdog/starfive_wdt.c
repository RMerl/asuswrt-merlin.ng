// SPDX-License-Identifier: GPL-2.0
/*
 * Starfive Watchdog driver
 *
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 */

#include <clk.h>
#include <dm.h>
#include <reset.h>
#include <wdt.h>
#include <linux/iopoll.h>

/* JH7110 Watchdog register define */
#define STARFIVE_WDT_JH7110_LOAD	0x000
#define STARFIVE_WDT_JH7110_VALUE	0x004
#define STARFIVE_WDT_JH7110_CONTROL	0x008	/*
						 * [0]: reset enable;
						 * [1]: interrupt enable && watchdog enable
						 * [31:2]: reserved.
						 */
#define STARFIVE_WDT_JH7110_INTCLR	0x00c	/* clear intterupt and reload the counter */
#define STARFIVE_WDT_JH7110_IMS		0x014
#define STARFIVE_WDT_JH7110_LOCK	0xc00	/* write 0x1ACCE551 to unlock */

/* WDOGCONTROL */
#define STARFIVE_WDT_ENABLE			0x1
#define STARFIVE_WDT_EN_SHIFT			0
#define STARFIVE_WDT_RESET_EN			0x1
#define STARFIVE_WDT_JH7110_RST_EN_SHIFT	1

/* WDOGLOCK */
#define STARFIVE_WDT_JH7110_UNLOCK_KEY		0x1acce551

/* WDOGINTCLR */
#define STARFIVE_WDT_INTCLR			0x1
#define STARFIVE_WDT_JH7100_INTCLR_AVA_SHIFT	1	/* Watchdog can clear interrupt when 0 */

#define STARFIVE_WDT_MAXCNT			0xffffffff
#define STARFIVE_WDT_DEFAULT_TIME		(15)
#define STARFIVE_WDT_DELAY_US			0
#define STARFIVE_WDT_TIMEOUT_US			10000

/* module parameter */
#define STARFIVE_WDT_EARLY_ENA			0

struct starfive_wdt_variant {
	unsigned int control;		/* Watchdog Control Resgister for reset enable */
	unsigned int load;		/* Watchdog Load register */
	unsigned int reload;		/* Watchdog Reload Control register */
	unsigned int enable;		/* Watchdog Enable Register */
	unsigned int value;		/* Watchdog Counter Value Register */
	unsigned int int_clr;		/* Watchdog Interrupt Clear Register */
	unsigned int unlock;		/* Watchdog Lock Register */
	unsigned int int_status;	/* Watchdog Interrupt Status Register */

	u32 unlock_key;
	char enrst_shift;
	char en_shift;
	bool intclr_check;		/*  whether need to check it before clearing interrupt */
	char intclr_ava_shift;
	bool double_timeout;		/* The watchdog need twice timeout to reboot */
};

struct starfive_wdt_priv {
	void __iomem *base;
	struct clk *core_clk;
	struct clk *apb_clk;
	struct reset_ctl_bulk *rst;
	const struct starfive_wdt_variant *variant;
	unsigned long freq;
	u32 count;			/* count of timeout */
	u32 reload;			/* restore the count */
};

/* Register layout and configuration for the JH7110 */
static const struct starfive_wdt_variant starfive_wdt_jh7110_variant = {
	.control = STARFIVE_WDT_JH7110_CONTROL,
	.load = STARFIVE_WDT_JH7110_LOAD,
	.enable = STARFIVE_WDT_JH7110_CONTROL,
	.value = STARFIVE_WDT_JH7110_VALUE,
	.int_clr = STARFIVE_WDT_JH7110_INTCLR,
	.unlock = STARFIVE_WDT_JH7110_LOCK,
	.unlock_key = STARFIVE_WDT_JH7110_UNLOCK_KEY,
	.int_status = STARFIVE_WDT_JH7110_IMS,
	.enrst_shift = STARFIVE_WDT_JH7110_RST_EN_SHIFT,
	.en_shift = STARFIVE_WDT_EN_SHIFT,
	.intclr_check = false,
	.double_timeout = true,
};

static int starfive_wdt_enable_clock(struct starfive_wdt_priv *wdt)
{
	int ret;

	ret = clk_enable(wdt->apb_clk);
	if (ret)
		return ret;

	ret = clk_enable(wdt->core_clk);
	if (ret) {
		clk_disable(wdt->apb_clk);
		return ret;
	}

	return 0;
}

static void starfive_wdt_disable_clock(struct starfive_wdt_priv *wdt)
{
	clk_disable(wdt->core_clk);
	clk_disable(wdt->apb_clk);
}

/* Write unlock-key to unlock. Write other value to lock. */
static void starfive_wdt_unlock(struct starfive_wdt_priv *wdt)
{
	writel(wdt->variant->unlock_key, wdt->base + wdt->variant->unlock);
}

static void starfive_wdt_lock(struct starfive_wdt_priv *wdt)
{
	writel(~wdt->variant->unlock_key, wdt->base + wdt->variant->unlock);
}

/* enable watchdog interrupt to reset/reboot */
static void starfive_wdt_enable_reset(struct starfive_wdt_priv *wdt)
{
	u32 val;

	val = readl(wdt->base + wdt->variant->control);
	val |= STARFIVE_WDT_RESET_EN << wdt->variant->enrst_shift;
	writel(val, wdt->base + wdt->variant->control);
}

/* waiting interrupt can be free to clear */
static int starfive_wdt_wait_int_free(struct starfive_wdt_priv *wdt)
{
	u32 value;

	return readl_poll_timeout(wdt->base + wdt->variant->int_clr, value,
				  !(value & BIT(wdt->variant->intclr_ava_shift)),
				  STARFIVE_WDT_TIMEOUT_US);
}

/* clear interrupt signal before initialization or reload */
static int starfive_wdt_int_clr(struct starfive_wdt_priv *wdt)
{
	int ret;

	if (wdt->variant->intclr_check) {
		ret = starfive_wdt_wait_int_free(wdt);
		if (ret)
			return ret;
	}
	writel(STARFIVE_WDT_INTCLR, wdt->base + wdt->variant->int_clr);

	return 0;
}

static inline void starfive_wdt_set_count(struct starfive_wdt_priv *wdt,
					  u32 val)
{
	writel(val, wdt->base + wdt->variant->load);
}

/* enable watchdog */
static inline void starfive_wdt_enable(struct starfive_wdt_priv *wdt)
{
	u32 val;

	val = readl(wdt->base + wdt->variant->enable);
	val |= STARFIVE_WDT_ENABLE << wdt->variant->en_shift;
	writel(val, wdt->base + wdt->variant->enable);
}

/* disable watchdog */
static inline void starfive_wdt_disable(struct starfive_wdt_priv *wdt)
{
	u32 val;

	val = readl(wdt->base + wdt->variant->enable);
	val &= ~(STARFIVE_WDT_ENABLE << wdt->variant->en_shift);
	writel(val, wdt->base + wdt->variant->enable);
}

static inline void starfive_wdt_set_reload_count(struct starfive_wdt_priv *wdt,
						 u32 count)
{
	starfive_wdt_set_count(wdt, count);

	/* 7100 need set any value to reload register and could reload value to counter */
	if (wdt->variant->reload)
		writel(0x1, wdt->base + wdt->variant->reload);
}

static int starfive_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	int ret;
	struct starfive_wdt_priv *wdt = dev_get_priv(dev);

	starfive_wdt_unlock(wdt);
	/* disable watchdog, to be safe */
	starfive_wdt_disable(wdt);

	starfive_wdt_enable_reset(wdt);
	ret = starfive_wdt_int_clr(wdt);
	if (ret)
		goto exit;

	wdt->count = (timeout_ms / 1000) * wdt->freq;
	if (wdt->variant->double_timeout)
		wdt->count /= 2;

	starfive_wdt_set_count(wdt, wdt->count);
	starfive_wdt_enable(wdt);

exit:
	starfive_wdt_lock(wdt);
	return ret;
}

static int starfive_wdt_stop(struct udevice *dev)
{
	struct starfive_wdt_priv *wdt = dev_get_priv(dev);

	starfive_wdt_unlock(wdt);
	starfive_wdt_disable(wdt);
	starfive_wdt_lock(wdt);

	return 0;
}

static int starfive_wdt_reset(struct udevice *dev)
{
	int ret;
	struct starfive_wdt_priv *wdt = dev_get_priv(dev);

	starfive_wdt_unlock(wdt);
	ret = starfive_wdt_int_clr(wdt);
	if (ret)
		goto exit;

	starfive_wdt_set_reload_count(wdt, wdt->count);

exit:
	starfive_wdt_lock(wdt);

	return ret;
}

static const struct wdt_ops starfive_wdt_ops = {
	.start = starfive_wdt_start,
	.stop = starfive_wdt_stop,
	.reset = starfive_wdt_reset,
};

static int starfive_wdt_probe(struct udevice *dev)
{
	struct starfive_wdt_priv *wdt = dev_get_priv(dev);
	int ret;

	ret = starfive_wdt_enable_clock(wdt);
	if (ret)
		return ret;

	ret = reset_deassert_bulk(wdt->rst);
	if (ret)
		goto err_reset;

	wdt->variant = (const struct starfive_wdt_variant *)dev_get_driver_data(dev);

	wdt->freq = clk_get_rate(wdt->core_clk);
	if (!wdt->freq) {
		ret = -EINVAL;
		goto err_get_freq;
	}

	return 0;

err_get_freq:
	reset_assert_bulk(wdt->rst);
err_reset:
	starfive_wdt_disable_clock(wdt);

	return ret;
}

static int starfive_wdt_of_to_plat(struct udevice *dev)
{
	struct starfive_wdt_priv *wdt = dev_get_priv(dev);

	wdt->base = (void *)dev_read_addr(dev);
	if (!wdt->base)
		return -ENODEV;

	wdt->apb_clk = devm_clk_get(dev, "apb");
	if (IS_ERR(wdt->apb_clk))
		return -ENODEV;

	wdt->core_clk = devm_clk_get(dev, "core");
	if (IS_ERR(wdt->core_clk))
		return -ENODEV;

	wdt->rst = devm_reset_bulk_get(dev);
	if (IS_ERR(wdt->rst))
		return -ENODEV;

	return 0;
}

static const struct udevice_id starfive_wdt_ids[] = {
	{
		.compatible = "starfive,jh7110-wdt",
		.data = (ulong)&starfive_wdt_jh7110_variant
	}, {
		/* sentinel */
	}
};

U_BOOT_DRIVER(starfive_wdt) = {
	.name = "starfive_wdt",
	.id = UCLASS_WDT,
	.of_match = starfive_wdt_ids,
	.priv_auto = sizeof(struct starfive_wdt_priv),
	.probe = starfive_wdt_probe,
	.of_to_plat = starfive_wdt_of_to_plat,
	.ops = &starfive_wdt_ops,
};
