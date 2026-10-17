// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <syscon.h>
#include <asm/arch/stm32.h>

static const struct udevice_id stm32mp_syscon_ids[] = {
	{ .compatible = "st,stm32mp157-syscfg", .data = STM32MP_SYSCON_SYSCFG },
	{ .compatible = "st,stm32mp25-syscfg", .data = STM32MP_SYSCON_SYSCFG},
	{ }
};

static int stm32mp_syscon_probe(struct udevice *dev)
{
	struct clk_bulk clk_bulk;
	int ret;

	ret = clk_get_bulk(dev, &clk_bulk);
	if (!ret)
		clk_enable_bulk(&clk_bulk);

	return 0;
}

U_BOOT_DRIVER(syscon_stm32mp) = {
	.name = "stmp32mp_syscon",
	.id = UCLASS_SYSCON,
	.of_match = stm32mp_syscon_ids,
	.bind = dm_scan_fdt_dev,
	.probe = stm32mp_syscon_probe,
};
