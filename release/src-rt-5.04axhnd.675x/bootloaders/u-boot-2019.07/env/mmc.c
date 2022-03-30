// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008-2011 Freescale Semiconductor, Inc.
 */

/* #define DEBUG */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <fdtdec.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <memalign.h>
#include <mmc.h>
#include <part.h>
#include <search.h>
#include <errno.h>

#define __STR(X) #X
#define STR(X) __STR(X)

#if defined(CONFIG_ENV_SIZE_REDUND) &&  \
	(CONFIG_ENV_SIZE_REDUND != CONFIG_ENV_SIZE)
#error CONFIG_ENV_SIZE_REDUND should be the same as CONFIG_ENV_SIZE
#endif

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_OFFSET 0
#endif

#if CONFIG_IS_ENABLED(OF_CONTROL)
static inline int mmc_offset_try_partition(const char *str, s64 *val)
{
	disk_partition_t info;
	struct blk_desc *desc;
	int len, i, ret;

	ret = blk_get_device_by_str("mmc", STR(CONFIG_SYS_MMC_ENV_DEV), &desc);
	if (ret < 0)
		return (ret);

	for (i = 1;;i++) {
		ret = part_get_info(desc, i, &info);
		if (ret < 0)
			return ret;

		if (!strncmp((const char *)info.name, str, sizeof(str)))
			break;
	}

	/* round up to info.blksz */
	len = (CONFIG_ENV_SIZE + info.blksz - 1) & ~(info.blksz - 1);

	/* use the top of the partion for the environment */
	*val = (info.start + info.size - 1) - len / info.blksz;

	return 0;
}

static inline s64 mmc_offset(int copy)
{
	const struct {
		const char *offset_redund;
		const char *partition;
		const char *offset;
	} dt_prop = {
		.offset_redund = "u-boot,mmc-env-offset-redundant",
		.partition = "u-boot,mmc-env-partition",
		.offset = "u-boot,mmc-env-offset",
	};
	s64 val = 0, defvalue;
	const char *propname;
	const char *str;
	int err;

	/* look for the partition in mmc CONFIG_SYS_MMC_ENV_DEV */
	str = fdtdec_get_config_string(gd->fdt_blob, dt_prop.partition);
	if (str) {
		/* try to place the environment at end of the partition */
		err = mmc_offset_try_partition(str, &val);
		if (!err)
			return val;
	}

	defvalue = CONFIG_ENV_OFFSET;
	propname = dt_prop.offset;

#if defined(CONFIG_ENV_OFFSET_REDUND)
	if (copy) {
		defvalue = CONFIG_ENV_OFFSET_REDUND;
		propname = dt_prop.offset_redund;
	}
#endif
	return fdtdec_get_config_int(gd->fdt_blob, propname, defvalue);
}
#else
static inline s64 mmc_offset(int copy)
{
	s64 offset = CONFIG_ENV_OFFSET;

#if defined(CONFIG_ENV_OFFSET_REDUND)
	if (copy)
		offset = CONFIG_ENV_OFFSET_REDUND;
#endif
	return offset;
}
#endif

__weak int mmc_get_env_addr(struct mmc *mmc, int copy, u32 *env_addr)
{
	s64 offset = mmc_offset(copy);

	if (offset < 0)
		offset += mmc->capacity;

	*env_addr = offset;

	return 0;
}

__weak int mmc_get_env_dev(void)
{
	return CONFIG_SYS_MMC_ENV_DEV;
}

#ifdef CONFIG_SYS_MMC_ENV_PART
__weak uint mmc_get_env_part(struct mmc *mmc)
{
	return CONFIG_SYS_MMC_ENV_PART;
}

static unsigned char env_mmc_orig_hwpart;

static int mmc_set_env_part(struct mmc *mmc)
{
	uint part = mmc_get_env_part(mmc);
	int dev = mmc_get_env_dev();
	int ret = 0;

	env_mmc_orig_hwpart = mmc_get_blk_desc(mmc)->hwpart;
	ret = blk_select_hwpart_devnum(IF_TYPE_MMC, dev, part);
	if (ret)
		puts("MMC partition switch failed\n");

	return ret;
}
#else
static inline int mmc_set_env_part(struct mmc *mmc) {return 0; };
#endif

static const char *init_mmc_for_env(struct mmc *mmc)
{
	if (!mmc)
		return "No MMC card found";

#if CONFIG_IS_ENABLED(BLK)
	struct udevice *dev;

	if (blk_get_from_parent(mmc->dev, &dev))
		return "No block device";
#else
	if (mmc_init(mmc))
		return "MMC init failed";
#endif
	if (mmc_set_env_part(mmc))
		return "MMC partition switch failed";

	return NULL;
}

static void fini_mmc_for_env(struct mmc *mmc)
{
#ifdef CONFIG_SYS_MMC_ENV_PART
	int dev = mmc_get_env_dev();

	blk_select_hwpart_devnum(IF_TYPE_MMC, dev, env_mmc_orig_hwpart);
#endif
}

#if defined(CONFIG_CMD_SAVEENV) && !defined(CONFIG_SPL_BUILD)
static inline int write_env(struct mmc *mmc, unsigned long size,
			    unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;
	struct blk_desc *desc = mmc_get_blk_desc(mmc);

	blk_start	= ALIGN(offset, mmc->write_bl_len) / mmc->write_bl_len;
	blk_cnt		= ALIGN(size, mmc->write_bl_len) / mmc->write_bl_len;

	n = blk_dwrite(desc, blk_start, blk_cnt, (u_char *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

static int env_mmc_save(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	int dev = mmc_get_env_dev();
	struct mmc *mmc = find_mmc_device(dev);
	u32	offset;
	int	ret, copy = 0;
	const char *errmsg;

	errmsg = init_mmc_for_env(mmc);
	if (errmsg) {
		printf("%s\n", errmsg);
		return 1;
	}

	ret = env_export(env_new);
	if (ret)
		goto fini;

#ifdef CONFIG_ENV_OFFSET_REDUND
	if (gd->env_valid == ENV_VALID)
		copy = 1;
#endif

	if (mmc_get_env_addr(mmc, copy, &offset)) {
		ret = 1;
		goto fini;
	}

	printf("Writing to %sMMC(%d)... ", copy ? "redundant " : "", dev);
	if (write_env(mmc, CONFIG_ENV_SIZE, offset, (u_char *)env_new)) {
		puts("failed\n");
		ret = 1;
		goto fini;
	}

	ret = 0;

#ifdef CONFIG_ENV_OFFSET_REDUND
	gd->env_valid = gd->env_valid == ENV_REDUND ? ENV_VALID : ENV_REDUND;
#endif

fini:
	fini_mmc_for_env(mmc);
	return ret;
}
#endif /* CONFIG_CMD_SAVEENV && !CONFIG_SPL_BUILD */

static inline int read_env(struct mmc *mmc, unsigned long size,
			   unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;
	struct blk_desc *desc = mmc_get_blk_desc(mmc);

	blk_start	= ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt		= ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;

	n = blk_dread(desc, blk_start, blk_cnt, (uchar *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

#ifdef CONFIG_ENV_OFFSET_REDUND
static int env_mmc_load(void)
{
#if !defined(ENV_IS_EMBEDDED)
	struct mmc *mmc;
	u32 offset1, offset2;
	int read1_fail = 0, read2_fail = 0;
	int ret;
	int dev = mmc_get_env_dev();
	const char *errmsg = NULL;

	ALLOC_CACHE_ALIGN_BUFFER(env_t, tmp_env1, 1);
	ALLOC_CACHE_ALIGN_BUFFER(env_t, tmp_env2, 1);

	mmc_initialize(NULL);

	mmc = find_mmc_device(dev);

	errmsg = init_mmc_for_env(mmc);
	if (errmsg) {
		ret = -EIO;
		goto err;
	}

	if (mmc_get_env_addr(mmc, 0, &offset1) ||
	    mmc_get_env_addr(mmc, 1, &offset2)) {
		ret = -EIO;
		goto fini;
	}

	read1_fail = read_env(mmc, CONFIG_ENV_SIZE, offset1, tmp_env1);
	read2_fail = read_env(mmc, CONFIG_ENV_SIZE, offset2, tmp_env2);

	ret = env_import_redund((char *)tmp_env1, read1_fail, (char *)tmp_env2,
				read2_fail);

fini:
	fini_mmc_for_env(mmc);
err:
	if (ret)
		set_default_env(errmsg, 0);

#endif
	return ret;
}
#else /* ! CONFIG_ENV_OFFSET_REDUND */
static int env_mmc_load(void)
{
#if !defined(ENV_IS_EMBEDDED)
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	struct mmc *mmc;
	u32 offset;
	int ret;
	int dev = mmc_get_env_dev();
	const char *errmsg;

	mmc = find_mmc_device(dev);

	errmsg = init_mmc_for_env(mmc);
	if (errmsg) {
		ret = -EIO;
		goto err;
	}

	if (mmc_get_env_addr(mmc, 0, &offset)) {
		ret = -EIO;
		goto fini;
	}

	if (read_env(mmc, CONFIG_ENV_SIZE, offset, buf)) {
		errmsg = "!read failed";
		ret = -EIO;
		goto fini;
	}

	ret = env_import(buf, 1);

fini:
	fini_mmc_for_env(mmc);
err:
	if (ret)
		set_default_env(errmsg, 0);
#endif
	return ret;
}
#endif /* CONFIG_ENV_OFFSET_REDUND */

U_BOOT_ENV_LOCATION(mmc) = {
	.location	= ENVL_MMC,
	ENV_NAME("MMC")
	.load		= env_mmc_load,
#ifndef CONFIG_SPL_BUILD
	.save		= env_save_ptr(env_mmc_save),
#endif
};
