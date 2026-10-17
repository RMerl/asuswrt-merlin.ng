// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <env_internal.h>
#include <asm/global_data.h>
#include <linux/stddef.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Because we only ever have the default environment available we must mark
 * it as invalid.
 */
static int env_nowhere_init(void)
{
	gd->env_valid = ENV_INVALID;

	return 0;
}

static int env_nowhere_load(void)
{
	/*
	 * For SPL, setting env_valid = ENV_INVALID is enough, as env_get()
	 * searches default_environment array in that case.
	 * For U-Boot proper, import the default environment to allow reload.
	 */
	if (!IS_ENABLED(CONFIG_SPL_BUILD))
		env_set_default(NULL, 0);

	gd->env_valid = ENV_INVALID;

	return 0;
}

U_BOOT_ENV_LOCATION(nowhere) = {
	.location	= ENVL_NOWHERE,
	.init		= env_nowhere_init,
	.load		= env_nowhere_load,
	ENV_NAME("nowhere")
};
