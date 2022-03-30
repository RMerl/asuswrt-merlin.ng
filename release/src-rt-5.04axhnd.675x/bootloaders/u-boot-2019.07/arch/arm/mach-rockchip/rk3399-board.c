// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <asm/arch-rockchip/boot_mode.h>

int board_late_init(void)
{
	setup_boot_mode();
	return 0;
}
