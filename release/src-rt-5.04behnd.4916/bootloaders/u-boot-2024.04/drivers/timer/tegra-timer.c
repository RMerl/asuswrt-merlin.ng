// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/tegra.h>

#define TEGRA_OSC_CLK_ENB_L_SET		(NV_PA_CLK_RST_BASE + 0x320)
#define TEGRA_OSC_SET_CLK_ENB_TMR	BIT(5)

#define TEGRA_TIMER_USEC_CNTR		(NV_PA_TMRUS_BASE + 0)
#define TEGRA_TIMER_USEC_CFG		(NV_PA_TMRUS_BASE + 4)

#define TEGRA_TIMER_RATE		1000000 /* 1 MHz */

/*
 * On pre-DM stage timer should be left configured by
 * previous bootloader for correct 1MHz clock.
 * In the case of reset default value is set to 1/13 of
 * CLK_M which should be decent enough to safely
 * get to DM stage.
 */
u64 notrace timer_early_get_count(void)
{
	/* At this stage raw timer is used */
	return readl(TEGRA_TIMER_USEC_CNTR);
}

unsigned long notrace timer_early_get_rate(void)
{
	return TEGRA_TIMER_RATE;
}

ulong timer_get_boot_us(void)
{
	return timer_early_get_count();
}

/*
 * At moment of calling get_count, timer driver is already
 * probed and is configured to have precise 1MHz clock.
 * Tegra timer has a step of 1 microsecond which removes
 * need of using adjusments involving uc_priv->clock_rate.
 */
static notrace u64 tegra_timer_get_count(struct udevice *dev)
{
	u32 val = timer_early_get_count();
	return timer_conv_64(val);
}

static int tegra_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	u32 usec_config, value;

	/* Timer rate has to be set unconditionally */
	uc_priv->clock_rate = TEGRA_TIMER_RATE;

	/*
	 * Configure microsecond timers to have 1MHz clock
	 * Config register is 0xqqww, where qq is "dividend", ww is "divisor"
	 * Uses n+1 scheme
	 */
	switch (clock_get_rate(CLOCK_ID_CLK_M)) {
	case 12000000:
		usec_config = 0x000b; /* (11+1)/(0+1) */
		break;
	case 12800000:
		usec_config = 0x043f; /* (63+1)/(4+1) */
		break;
	case 13000000:
		usec_config = 0x000c; /* (12+1)/(0+1) */
		break;
	case 16800000:
		usec_config = 0x0453; /* (83+1)/(4+1) */
		break;
	case 19200000:
		usec_config = 0x045f; /* (95+1)/(4+1) */
		break;
	case 26000000:
		usec_config = 0x0019; /* (25+1)/(0+1) */
		break;
	case 38400000:
		usec_config = 0x04bf; /* (191+1)/(4+1) */
		break;
	case 48000000:
		usec_config = 0x002f; /* (47+1)/(0+1) */
		break;
	default:
		return -EINVAL;
	}

	/* Enable clock to timer hardware */
	value = readl_relaxed(TEGRA_OSC_CLK_ENB_L_SET);
	writel_relaxed(value | TEGRA_OSC_SET_CLK_ENB_TMR,
		       TEGRA_OSC_CLK_ENB_L_SET);

	writel_relaxed(usec_config, TEGRA_TIMER_USEC_CFG);

	return 0;
}

static const struct timer_ops tegra_timer_ops = {
	.get_count = tegra_timer_get_count,
};

static const struct udevice_id tegra_timer_ids[] = {
	{ .compatible = "nvidia,tegra20-timer" },
	{ .compatible = "nvidia,tegra30-timer" },
	{ .compatible = "nvidia,tegra114-timer" },
	{ .compatible = "nvidia,tegra124-timer" },
	{ .compatible = "nvidia,tegra210-timer" },
	{ }
};

U_BOOT_DRIVER(tegra_timer) = {
	.name		= "tegra_timer",
	.id		= UCLASS_TIMER,
	.of_match	= tegra_timer_ids,
	.probe		= tegra_timer_probe,
	.ops		= &tegra_timer_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};
