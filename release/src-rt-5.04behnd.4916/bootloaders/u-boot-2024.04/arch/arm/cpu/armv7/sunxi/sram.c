// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * SRAM init for older sunxi SoCs.
 */

#include <common.h>
#include <init.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>

void sunxi_sram_init(void)
{
	/*
	 * Undocumented magic taken from boot0, without this DRAM
	 * access gets messed up (seems cache related).
	 * The boot0 sources describe this as: "config ema for cache sram"
	 * Newer SoCs (A83T, H3 and anything beyond) don't need this anymore.
	 */
	if (IS_ENABLED(CONFIG_MACH_SUN6I))
		setbits_le32(SUNXI_SRAMC_BASE + 0x44, 0x1800);

	if (IS_ENABLED(CONFIG_MACH_SUN8I)) {
		uint version = sunxi_get_sram_id();

		if (IS_ENABLED(CONFIG_MACH_SUN8I_A23)) {
			if (version == 0x1650)
				setbits_le32(SUNXI_SRAMC_BASE + 0x44, 0x1800);
			else /* 0x1661 ? */
				setbits_le32(SUNXI_SRAMC_BASE + 0x44, 0xc0);
		} else if (IS_ENABLED(CONFIG_MACH_SUN8I_A33)) {
			if (version != 0x1667)
				setbits_le32(SUNXI_SRAMC_BASE + 0x44, 0xc0);
		}
	}
}
