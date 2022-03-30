// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <config.h>
#include <asm/arch/cpu.h>

void spl_board_init(void)
{
#if CONFIG_SPL_BOOT_DEVICE == SPL_BOOT_SPI_NOR_FLASH
	u32 *bootrom_save = (u32 *)CONFIG_SPL_BOOTROM_SAVE;
	u32 *regs = (u32 *)(*bootrom_save);

	printf("Returning to BootROM (return address %08x)...\n", regs[13]);
	return_to_bootrom();
#endif
}
