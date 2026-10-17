// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/global_data.h>
#include <dt-bindings/clock/rk3288-cru.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <power/regulator.h>

/*
 * We should increase the DDR voltage to 1.2V using the PWM regulator.
 * There is a U-Boot driver for this but it may need to add support for the
 * 'voltage-table' property.
 */
#ifndef CONFIG_SPL_BUILD
#if !CONFIG_IS_ENABLED(ROCKCHIP_BACK_TO_BROM)
static int veyron_init(void)
{
	struct udevice *dev;
	struct clk clk;
	int ret;

	ret = regulator_get_by_platname("vdd_arm", &dev);
	if (ret) {
		debug("Cannot set regulator name\n");
		return ret;
	}

	/* Slowly raise to max CPU voltage to prevent overshoot */
	ret = regulator_set_value(dev, 1200000);
	if (ret)
		return ret;
	udelay(175); /* Must wait for voltage to stabilize, 2mV/us */
	ret = regulator_set_value(dev, 1400000);
	if (ret)
		return ret;
	udelay(100); /* Must wait for voltage to stabilize, 2mV/us */

	ret = rockchip_get_clk(&clk.dev);
	if (ret)
		return ret;
	clk.id = PLL_APLL;
	ret = clk_set_rate(&clk, 1800000000);
	if (IS_ERR_VALUE(ret))
		return ret;

	ret = regulator_get_by_platname("vcc33_sd", &dev);
	if (ret) {
		debug("Cannot get regulator name\n");
		return ret;
	}

	ret = regulator_set_value(dev, 3300000);
	if (ret)
		return ret;

	ret = regulators_enable_boot_on(false);
	if (ret) {
		debug("%s: Cannot enable boot on regulators\n", __func__);
		return ret;
	}

	return 0;
}
#endif

int board_early_init_r(void)
{
	struct udevice *dev;
	int ret;

#if !CONFIG_IS_ENABLED(ROCKCHIP_BACK_TO_BROM)
	if (!fdt_node_check_compatible(gd->fdt_blob, 0, "google,veyron")) {
		ret = veyron_init();
		if (ret)
			return ret;
	}
#endif
	/*
	 * This init is done in SPL, but when chain-loading U-Boot SPL will
	 * have been skipped. Allow the clock driver to check if it needs
	 * setting up.
	 */
	ret = rockchip_get_clk(&dev);
	if (ret) {
		debug("CLK init failed: %d\n", ret);
		return ret;
	}

	return 0;
}
#endif
