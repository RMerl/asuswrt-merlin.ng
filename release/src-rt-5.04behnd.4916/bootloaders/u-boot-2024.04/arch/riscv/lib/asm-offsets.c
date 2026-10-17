// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * From arch/x86/lib/asm-offsets.c
 *
 * This program is used to generate definitions needed by
 * assembly language modules.
 */

#include <asm/global_data.h>
#include <linux/kbuild.h>

int main(void)
{
	DEFINE(GD_BOOT_HART, offsetof(gd_t, arch.boot_hart));
	DEFINE(GD_FIRMWARE_FDT_ADDR, offsetof(gd_t, arch.firmware_fdt_addr));
#if !CONFIG_IS_ENABLED(XIP)
#ifdef CONFIG_AVAILABLE_HARTS
	DEFINE(GD_AVAILABLE_HARTS, offsetof(gd_t, arch.available_harts));
#endif
#endif

	return 0;
}
