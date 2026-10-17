// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021,2023 Renesas Electronics Corporation
 *
 */

#include <mach/renesas.h>
#include <asm/io.h>
#include <linux/libfdt.h>

#define SYSC_LSI_DEVID		0x11020A04

/* If the firmware passed a device tree, use it for soc identification. */
extern u64 rcar_atf_boot_args[];

/* CPU information table */
struct tfa_info {
	const char *soc_name;
	const char *cpu_name;
	u32 cpu_type;
};

static const struct tfa_info tfa_info[] = {
	{ "renesas,r9a07g044l2", "R9A07G044L", RENESAS_CPU_TYPE_R9A07G044L },
};

static const struct tfa_info invalid_tfa_info = { NULL, "(invalid)", 0 };

static const struct tfa_info *get_tfa_info(void)
{
	void *atf_fdt_blob = (void *)(rcar_atf_boot_args[1]);

	if (fdt_magic(atf_fdt_blob) == FDT_MAGIC) {
		unsigned int i;
		for (i = 0; i < ARRAY_SIZE(tfa_info); i++) {
			if (!fdt_node_check_compatible(atf_fdt_blob, 0,
						       tfa_info[i].soc_name))
				return &tfa_info[i];
		}
	}

	return &invalid_tfa_info;
}

const u8 *rzg_get_cpu_name(void)
{
	return get_tfa_info()->cpu_name;
}

u32 renesas_get_cpu_type(void)
{
	return get_tfa_info()->cpu_type;
}

u32 renesas_get_cpu_rev_integer(void)
{
	return (readl(SYSC_LSI_DEVID) >> 28) + 1;
}

u32 renesas_get_cpu_rev_fraction(void)
{
	return 0;
}
