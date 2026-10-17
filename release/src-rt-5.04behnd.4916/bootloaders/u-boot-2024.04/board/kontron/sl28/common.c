// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>

#include "sl28.h"

DECLARE_GLOBAL_DATA_PTR;

u32 get_lpuart_clk(void)
{
	return gd->bus_clk / CONFIG_SYS_FSL_LPUART_CLK_DIV;
}

enum boot_source sl28_boot_source(void)
{
	u32 rcw_src = in_le32(DCFG_BASE + DCFG_PORSR1) & DCFG_PORSR1_RCW_SRC;

	switch (rcw_src) {
	case DCFG_PORSR1_RCW_SRC_SDHC1:
		return BOOT_SOURCE_SDHC;
	case DCFG_PORSR1_RCW_SRC_SDHC2:
		return BOOT_SOURCE_MMC;
	case DCFG_PORSR1_RCW_SRC_I2C:
		return BOOT_SOURCE_I2C;
	case DCFG_PORSR1_RCW_SRC_FSPI_NOR:
		return BOOT_SOURCE_SPI;
	default:
		debug("unknown bootsource (%08x)\n", rcw_src);
		return BOOT_SOURCE_UNKNOWN;
	}
}
