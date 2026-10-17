// SPDX-License-Identifier: GPL-2.0+
/*
 * Board init file for Skyworth HC2910 2AGHD05
 */

#include <common.h>
#include <fdtdec.h>
#include <init.h>
#include <asm/system.h>
#include <linux/io.h>

#define HI3798MV200_PERI_CTRL_BASE 0xf8a20000
#define SDIO0_LDO_OFFSET 0x11c

static int sdio0_set_ldo(void)
{
	// SDIO LDO bypassed, 3.3V
	writel(HI3798MV200_PERI_CTRL_BASE + SDIO0_LDO_OFFSET, 0x60);
	return 0;
}

int board_init(void)
{
	sdio0_set_ldo();
	return 0;
}
