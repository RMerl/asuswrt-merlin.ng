// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008-2011 Freescale Semiconductor, Inc.
 */

/* #define DEBUG */

#include <common.h>
#include <asm/global_data.h>

#include <command.h>
#include <env.h>
#include <env_internal.h>
#include <fdtdec.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <memalign.h>
#include <mmc.h>
#include <part.h>
#include <search.h>
#include <errno.h>
#include <dm/ofnode.h>

#define ENV_MMC_INVALID_OFFSET ((s64)-1)

#if defined(CONFIG_ENV_MMC_USE_DT)
/* ENV offset is invalid when not defined in Device Tree */
#define ENV_MMC_OFFSET		ENV_MMC_INVALID_OFFSET
#define ENV_MMC_OFFSET_REDUND	ENV_MMC_INVALID_OFFSET

#else
/* Default ENV offset when not defined in Device Tree */
#define ENV_MMC_OFFSET		CONFIG_ENV_OFFSET

#if defined(CONFIG_ENV_OFFSET_REDUND)
#define ENV_MMC_OFFSET_REDUND	CONFIG_ENV_OFFSET_REDUND
#else
#define ENV_MMC_OFFSET_REDUND	ENV_MMC_INVALID_OFFSET
#endif
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 * In case the environment is redundant, stored in eMMC hardware boot
 * partition and the environment and redundant environment offsets are
 * identical, store the environment and redundant environment in both
 * eMMC boot partitions, one copy in each.
 * */
#if (defined(CONFIG_SYS_REDUNDAND_ENVIRONMENT) && \
     (CONFIG_SYS_MMC_ENV_PART == 1) && \
     (CONFIG_ENV_OFFSET == CONFIG_ENV_OFFSET_REDUND))
#define ENV_MMC_HWPART_REDUND	1
#endif

#if CONFIG_IS_ENABLED(OF_CONTROL)
static inline int mmc_offset_try_partition(const char *str, int copy, s64 *val)
{
	struct disk_partition info;
	struct blk_desc *desc;
	int len, i, ret;
	char dev_str[4];

	snprintf(dev_str, sizeof(dev_str), "%d", mmc_get_env_dev());
	ret = blk_get_device_by_str("mmc", dev_str, &desc);
	if (ret < 0)
		return (ret);

	for (i = 1;;i++) {
		ret = part_get_info(desc, i, &info);
		if (ret < 0)
			return ret;

		if (str && !strncmp((const char *)info.name, str, sizeof(info.name)))
			break;
#ifdef CONFIG_PARTITION_TYPE_GUID
		if (!str) {
			const efi_guid_t env_guid = PARTITION_U_BOOT_ENVIRONMENT;
			efi_guid_t type_guid;

			uuid_str_to_bin(info.type_guid, type_guid.b, UUID_STR_FORMAT_GUID);
			if (!memcmp(&env_guid, &type_guid, sizeof(efi_guid_t)))
				break;
		}
#endif
	}

	/* round up to info.blksz */
	len = DIV_ROUND_UP(CONFIG_ENV_SIZE, info.blksz);

	/* use the top of the partion for the environment */
	*val = (info.start + info.size - (1 + copy) * len) * info.blksz;

	return 0;
}

static inline s64 mmc_offset(struct mmc *mmc, int copy)
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
	int hwpart = 0;
	int err;

	if (IS_ENABLED(CONFIG_SYS_MMC_ENV_PART))
		hwpart = mmc_get_env_part(mmc);

#if defined(CONFIG_ENV_MMC_PARTITION)
	str = CONFIG_ENV_MMC_PARTITION;
#else
	/* look for the partition in mmc CONFIG_SYS_MMC_ENV_DEV */
	str = ofnode_conf_read_str(dt_prop.partition);
#endif

	if (str) {
		/* try to place the environment at end of the partition */
		err = mmc_offset_try_partition(str, copy, &val);
		if (!err)
			return val;
		debug("env partition '%s' not found (%d)", str, err);
	}

	/* try the GPT partition with "U-Boot ENV" TYPE GUID */
	if (IS_ENABLED(CONFIG_PARTITION_TYPE_GUID) && hwpart == 0) {
		err = mmc_offset_try_partition(NULL, copy, &val);
		if (!err)
			return val;
	}

	defvalue = ENV_MMC_OFFSET;
	propname = dt_prop.offset;

	if (IS_ENABLED(CONFIG_SYS_REDUNDAND_ENVIRONMENT) && copy) {
		defvalue = ENV_MMC_OFFSET_REDUND;
		propname = dt_prop.offset_redund;
	}

	return ofnode_conf_read_int(propname, defvalue);
}
#else
static inline s64 mmc_offset(struct mmc *mmc, int copy)
{
	s64 offset = ENV_MMC_OFFSET;

	if (IS_ENABLED(CONFIG_SYS_REDUNDAND_ENVIRONMENT) && copy)
		offset = ENV_MMC_OFFSET_REDUND;

	return offset;
}
#endif

__weak int mmc_get_env_addr(struct mmc *mmc, int copy, u32 *env_addr)
{
	s64 offset = mmc_offset(mmc, copy);

	if (offset == ENV_MMC_INVALID_OFFSET) {
		printf("Invalid ENV offset in MMC, copy=%d\n", copy);
		return -ENOENT;
	}

	if (offset < 0)
		offset += mmc->capacity;

	*env_addr = offset;

	return 0;
}

#ifdef CONFIG_SYS_MMC_ENV_PART
__weak uint mmc_get_env_part(struct mmc *mmc)
{
	return CONFIG_SYS_MMC_ENV_PART;
}

static unsigned char env_mmc_orig_hwpart;

static int mmc_set_env_part(struct mmc *mmc, uint part)
{
	int dev = mmc_get_env_dev();
	int ret = 0;

	ret = blk_select_hwpart_devnum(UCLASS_MMC, dev, part);
	if (ret)
		puts("MMC partition switch failed\n");

	return ret;
}

static bool mmc_set_env_part_init(struct mmc *mmc)
{
	env_mmc_orig_hwpart = mmc_get_blk_desc(mmc)->hwpart;
	if (mmc_set_env_part(mmc, mmc_get_env_part(mmc)))
		return false;

	return true;
}

static int mmc_set_env_part_restore(struct mmc *mmc)
{
	return mmc_set_env_part(mmc, env_mmc_orig_hwpart);
}
#else
static inline int mmc_set_env_part(struct mmc *mmc, uint part) {return 0; };
static bool mmc_set_env_part_init(struct mmc *mmc) {return true; }
static inline int mmc_set_env_part_restore(struct mmc *mmc) {return 0; };
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
	if (!mmc_set_env_part_init(mmc))
		return "MMC partition switch failed";

	return NULL;
}

static void fini_mmc_for_env(struct mmc *mmc)
{
	mmc_set_env_part_restore(mmc);
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

	if (IS_ENABLED(CONFIG_SYS_REDUNDAND_ENVIRONMENT)) {
		if (gd->env_valid == ENV_VALID)
			copy = 1;

		if (IS_ENABLED(ENV_MMC_HWPART_REDUND)) {
			ret = mmc_set_env_part(mmc, copy + 1);
			if (ret)
				goto fini;
		}
	}

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

	if (IS_ENABLED(CONFIG_SYS_REDUNDAND_ENVIRONMENT))
		gd->env_valid = gd->env_valid == ENV_REDUND ? ENV_VALID : ENV_REDUND;

fini:
	fini_mmc_for_env(mmc);

	return ret;
}

static inline int erase_env(struct mmc *mmc, unsigned long size,
			    unsigned long offset)
{
	uint blk_start, blk_cnt, n;
	struct blk_desc *desc = mmc_get_blk_desc(mmc);
	u32 erase_size;

	erase_size = mmc->erase_grp_size * desc->blksz;
	blk_start = ALIGN_DOWN(offset, erase_size) / desc->blksz;
	blk_cnt = ALIGN(size, erase_size) / desc->blksz;

	n = blk_derase(desc, blk_start, blk_cnt);
	printf("%d blocks erased at 0x%x: %s\n", n, blk_start,
	       (n == blk_cnt) ? "OK" : "ERROR");

	return (n == blk_cnt) ? 0 : 1;
}

static int env_mmc_erase(void)
{
	int dev = mmc_get_env_dev();
	struct mmc *mmc = find_mmc_device(dev);
	int	ret, copy = 0;
	u32	offset;
	const char *errmsg;

	errmsg = init_mmc_for_env(mmc);
	if (errmsg) {
		printf("%s\n", errmsg);
		return 1;
	}

	if (mmc_get_env_addr(mmc, copy, &offset)) {
		ret = CMD_RET_FAILURE;
		goto fini;
	}

	printf("\n");
	ret = erase_env(mmc, CONFIG_ENV_SIZE, offset);

	if (IS_ENABLED(CONFIG_SYS_REDUNDAND_ENVIRONMENT)) {
		copy = 1;

		if (IS_ENABLED(ENV_MMC_HWPART_REDUND)) {
			ret = mmc_set_env_part(mmc, copy + 1);
			if (ret)
				goto fini;
		}

		if (mmc_get_env_addr(mmc, copy, &offset)) {
			ret = CMD_RET_FAILURE;
			goto fini;
		}

		ret |= erase_env(mmc, CONFIG_ENV_SIZE, offset);
	}

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

#if defined(ENV_IS_EMBEDDED)
static int env_mmc_load(void)
{
	return 0;
}
#elif defined(CONFIG_SYS_REDUNDAND_ENVIRONMENT)
static int env_mmc_load(void)
{
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

	if (IS_ENABLED(ENV_MMC_HWPART_REDUND)) {
		ret = mmc_set_env_part(mmc, 1);
		if (ret)
			goto fini;
	}

	read1_fail = read_env(mmc, CONFIG_ENV_SIZE, offset1, tmp_env1);

	if (IS_ENABLED(ENV_MMC_HWPART_REDUND)) {
		ret = mmc_set_env_part(mmc, 2);
		if (ret)
			goto fini;
	}

	read2_fail = read_env(mmc, CONFIG_ENV_SIZE, offset2, tmp_env2);

	ret = env_import_redund((char *)tmp_env1, read1_fail, (char *)tmp_env2,
				read2_fail, H_EXTERNAL);

fini:
	fini_mmc_for_env(mmc);
err:
	if (ret)
		env_set_default(errmsg, 0);

	return ret;
}
#else /* ! CONFIG_SYS_REDUNDAND_ENVIRONMENT */
static int env_mmc_load(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	struct mmc *mmc;
	u32 offset;
	int ret;
	int dev = mmc_get_env_dev();
	const char *errmsg;
	env_t *ep = NULL;

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

	ret = env_import(buf, 1, H_EXTERNAL);
	if (!ret) {
		ep = (env_t *)buf;
		gd->env_addr = (ulong)&ep->data;
	}

fini:
	fini_mmc_for_env(mmc);
err:
	if (ret)
		env_set_default(errmsg, 0);

	return ret;
}
#endif /* CONFIG_SYS_REDUNDAND_ENVIRONMENT */

U_BOOT_ENV_LOCATION(mmc) = {
	.location	= ENVL_MMC,
	ENV_NAME("MMC")
	.load		= env_mmc_load,
#if defined(CONFIG_CMD_SAVEENV) && !defined(CONFIG_SPL_BUILD)
	.save		= env_save_ptr(env_mmc_save),
	.erase		= ENV_ERASE_PTR(env_mmc_erase)
#endif
};
