// SPDX-License-Identifier: GPL-2.0+
/*
 * RZ/G2L board support.
 * Copyright (C) 2023 Renesas Electronics Corporation
 */

#include <fdtdec.h>
#include <linux/libfdt.h>

#if IS_ENABLED(CONFIG_MULTI_DTB_FIT)
/* If the firmware passed a device tree, use it for board identification. */
extern u64 rcar_atf_boot_args[];

static bool is_rzg2l_board(const char *board_name)
{
	void *atf_fdt_blob = (void *)(rcar_atf_boot_args[1]);

	return fdt_node_check_compatible(atf_fdt_blob, 0, board_name) == 0;
}

int board_fit_config_name_match(const char *name)
{
	void *atf_fdt_blob = (void *)(rcar_atf_boot_args[1]);

	if (fdt_magic(atf_fdt_blob) != FDT_MAGIC)
		return -1;

	if (is_rzg2l_board("renesas,r9a07g044l2"))
		return strcmp(name, "r9a07g044l2-smarc");

	return -1;
}
#endif

static void apply_atf_overlay(void *fdt_blob)
{
	void *atf_fdt_blob = (void *)(rcar_atf_boot_args[1]);

	if (fdt_magic(atf_fdt_blob) == FDT_MAGIC)
		fdt_overlay_apply_node(fdt_blob, 0, atf_fdt_blob, 0);
}

int fdtdec_board_setup(const void *fdt_blob)
{
	apply_atf_overlay((void *)fdt_blob);

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}

int board_init(void)
{
	return 0;
}
