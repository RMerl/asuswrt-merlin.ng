// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Kontron Electronics GmbH
 */

#include <asm/types.h>
#include <asm/arch/sys_proto.h>

#include <sl-mx6ul-common.h>

bool sl_mx6ul_is_spi_nor_boot(void)
{
	u32 bmode = imx6_src_get_boot_mode();

	/*
	 * Check if "EEPROM Recovery" enabled and ECSPI2_CONREG not 0x0.
	 * If this is the case and U-Boot didn't initialize the SPI bus
	 * yet, we can safely assume that we are booting from SPI NOR.
	 */
	if ((bmode & 0x40000000) && readl(0x0200c008))
		return true;

	return false;
}
