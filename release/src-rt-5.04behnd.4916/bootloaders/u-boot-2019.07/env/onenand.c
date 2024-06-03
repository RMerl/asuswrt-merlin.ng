// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010 DENX Software Engineering
 * Wolfgang Denk <wd@denx.de>
 *
 * (C) Copyright 2005-2009 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <search.h>
#include <errno.h>
#include <onenand_uboot.h>

#include <linux/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

#define ONENAND_MAX_ENV_SIZE	CONFIG_ENV_SIZE
#define ONENAND_ENV_SIZE(mtd)	(ONENAND_MAX_ENV_SIZE - ENV_HEADER_SIZE)

DECLARE_GLOBAL_DATA_PTR;

static int env_onenand_load(void)
{
	struct mtd_info *mtd = &onenand_mtd;
#ifdef CONFIG_ENV_ADDR_FLEX
	struct onenand_chip *this = &onenand_chip;
#endif
	int rc;
	size_t retlen;
#ifdef ENV_IS_EMBEDDED
	char *buf = (char *)&environment;
#else
	loff_t env_addr = CONFIG_ENV_ADDR;
	char onenand_env[ONENAND_MAX_ENV_SIZE];
	char *buf = (char *)&onenand_env[0];
#endif /* ENV_IS_EMBEDDED */

#ifndef ENV_IS_EMBEDDED
# ifdef CONFIG_ENV_ADDR_FLEX
	if (FLEXONENAND(this))
		env_addr = CONFIG_ENV_ADDR_FLEX;
# endif
	/* Check OneNAND exist */
	if (mtd->writesize)
		/* Ignore read fail */
		mtd_read(mtd, env_addr, ONENAND_MAX_ENV_SIZE,
				&retlen, (u_char *)buf);
	else
		mtd->writesize = MAX_ONENAND_PAGESIZE;
#endif /* !ENV_IS_EMBEDDED */

	rc = env_import(buf, 1);
	if (!rc)
		gd->env_valid = ENV_VALID;

	return rc;
}

static int env_onenand_save(void)
{
	env_t	env_new;
	int ret;
	struct mtd_info *mtd = &onenand_mtd;
#ifdef CONFIG_ENV_ADDR_FLEX
	struct onenand_chip *this = &onenand_chip;
#endif
	loff_t	env_addr = CONFIG_ENV_ADDR;
	size_t	retlen;
	struct erase_info instr = {
		.callback	= NULL,
	};

	ret = env_export(&env_new);
	if (ret)
		return ret;

	instr.len = CONFIG_ENV_SIZE;
#ifdef CONFIG_ENV_ADDR_FLEX
	if (FLEXONENAND(this)) {
		env_addr = CONFIG_ENV_ADDR_FLEX;
		instr.len = CONFIG_ENV_SIZE_FLEX;
		instr.len <<= onenand_mtd.eraseregions[0].numblocks == 1 ?
				1 : 0;
	}
#endif
	instr.addr = env_addr;
	instr.mtd = mtd;
	if (mtd_erase(mtd, &instr)) {
		printf("OneNAND: erase failed at 0x%08llx\n", env_addr);
		return 1;
	}

	if (mtd_write(mtd, env_addr, ONENAND_MAX_ENV_SIZE, &retlen,
			(u_char *)&env_new)) {
		printf("OneNAND: write failed at 0x%llx\n", instr.addr);
		return 2;
	}

	return 0;
}

U_BOOT_ENV_LOCATION(onenand) = {
	.location	= ENVL_ONENAND,
	ENV_NAME("OneNAND")
	.load		= env_onenand_load,
	.save		= env_save_ptr(env_onenand_save),
};
