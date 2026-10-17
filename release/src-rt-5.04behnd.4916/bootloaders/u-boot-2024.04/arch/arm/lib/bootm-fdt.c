// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Google Inc.
 *
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 *  - Added prep subcommand support
 *  - Reorganized source - modeled after powerpc version
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2001  Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 */

#include <common.h>
#include <fdt_support.h>
#ifdef CONFIG_ARMV7_NONSEC
#include <asm/armv7.h>
#endif
#include <asm/global_data.h>
#include <asm/psci.h>
#include <asm/spin_table.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_FMAN_ENET
__weak int fdt_update_ethernet_dt(void *blob)
{
	return 0;
}
#endif

int arch_fixup_fdt(void *blob)
{
	__maybe_unused int ret = 0;
#if defined(CONFIG_ARMV7_NONSEC) || defined(CONFIG_OF_LIBFDT)
	struct bd_info *bd = gd->bd;
	int bank;
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start[bank] = bd->bi_dram[bank].start;
		size[bank] = bd->bi_dram[bank].size;
#ifdef CONFIG_ARMV7_NONSEC
		ret = armv7_apply_memory_carveout(&start[bank], &size[bank]);
		if (ret)
			return ret;
#endif
	}

#ifdef CONFIG_OF_LIBFDT
	ret = fdt_fixup_memory_banks(blob, start, size, CONFIG_NR_DRAM_BANKS);
	if (ret)
		return ret;
#endif

#ifdef CONFIG_ARMV8_SPIN_TABLE
	ret = spin_table_update_dt(blob);
	if (ret)
		return ret;
#endif

#if defined(CONFIG_ARMV7_NONSEC) || defined(CONFIG_ARMV8_PSCI) || \
	CONFIG_IS_ENABLED(SEC_FIRMWARE_ARMV8_PSCI)
	ret = psci_update_dt(blob);
	if (ret)
		return ret;
#endif
#endif

#ifdef CONFIG_FMAN_ENET
	ret = fdt_update_ethernet_dt(blob);
	if (ret)
		return ret;
#endif
	return 0;
}
