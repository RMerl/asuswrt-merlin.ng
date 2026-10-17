// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Renesas Electronics Corporation
 *
 */

#include <linux/libfdt.h>

/* If the firmware passed a device tree, use it for soc identification. */
extern u64 rcar_atf_boot_args[];

/* CPU information table */
static const struct {
	char *soc_name;
	u8 cpu_name[10];
} tfa_info[] = {
	{ "renesas,r8a774a1", "R8A774A1" },
	{ "renesas,r8a774b1", "R8A774B1" },
	{ "renesas,r8a774c0", "R8A774C0" },
	{ "renesas,r8a774e1", "R8A774E1" }
};

const u8 *rzg_get_cpu_name(void)
{
	void *atf_fdt_blob = (void *)(rcar_atf_boot_args[1]);
	bool ret = false;
	int i;

	if (fdt_magic(atf_fdt_blob) != FDT_MAGIC)
		return NULL;

	for (i = 0; i < ARRAY_SIZE(tfa_info); i++) {
		if (fdt_node_check_compatible(atf_fdt_blob, 0,
					      tfa_info[i].soc_name) == 0) {
			ret = true;
			break;
		}
	}

	return ret ? tfa_info[i].cpu_name : NULL;
}
