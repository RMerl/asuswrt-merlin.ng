// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2008
 * Stuart Wood, Lab X Technologies <stuart.wood@labxtechnologies.com>
 *
 * (C) Copyright 2004
 * Jian Zhang, Texas Instruments, jzhang@ti.com.
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
#include <malloc.h>
#include <memalign.h>
#include <nand.h>
#include <search.h>
#include <errno.h>
#include <u-boot/crc.h>

#if defined(CONFIG_CMD_SAVEENV) && defined(CONFIG_CMD_NAND) && \
		!defined(CONFIG_SPL_BUILD)
#define CMD_SAVEENV
#elif defined(CONFIG_ENV_OFFSET_REDUND) && !defined(CONFIG_SPL_BUILD)
#error CONFIG_ENV_OFFSET_REDUND must have CONFIG_CMD_SAVEENV & CONFIG_CMD_NAND
#endif

#if defined(ENV_IS_EMBEDDED)
static env_t *env_ptr = &environment;
#elif defined(CONFIG_NAND_ENV_DST)
static env_t *env_ptr = (env_t *)CONFIG_NAND_ENV_DST;
#endif /* ENV_IS_EMBEDDED */

DECLARE_GLOBAL_DATA_PTR;

/*
 * This is called before nand_init() so we can't read NAND to
 * validate env data.
 *
 * Mark it OK for now. env_relocate() in env_common.c will call our
 * relocate function which does the real validation.
 *
 * When using a NAND boot image (like sequoia_nand), the environment
 * can be embedded or attached to the U-Boot image in NAND flash.
 * This way the SPL loads not only the U-Boot image from NAND but
 * also the environment.
 */
static int env_nand_init(void)
{
#if defined(ENV_IS_EMBEDDED) || defined(CONFIG_NAND_ENV_DST)
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env1;

#ifdef CONFIG_ENV_OFFSET_REDUND
	env_t *tmp_env2;

	tmp_env2 = (env_t *)((ulong)env_ptr + CONFIG_ENV_SIZE);
	crc2_ok = crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc;
#endif
	tmp_env1 = env_ptr;
	crc1_ok = crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc;

	if (!crc1_ok && !crc2_ok) {
		gd->env_addr	= 0;
		gd->env_valid	= ENV_INVALID;

		return 0;
	} else if (crc1_ok && !crc2_ok) {
		gd->env_valid = ENV_VALID;
	}
#ifdef CONFIG_ENV_OFFSET_REDUND
	else if (!crc1_ok && crc2_ok) {
		gd->env_valid = ENV_REDUND;
	} else {
		/* both ok - check serial */
		if (tmp_env1->flags == 255 && tmp_env2->flags == 0)
			gd->env_valid = ENV_REDUND;
		else if (tmp_env2->flags == 255 && tmp_env1->flags == 0)
			gd->env_valid = ENV_VALID;
		else if (tmp_env1->flags > tmp_env2->flags)
			gd->env_valid = ENV_VALID;
		else if (tmp_env2->flags > tmp_env1->flags)
			gd->env_valid = ENV_REDUND;
		else /* flags are equal - almost impossible */
			gd->env_valid = ENV_VALID;
	}

	if (gd->env_valid == ENV_REDUND)
		env_ptr = tmp_env2;
	else
#endif
	if (gd->env_valid == ENV_VALID)
		env_ptr = tmp_env1;

	gd->env_addr = (ulong)env_ptr->data;

#else /* ENV_IS_EMBEDDED || CONFIG_NAND_ENV_DST */
	gd->env_valid	= ENV_INVALID;
#endif /* ENV_IS_EMBEDDED || CONFIG_NAND_ENV_DST */

	return 0;
}

#ifdef CMD_SAVEENV
/*
 * The legacy NAND code saved the environment in the first NAND device i.e.,
 * nand_dev_desc + 0. This is also the behaviour using the new NAND code.
 */
static int writeenv(size_t offset, u_char *buf)
{
	size_t end = offset + CONFIG_ENV_RANGE;
	size_t amount_saved = 0;
	size_t blocksize, len;
	struct mtd_info *mtd;
	u_char *char_ptr;

	mtd = get_nand_dev_by_index(0);
	if (!mtd)
		return 1;

	blocksize = mtd->erasesize;
	len = min(blocksize, (size_t)CONFIG_ENV_SIZE);

	while (amount_saved < CONFIG_ENV_SIZE && offset < end) {
		if (nand_block_isbad(mtd, offset)) {
			offset += blocksize;
		} else {
			char_ptr = &buf[amount_saved];
			if (nand_write(mtd, offset, &len, char_ptr))
				return 1;

			offset += blocksize;
			amount_saved += len;
		}
	}
	if (amount_saved != CONFIG_ENV_SIZE)
		return 1;

	return 0;
}

struct nand_env_location {
	const char *name;
	const nand_erase_options_t erase_opts;
};

static int erase_and_write_env(const struct nand_env_location *location,
		u_char *env_new)
{
	struct mtd_info *mtd;
	int ret = 0;

	mtd = get_nand_dev_by_index(0);
	if (!mtd)
		return 1;

	printf("Erasing %s...\n", location->name);
	if (nand_erase_opts(mtd, &location->erase_opts))
		return 1;

	printf("Writing to %s... ", location->name);
	ret = writeenv(location->erase_opts.offset, env_new);
	puts(ret ? "FAILED!\n" : "OK\n");

	return ret;
}

static int env_nand_save(void)
{
	int	ret = 0;
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	int	env_idx = 0;
	static const struct nand_env_location location[] = {
		{
			.name = "NAND",
			.erase_opts = {
				.length = CONFIG_ENV_RANGE,
				.offset = CONFIG_ENV_OFFSET,
			},
		},
#ifdef CONFIG_ENV_OFFSET_REDUND
		{
			.name = "redundant NAND",
			.erase_opts = {
				.length = CONFIG_ENV_RANGE,
				.offset = CONFIG_ENV_OFFSET_REDUND,
			},
		},
#endif
	};

	ret = env_export(env_new);
	if (ret)
		return ret;

#ifdef CONFIG_ENV_OFFSET_REDUND
	env_idx = (gd->env_valid == ENV_VALID);
#endif

	ret = erase_and_write_env(&location[env_idx], (u_char *)env_new);
#ifdef CONFIG_ENV_OFFSET_REDUND
	if (!ret) {
		/* preset other copy for next write */
		gd->env_valid = gd->env_valid == ENV_REDUND ? ENV_VALID :
				ENV_REDUND;
		return ret;
	}

	env_idx = (env_idx + 1) & 1;
	ret = erase_and_write_env(&location[env_idx], (u_char *)env_new);
	if (!ret)
		printf("Warning: primary env write failed,"
				" redundancy is lost!\n");
#endif

	return ret;
}
#endif /* CMD_SAVEENV */

#if defined(CONFIG_SPL_BUILD)
static int readenv(size_t offset, u_char *buf)
{
	return nand_spl_load_image(offset, CONFIG_ENV_SIZE, buf);
}
#else
static int readenv(size_t offset, u_char *buf)
{
	size_t end = offset + CONFIG_ENV_RANGE;
	size_t amount_loaded = 0;
	size_t blocksize, len;
	struct mtd_info *mtd;
	u_char *char_ptr;

	mtd = get_nand_dev_by_index(0);
	if (!mtd)
		return 1;

	blocksize = mtd->erasesize;
	len = min(blocksize, (size_t)CONFIG_ENV_SIZE);

	while (amount_loaded < CONFIG_ENV_SIZE && offset < end) {
		if (nand_block_isbad(mtd, offset)) {
			offset += blocksize;
		} else {
			char_ptr = &buf[amount_loaded];
			if (nand_read_skip_bad(mtd, offset,
					       &len, NULL,
					       mtd->size, char_ptr))
				return 1;

			offset += blocksize;
			amount_loaded += len;
		}
	}

	if (amount_loaded != CONFIG_ENV_SIZE)
		return 1;

	return 0;
}
#endif /* #if defined(CONFIG_SPL_BUILD) */

#ifdef CONFIG_ENV_OFFSET_OOB
int get_nand_env_oob(struct mtd_info *mtd, unsigned long *result)
{
	struct mtd_oob_ops ops;
	uint32_t oob_buf[ENV_OFFSET_SIZE / sizeof(uint32_t)];
	int ret;

	ops.datbuf	= NULL;
	ops.mode	= MTD_OOB_AUTO;
	ops.ooboffs	= 0;
	ops.ooblen	= ENV_OFFSET_SIZE;
	ops.oobbuf	= (void *)oob_buf;

	ret = mtd->read_oob(mtd, ENV_OFFSET_SIZE, &ops);
	if (ret) {
		printf("error reading OOB block 0\n");
		return ret;
	}

	if (oob_buf[0] == ENV_OOB_MARKER) {
		*result = ovoid ob_buf[1] * mtd->erasesize;
	} else if (oob_buf[0] == ENV_OOB_MARKER_OLD) {
		*result = oob_buf[1];
	} else {
		printf("No dynamic environment marker in OOB block 0\n");
		return -ENOENT;
	}

	return 0;
}
#endif

#ifdef CONFIG_ENV_OFFSET_REDUND
static int env_nand_load(void)
{
#if defined(ENV_IS_EMBEDDED)
	return 0;
#else
	int read1_fail, read2_fail;
	env_t *tmp_env1, *tmp_env2;
	int ret = 0;

	tmp_env1 = (env_t *)malloc(CONFIG_ENV_SIZE);
	tmp_env2 = (env_t *)malloc(CONFIG_ENV_SIZE);
	if (tmp_env1 == NULL || tmp_env2 == NULL) {
		puts("Can't allocate buffers for environment\n");
		env_set_default("malloc() failed", 0);
		ret = -EIO;
		goto done;
	}

	read1_fail = readenv(CONFIG_ENV_OFFSET, (u_char *) tmp_env1);
	read2_fail = readenv(CONFIG_ENV_OFFSET_REDUND, (u_char *) tmp_env2);

	ret = env_import_redund((char *)tmp_env1, read1_fail, (char *)tmp_env2,
				read2_fail, H_EXTERNAL);

done:
	free(tmp_env1);
	free(tmp_env2);

	return ret;
#endif /* ! ENV_IS_EMBEDDED */
}
#else /* ! CONFIG_ENV_OFFSET_REDUND */
/*
 * The legacy NAND code saved the environment in the first NAND
 * device i.e., nand_dev_desc + 0. This is also the behaviour using
 * the new NAND code.
 */
static int env_nand_load(void)
{
#if !defined(ENV_IS_EMBEDDED)
	int ret;
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);

#if defined(CONFIG_ENV_OFFSET_OOB)
	struct mtd_info *mtd  = get_nand_dev_by_index(0);
	/*
	 * If unable to read environment offset from NAND OOB then fall through
	 * to the normal environment reading code below
	 */
	if (mtd && !get_nand_env_oob(mtd, &nand_env_oob_offset)) {
		printf("Found Environment offset in OOB..\n");
	} else {
		env_set_default("no env offset in OOB", 0);
		return;
	}
#endif

	ret = readenv(CONFIG_ENV_OFFSET, (u_char *)buf);
	if (ret) {
		env_set_default("readenv() failed", 0);
		return -EIO;
	}

	return env_import(buf, 1, H_EXTERNAL);
#endif /* ! ENV_IS_EMBEDDED */

	return 0;
}
#endif /* CONFIG_ENV_OFFSET_REDUND */

U_BOOT_ENV_LOCATION(nand) = {
	.location	= ENVL_NAND,
	ENV_NAME("NAND")
	.load		= env_nand_load,
#if defined(CMD_SAVEENV)
	.save		= env_save_ptr(env_nand_save),
#endif
	.init		= env_nand_init,
};
