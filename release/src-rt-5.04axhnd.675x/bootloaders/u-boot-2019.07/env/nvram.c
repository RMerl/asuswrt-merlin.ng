// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 */

/*
 * 09-18-2001 Andreas Heppel, Sysgo RTS GmbH <aheppel@sysgo.de>
 *
 * It might not be possible in all cases to use 'memcpy()' to copy
 * the environment to NVRAM, as the NVRAM might not be mapped into
 * the memory space. (I.e. this is the case for the BAB750). In those
 * cases it might be possible to access the NVRAM using a different
 * method. For example, the RTC on the BAB750 is accessible in IO
 * space using its address and data registers. To enable usage of
 * NVRAM in those cases I invented the functions 'nvram_read()' and
 * 'nvram_write()', which will be activated upon the configuration
 * #define CONFIG_SYS_NVRAM_ACCESS_ROUTINE. Note, that those functions are
 * strongly dependent on the used HW, and must be redefined for each
 * board that wants to use them.
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <search.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_NVRAM_ACCESS_ROUTINE
extern void *nvram_read(void *dest, const long src, size_t count);
extern void nvram_write(long dest, const void *src, size_t count);
#else
env_t *env_ptr = (env_t *)CONFIG_ENV_ADDR;
#endif

#ifdef CONFIG_SYS_NVRAM_ACCESS_ROUTINE
/** Call this function from overridden env_get_char_spec() if you need
 * this functionality.
 */
int env_nvram_get_char(int index)
{
	uchar c;

	nvram_read(&c, CONFIG_ENV_ADDR + index, 1);

	return c;
}
#endif

static int env_nvram_load(void)
{
	char buf[CONFIG_ENV_SIZE];

#if defined(CONFIG_SYS_NVRAM_ACCESS_ROUTINE)
	nvram_read(buf, CONFIG_ENV_ADDR, CONFIG_ENV_SIZE);
#else
	memcpy(buf, (void *)CONFIG_ENV_ADDR, CONFIG_ENV_SIZE);
#endif
	return env_import(buf, 1);
}

static int env_nvram_save(void)
{
	env_t	env_new;
	int	rcode = 0;

	rcode = env_export(&env_new);
	if (rcode)
		return rcode;

#ifdef CONFIG_SYS_NVRAM_ACCESS_ROUTINE
	nvram_write(CONFIG_ENV_ADDR, &env_new, CONFIG_ENV_SIZE);
#else
	if (memcpy((char *)CONFIG_ENV_ADDR, &env_new, CONFIG_ENV_SIZE) == NULL)
		rcode = 1;
#endif
	return rcode;
}

/*
 * Initialize Environment use
 *
 * We are still running from ROM, so data use is limited
 */
static int env_nvram_init(void)
{
#if defined(CONFIG_SYS_NVRAM_ACCESS_ROUTINE)
	ulong crc;
	uchar data[ENV_SIZE];

	nvram_read(&crc, CONFIG_ENV_ADDR, sizeof(ulong));
	nvram_read(data, CONFIG_ENV_ADDR + sizeof(ulong), ENV_SIZE);

	if (crc32(0, data, ENV_SIZE) == crc) {
		gd->env_addr	= (ulong)CONFIG_ENV_ADDR + sizeof(long);
#else
	if (crc32(0, env_ptr->data, ENV_SIZE) == env_ptr->crc) {
		gd->env_addr	= (ulong)&env_ptr->data;
#endif
		gd->env_valid = ENV_VALID;
	} else {
		gd->env_addr	= (ulong)&default_environment[0];
		gd->env_valid	= ENV_INVALID;
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
