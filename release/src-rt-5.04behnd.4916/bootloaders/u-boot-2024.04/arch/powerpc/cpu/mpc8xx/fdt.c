// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 (C) Bryan O'Donoghue
 *
 * Code copied & edited from Freescale mpc85xx stuff.
 */

#include <time.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

void ft_cpu_setup(void *blob, struct bd_info *bd)
{
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "timebase-frequency", get_tbclk(), 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "bus-frequency", bd->bi_busfreq, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "clock-frequency", bd->bi_intfreq, 1);
	do_fixup_by_compat_u32(blob, "fsl,pq1-soc", "clock-frequency",
			       bd->bi_intfreq, 1);
	do_fixup_by_compat_u32(blob, "fsl,cpm-brg", "clock-frequency",
			       gd->arch.brg_clk, 1);

	fdt_fixup_memory(blob, (u64)gd->ram_base, (u64)gd->ram_size);
}
