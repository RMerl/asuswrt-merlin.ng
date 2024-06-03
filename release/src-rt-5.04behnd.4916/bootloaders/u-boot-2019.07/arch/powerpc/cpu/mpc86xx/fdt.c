// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008, 2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/mp.h>

extern void ft_fixup_num_cores(void *blob);
extern void ft_srio_setup(void *blob);

void ft_cpu_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_MP
	int off;
	u32 bootpg = determine_mp_bootpg(NULL);
#endif

	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "timebase-frequency", bd->bi_busfreq / 4, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "bus-frequency", bd->bi_busfreq, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "clock-frequency", bd->bi_intfreq, 1);
	do_fixup_by_prop_u32(blob, "device_type", "soc", 4,
			     "bus-frequency", bd->bi_busfreq, 1);

	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);

#ifdef CONFIG_SYS_NS16550
	do_fixup_by_compat_u32(blob, "ns16550",
			       "clock-frequency", CONFIG_SYS_NS16550_CLK, 1);
#endif

#ifdef CONFIG_MP
	/* Reserve the boot page so OSes dont use it */
	off = fdt_add_mem_rsv(blob, bootpg, (u64)4096);
	if (off < 0)
		printf("%s: %s\n", __FUNCTION__, fdt_strerror(off));

	ft_fixup_num_cores(blob);
#endif

#ifdef CONFIG_SYS_SRIO
	ft_srio_setup(blob);
#endif
}
