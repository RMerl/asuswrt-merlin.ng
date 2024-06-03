// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010-2016 Freescale Semiconductor, Inc.
 */

/* #define DEBUG */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <errno.h>
#include <memalign.h>
#include <sata.h>
#include <search.h>

#if defined(CONFIG_ENV_SIZE_REDUND) || defined(CONFIG_ENV_OFFSET_REDUND)
#error ENV REDUND not supported
#endif

#if !defined(CONFIG_ENV_OFFSET) || !defined(CONFIG_ENV_SIZE)
#error CONFIG_ENV_OFFSET or CONFIG_ENV_SIZE not defined
#endif

__weak int sata_get_env_dev(void)
{
	return CONFIG_SYS_SATA_ENV_DEV;
}

#ifdef CONFIG_CMD_SAVEENV
static inline int write_env(struct blk_desc *sata, unsigned long size,
			    unsigned long offset, void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start = ALIGN(offset, sata->blksz) / sata->blksz;
	blk_cnt   = ALIGN(size, sata->blksz) / sata->blksz;

	n = blk_dwrite(sata, blk_start, blk_cnt, buffer);

	return (n == blk_cnt) ? 0 : -1;
}

static int env_sata_save(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	struct blk_desc *sata = NULL;
	int env_sata, ret;

	if (sata_initialize())
		return 1;

	env_sata = sata_get_env_dev();

	sata = sata_get_dev(env_sata);
	if (sata == NULL) {
		printf("Unknown SATA(%d) device for environment!\n",
		       env_sata);
		return 1;
	}

	ret = env_export(env_new);
	if (ret)
		return 1;

	printf("Writing to SATA(%d)...", env_sata);
	if (write_env(sata, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, (u_char *)env_new)) {
		puts("failed\n");
		return 1;
	}

	puts("done\n");
	return 0;
}
#endif /* CONFIG_CMD_SAVEENV */

static inline int read_env(struct blk_desc *sata, unsigned long size,
			   unsigned long offset, void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start = ALIGN(offset, sata->blksz) / sata->blksz;
	blk_cnt   = ALIGN(size, sata->blksz) / sata->blksz;

	n = blk_dread(sata, blk_start, blk_cnt, buffer);

	return (n == blk_cnt) ? 0 : -1;
}

static void env_sata_load(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	struct blk_desc *sata = NULL;
	int env_sata;

	if (sata_initialize())
		return -EIO;

	env_sata = sata_get_env_dev();

	sata = sata_get_dev(env_sata);
	if (sata == NULL) {
		printf("Unknown SATA(%d) device for environment!\n", env_sata);
		return -EIO;
	}

	if (read_env(sata, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, buf)) {
		set_default_env(NULL, 0);
		return -EIO;
	}

	return env_import(buf, 1);
}

U_BOOT_ENV_LOCATION(sata) = {
	.location	= ENVL_ESATA,
	ENV_NAME("SATA")
	.load		= env_sata_load,
	.save		= env_save_ptr(env_sata_save),
};
