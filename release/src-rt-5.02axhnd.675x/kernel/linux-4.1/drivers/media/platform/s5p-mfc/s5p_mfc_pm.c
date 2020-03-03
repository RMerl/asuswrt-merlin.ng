/*
 * linux/drivers/media/platform/s5p-mfc/s5p_mfc_pm.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include "s5p_mfc_common.h"
#include "s5p_mfc_debug.h"
#include "s5p_mfc_pm.h"

#define MFC_GATE_CLK_NAME	"mfc"
#define MFC_SCLK_NAME		"sclk_mfc"
#define MFC_SCLK_RATE		(200 * 1000000)

#define CLK_DEBUG

static struct s5p_mfc_pm *pm;
static struct s5p_mfc_dev *p_dev;

#ifdef CLK_DEBUG
static atomic_t clk_ref;
#endif

int s5p_mfc_init_pm(struct s5p_mfc_dev *dev)
{
	int ret = 0;

	pm = &dev->pm;
	p_dev = dev;
	pm->clock_gate = clk_get(&dev->plat_dev->dev, MFC_GATE_CLK_NAME);
	if (IS_ERR(pm->clock_gate)) {
		mfc_err("Failed to get clock-gating control\n");
		ret = PTR_ERR(pm->clock_gate);
		goto err_g_ip_clk;
	}

	ret = clk_prepare(pm->clock_gate);
	if (ret) {
		mfc_err("Failed to prepare clock-gating control\n");
		goto err_p_ip_clk;
	}

	if (dev->variant->version != MFC_VERSION_V6) {
		pm->clock = clk_get(&dev->plat_dev->dev, MFC_SCLK_NAME);
		if (IS_ERR(pm->clock)) {
			mfc_info("Failed to get MFC special clock control\n");
		} else {
			clk_set_rate(pm->clock, MFC_SCLK_RATE);
			ret = clk_prepare_enable(pm->clock);
			if (ret) {
				mfc_err("Failed to enable MFC special clock\n");
				goto err_s_clk;
			}
		}
	}

	atomic_set(&pm->power, 0);
#ifdef CONFIG_PM
	pm->device = &dev->plat_dev->dev;
	pm_runtime_enable(pm->device);
#endif
#ifdef CLK_DEBUG
	atomic_set(&clk_ref, 0);
#endif
	return 0;

err_s_clk:
	clk_put(pm->clock);
err_p_ip_clk:
	clk_put(pm->clock_gate);
err_g_ip_clk:
	return ret;
}

void s5p_mfc_final_pm(struct s5p_mfc_dev *dev)
{
	if (dev->variant->version != MFC_VERSION_V6 &&
	    pm->clock) {
		clk_disable_unprepare(pm->clock);
		clk_put(pm->clock);
	}
	clk_unprepare(pm->clock_gate);
	clk_put(pm->clock_gate);
#ifdef CONFIG_PM
	pm_runtime_disable(pm->device);
#endif
}

int s5p_mfc_clock_on(void)
{
	int ret;
#ifdef CLK_DEBUG
	atomic_inc(&clk_ref);
	mfc_debug(3, "+ %d\n", atomic_read(&clk_ref));
#endif
	ret = clk_enable(pm->clock_gate);
	return ret;
}

void s5p_mfc_clock_off(void)
{
#ifdef CLK_DEBUG
	atomic_dec(&clk_ref);
	mfc_debug(3, "- %d\n", atomic_read(&clk_ref));
#endif
	clk_disable(pm->clock_gate);
}

int s5p_mfc_power_on(void)
{
#ifdef CONFIG_PM
	return pm_runtime_get_sync(pm->device);
#else
	atomic_set(&pm->power, 1);
	return 0;
#endif
}

int s5p_mfc_power_off(void)
{
#ifdef CONFIG_PM
	return pm_runtime_put_sync(pm->device);
#else
	atomic_set(&pm->power, 0);
	return 0;
#endif
}


