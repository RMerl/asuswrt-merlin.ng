// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <mmc.h>
#include <spl.h>

u32 spl_boot_mode(const u32 boot_device)
{
	struct mmc *mmc;

	/*
	 * work around a bug in the Boot ROM of LD4, Pro4, and sLD8:
	 *
	 * The boot ROM in these SoCs breaks the PARTITION_CONFIG [179] of
	 * Extended CSD register; when switching to the Boot Partition 1, the
	 * Boot ROM should issue the SWITCH command (CMD6) with Set Bits for
	 * the Access Bits, but in fact it uses Write Byte for the Access Bits.
	 * As a result, the BOOT_PARTITION_ENABLE field of the PARTITION_CONFIG
	 * is lost.  This bug was fixed for PH1-Pro5 and later SoCs.
	 *
	 * Fixup mmc->part_config here because it is used to determine the
	 * partition which the U-Boot image is read from.
	 */
	mmc = find_mmc_device(0);
	mmc->part_config &= ~EXT_CSD_BOOT_PART_NUM(PART_ACCESS_MASK);
	mmc->part_config |= EXT_CSD_BOOT_PARTITION_ENABLE;

	return MMCSD_MODE_EMMCBOOT;
}
