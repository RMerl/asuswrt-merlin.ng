// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <stm32_rcc.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

struct stm32_rcc_clk stm32_rcc_clk_f42x = {
	.drv_name = "stm32fx_rcc_clock",
	.soc = STM32F42X,
};

struct stm32_rcc_clk stm32_rcc_clk_f469 = {
	.drv_name = "stm32fx_rcc_clock",
	.soc = STM32F469,
};

struct stm32_rcc_clk stm32_rcc_clk_f7 = {
	.drv_name = "stm32fx_rcc_clock",
	.soc = STM32F7,
};

struct stm32_rcc_clk stm32_rcc_clk_h7 = {
	.drv_name = "stm32h7_rcc_clock",
};

struct stm32_rcc_clk stm32_rcc_clk_mp1 = {
	.drv_name = "stm32mp1_clk",
	.soc = STM32MP1,
};

static int stm32_rcc_bind(struct udevice *dev)
{
	struct udevice *child;
	struct driver *drv;
	struct stm32_rcc_clk *rcc_clk =
		(struct stm32_rcc_clk *)dev_get_driver_data(dev);
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);
	drv = lists_driver_lookup_name(rcc_clk->drv_name);
	if (!drv) {
		debug("Cannot find driver '%s'\n", rcc_clk->drv_name);
		return -ENOENT;
	}

	ret = device_bind_with_driver_data(dev, drv, rcc_clk->drv_name,
					   rcc_clk->soc,
					   dev_ofnode(dev), &child);

	if (ret)
		return ret;

	drv = lists_driver_lookup_name("stm32_rcc_reset");
	if (!drv) {
		dev_err(dev, "Cannot find driver stm32_rcc_reset'\n");
		return -ENOENT;
	}

	return device_bind_with_driver_data(dev, drv, "stm32_rcc_reset",
					    rcc_clk->soc,
					    dev_ofnode(dev), &child);
}

static const struct misc_ops stm32_rcc_ops = {
};

static const struct udevice_id stm32_rcc_ids[] = {
	{.compatible = "st,stm32f42xx-rcc", .data = (ulong)&stm32_rcc_clk_f42x },
	{.compatible = "st,stm32f469-rcc", .data = (ulong)&stm32_rcc_clk_f469 },
	{.compatible = "st,stm32f746-rcc", .data = (ulong)&stm32_rcc_clk_f7 },
	{.compatible = "st,stm32h743-rcc", .data = (ulong)&stm32_rcc_clk_h7 },
	{.compatible = "st,stm32mp1-rcc", .data = (ulong)&stm32_rcc_clk_mp1 },
	{ }
};

U_BOOT_DRIVER(stm32_rcc) = {
	.name		= "stm32-rcc",
	.id		= UCLASS_MISC,
	.of_match	= stm32_rcc_ids,
	.bind		= stm32_rcc_bind,
	.ops		= &stm32_rcc_ops,
};
