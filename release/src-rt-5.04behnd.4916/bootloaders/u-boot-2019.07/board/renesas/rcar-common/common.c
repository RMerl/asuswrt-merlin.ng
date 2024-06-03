// SPDX-License-Identifier: GPL-2.0
/*
 * board/renesas/rcar-common/common.c
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 * Copyright (C) 2013 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2015 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <common.h>
#include <asm/arch/rmobile.h>

#ifdef CONFIG_RCAR_GEN3

DECLARE_GLOBAL_DATA_PTR;

/* If the firmware passed a device tree use it for U-Boot DRAM setup. */
extern u64 rcar_atf_boot_args[];

int dram_init(void)
{
	const void *atf_fdt_blob = (const void *)(rcar_atf_boot_args[1]);
	const void *blob;

	/* Check if ATF passed us DTB. If not, fall back to builtin DTB. */
	if (fdt_magic(atf_fdt_blob) == FDT_MAGIC)
		blob = atf_fdt_blob;
	else
		blob = gd->fdt_blob;

	return fdtdec_setup_mem_size_base_fdt(blob);
}

int dram_init_banksize(void)
{
	const void *atf_fdt_blob = (const void *)(rcar_atf_boot_args[1]);
	const void *blob;

	/* Check if ATF passed us DTB. If not, fall back to builtin DTB. */
	if (fdt_magic(atf_fdt_blob) == FDT_MAGIC)
		blob = atf_fdt_blob;
	else
		blob = gd->fdt_blob;

	fdtdec_setup_memory_banksize_fdt(blob);

	return 0;
}
#endif
