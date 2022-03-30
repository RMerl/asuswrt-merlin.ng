// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 */

#include <common.h>
#include <efi.h>
#include <asm/u-boot-x86.h>

DECLARE_GLOBAL_DATA_PTR;

ulong board_get_usable_ram_top(ulong total_size)
{
	return (ulong)efi_get_ram_base() + gd->ram_size;
}

int dram_init(void)
{
	/* gd->ram_size is set as part of EFI init */

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = efi_get_ram_base();
	gd->bd->bi_dram[0].size = CONFIG_EFI_RAM_SIZE;

	return 0;
}
