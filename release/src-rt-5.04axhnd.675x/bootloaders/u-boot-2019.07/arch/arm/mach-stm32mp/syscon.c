// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <asm/arch/stm32.h>

static const struct udevice_id stm32mp_syscon_ids[] = {
	{ .compatible = "st,stm32mp1-etzpc", .data = STM32MP_SYSCON_ETZPC },
	{ .compatible = "st,stm32mp1-pwr", .data = STM32MP_SYSCON_PWR },
	{ .compatible = "st,stm32-stgen", .data = STM32MP_SYSCON_STGEN },
	{ .compatible = "st,stm32mp157-syscfg",
	  .data = STM32MP_SYSCON_SYSCFG },
	{ }
};

U_BOOT_DRIVER(syscon_stm32mp) = {
	.name = "stmp32mp_syscon",
	.id = UCLASS_SYSCON,
	.of_match = stm32mp_syscon_ids,
	.bind = dm_scan_fdt_dev,
};
