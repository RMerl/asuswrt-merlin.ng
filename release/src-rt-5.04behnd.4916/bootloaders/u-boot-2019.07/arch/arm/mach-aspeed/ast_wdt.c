// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Google, Inc
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/wdt.h>
#include <linux/err.h>

u32 ast_reset_mode_from_flags(ulong flags)
{
	return flags & WDT_CTRL_RESET_MASK;
}

u32 ast_reset_mask_from_flags(ulong flags)
{
	return flags >> 2;
}

ulong ast_flags_from_reset_mode_mask(u32 reset_mode, u32 reset_mask)
{
	ulong ret = reset_mode & WDT_CTRL_RESET_MASK;

	if (ret == WDT_CTRL_RESET_SOC)
		ret |= (reset_mask << 2);

	return ret;
}
