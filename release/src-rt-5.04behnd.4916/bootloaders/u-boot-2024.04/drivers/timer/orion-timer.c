// SPDX-License-Identifier: GPL-2.0+
#include <asm/io.h>
#include <common.h>
#include <div64.h>
#include <dm/device.h>
#include <dm/fdtaddr.h>
#include <timer.h>

#define TIMER_CTRL		0x00
#define TIMER0_EN		BIT(0)
#define TIMER0_RELOAD_EN	BIT(1)
#define TIMER0_RELOAD		0x10
#define TIMER0_VAL		0x14

enum input_clock_type {
	INPUT_CLOCK_NON_FIXED,
	INPUT_CLOCK_25MHZ,	/* input clock rate is fixed to 25MHz */
};

struct orion_timer_priv {
	void *base;
};

#define MVEBU_TIMER_FIXED_RATE_25MHZ	25000000

static bool early_init_done(void *base)
{
	if ((readl(base + TIMER_CTRL) & TIMER0_EN) &&
	    (readl(base + TIMER0_RELOAD) == ~0))
		return true;
	return false;
}

/* Common functions for early (boot) and DM based timer */
static void orion_timer_init(void *base, enum input_clock_type type)
{
	/* Only init the timer once */
	if (early_init_done(base))
		return;

	writel(~0, base + TIMER0_VAL);
	writel(~0, base + TIMER0_RELOAD);

	if (type == INPUT_CLOCK_25MHZ) {
		/*
		 * On Armada XP / 38x ..., the 25MHz clock source needs to
		 * be enabled
		 */
		setbits_le32(base + TIMER_CTRL, BIT(11));
	}

	/* enable timer */
	setbits_le32(base + TIMER_CTRL, TIMER0_EN | TIMER0_RELOAD_EN);
}

static uint64_t orion_timer_get_count(void *base)
{
	return timer_conv_64(~readl(base + TIMER0_VAL));
}

/* Early (e.g. bootstage etc) timer functions */
static void notrace timer_early_init(void)
{
	if (IS_ENABLED(CONFIG_ARCH_MVEBU))
		orion_timer_init((void *)MVEBU_TIMER_BASE, INPUT_CLOCK_25MHZ);
	else
		orion_timer_init((void *)MVEBU_TIMER_BASE, INPUT_CLOCK_NON_FIXED);
}

/**
 * timer_early_get_rate() - Get the timer rate before driver model
 */
unsigned long notrace timer_early_get_rate(void)
{
	timer_early_init();

	if (IS_ENABLED(CONFIG_ARCH_MVEBU))
		return MVEBU_TIMER_FIXED_RATE_25MHZ;
	else
		return CFG_SYS_TCLK;
}

/**
 * timer_early_get_count() - Get the timer count before driver model
 *
 */
u64 notrace timer_early_get_count(void)
{
	timer_early_init();

	return orion_timer_get_count((void *)MVEBU_TIMER_BASE);
}

ulong timer_get_boot_us(void)
{
	u64 ticks;

	ticks = timer_early_get_count();
	return lldiv(ticks * 1000, timer_early_get_rate());
}

/* DM timer functions */
static uint64_t dm_orion_timer_get_count(struct udevice *dev)
{
	struct orion_timer_priv *priv = dev_get_priv(dev);

	return orion_timer_get_count(priv->base);
}

static int orion_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	enum input_clock_type type = dev_get_driver_data(dev);
	struct orion_timer_priv *priv = dev_get_priv(dev);

	priv->base = devfdt_remap_addr_index(dev, 0);
	if (!priv->base) {
		debug("unable to map registers\n");
		return -ENOMEM;
	}

	if (type == INPUT_CLOCK_25MHZ)
		uc_priv->clock_rate = MVEBU_TIMER_FIXED_RATE_25MHZ;
	else
		uc_priv->clock_rate = CFG_SYS_TCLK;
	orion_timer_init(priv->base, type);

	return 0;
}

static const struct timer_ops orion_timer_ops = {
	.get_count = dm_orion_timer_get_count,
};

static const struct udevice_id orion_timer_ids[] = {
	{ .compatible = "marvell,orion-timer", .data = INPUT_CLOCK_NON_FIXED },
	{ .compatible = "marvell,armada-370-timer", .data = INPUT_CLOCK_25MHZ },
	{ .compatible = "marvell,armada-xp-timer", .data = INPUT_CLOCK_25MHZ },
	{}
};

U_BOOT_DRIVER(orion_timer) = {
	.name	= "orion_timer",
	.id	= UCLASS_TIMER,
	.of_match = orion_timer_ids,
	.probe = orion_timer_probe,
	.ops	= &orion_timer_ops,
	.priv_auto	= sizeof(struct orion_timer_priv),
};
