// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Neil Armstrong <neil.armstrong@linaro.org>
 */

#include <common.h>
#include <asm/arch/boot.h>
#include <power/regulator.h>

int mmc_get_env_dev(void)
{
	if (meson_get_boot_device() == BOOT_DEVICE_EMMC)
		return 1;
	return 0;
}

int board_init(void)
{
	regulators_enable_boot_on(_DEBUG);

	return 0;
}
