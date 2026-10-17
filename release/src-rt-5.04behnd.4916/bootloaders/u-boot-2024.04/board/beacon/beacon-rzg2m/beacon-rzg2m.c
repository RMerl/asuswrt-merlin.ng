// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Compass Electronics Group, LLC
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;

	return 0;
}

#if IS_ENABLED(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	if (!strcmp(rzg_get_cpu_name(), "R8A774A1") && !strcmp(name, "r8a774a1-beacon-rzg2m-kit"))
		return 0;

	if (!strcmp(rzg_get_cpu_name(), "R8A774B1") && !strcmp(name, "r8a774b1-beacon-rzg2n-kit"))
		return 0;

	if (!strcmp(rzg_get_cpu_name(), "R8A774E1") && !strcmp(name, "r8a774e1-beacon-rzg2h-kit"))
		return 0;

	return -1;
}
#endif
