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
#include <environment.h>
#include <linux/stddef.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Because we only ever have the default environment available we must mark
 * it as invalid.
 */
static int env_nowhere_init(void)
{
	gd->env_addr	= (ulong)&default_environment[0];
	gd->env_valid	= ENV_INVALID;

	return 0;
}

U_BOOT_ENV_LOCATION(nowhere) = {
	.location	= ENVL_NOWHERE,
	.init		= env_nowhere_init,
	ENV_NAME("nowhere")
};
