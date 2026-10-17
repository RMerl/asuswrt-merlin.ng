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
#include <search.h>
#include <errno.h>
#include <u-boot/crc.h>

DECLARE_GLOBAL_DATA_PTR;

static env_t *env_ptr = (env_t *)CONFIG_ENV_ADDR;

static int env_nvram_load(void)
{
	char buf[CONFIG_ENV_SIZE];

	memcpy(buf, (void *)CONFIG_ENV_ADDR, CONFIG_ENV_SIZE);

	return env_import(buf, 1, H_EXTERNAL);
}

static int env_nvram_save(void)
{
	env_t	env_new;
	int	rcode = 0;

	rcode = env_export(&env_new);
	if (rcode)
		return rcode;

	if (memcpy((char *)CONFIG_ENV_ADDR, &env_new, CONFIG_ENV_SIZE) == NULL)
		rcode = 1;

	return rcode;
}

/*
 * Initialize Environment use
 *
 * We are still running from ROM, so data use is limited
 */
static int env_nvram_init(void)
{
	if (crc32(0, env_ptr->data, ENV_SIZE) == env_ptr->crc) {
		gd->env_addr = (ulong)&env_ptr->data;
		gd->env_valid = ENV_VALID;
	} else {
		gd->env_valid = ENV_INVALID;
	}

	return 0;
}

U_BOOT_ENV_LOCATION(nvram) = {
	.location	= ENVL_NVRAM,
	ENV_NAME("NVRAM")
	.load		= env_nvram_load,
	.save		= env_save_ptr(env_nvram_save),
	.init		= env_nvram_init,
};
