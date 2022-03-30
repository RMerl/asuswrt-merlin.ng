// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include "mmc_private.h"

static struct list_head mmc_devices;
static int cur_dev_num = -1;

#if CONFIG_IS_ENABLED(MMC_TINY)
static struct mmc mmc_static;
struct mmc *find_mmc_device(int dev_num)
{
	return &mmc_static;
}

void mmc_do_preinit(void)
{
	struct mmc *m = &mmc_static;
#ifdef CONFIG_FSL_ESDHC_ADAPTER_IDENT
	mmc_set_preinit(m, 1);
#endif
	if (m->preinit)
		mmc_start_init(m);
}

struct blk_desc *mmc_get_blk_desc(struct mmc *mmc)
{
	return &mmc->block_dev;
}
#else
struct mmc *find_mmc_device(int dev_num)
{
	struct mmc *m;
	struct list_head *entry;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

		if (m->block_dev.devnum == dev_num)
			return m;
	}

#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
	printf("MMC Device %d not found\n", dev_num);
#endif

	return NULL;
}

int mmc_get_next_devnum(void)
{
	return cur_dev_num++;
}

struct blk_desc *mmc_get_blk_desc(struct mmc *mmc)
{
	return &mmc->block_dev;
}

int get_mmc_num(void)
{
	return cur_dev_num;
}

void mmc_do_preinit(void)
{
	struct mmc *m;
	struct list_head *entry;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

#ifdef CONFIG_FSL_ESDHC_ADAPTER_IDENT
		mmc_set_preinit(m, 1);
#endif
		if (m->preinit)
			mmc_start_init(m);
	}
}
#endif

void mmc_list_init(void)
{
	INIT_LIST_HEAD(&mmc_devices);
	cur_dev_num = 0;
}

void mmc_list_add(struct mmc *mmc)
{
	INIT_LIST_HEAD(&mmc->link);

	list_add_tail(&mmc->link, &mmc_devices);
}

#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
void print_mmc_devices(char separator)
{
	struct mmc *m;
	struct list_head *entry;
	char *mmc_type;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

		if (m->has_init)
			mmc_type = IS_SD(m) ? "SD" : "eMMC";
		else
			mmc_type = NULL;

		printf("%s: %d", m->cfg->name, m->block_dev.devnum);
		if (mmc_type)
			printf(" (%s)", mmc_type);

		if (entry->next != &mmc_devices) {
			printf("%c", separator);
			if (separator != '\n')
				puts(" ");
		}
	}

	printf("\n");
}

#else
void print_mmc_devices(char separator) { }
#endif

#if CONFIG_IS_ENABLED(MMC_TINY)
static struct mmc mmc_static = {
	.dsr_imp		= 0,
	.dsr			= 0xffffffff,
	.block_dev = {
		.if_type	= IF_TYPE_MMC,
		.removable	= 1,
		.devnum		= 0,
		.block_read	= mmc_bread,
		.block_write	= mmc_bwrite,
		.block_erase	= mmc_berase,
		.part_type	= 0,
	},
};

struct mmc *mmc_create(const struct mmc_config *cfg, void *priv)
{
	struct mmc *mmc = &mmc_static;

	mmc->cfg = cfg;
	mmc->priv = priv;

	return mmc;
}

void mmc_destroy(struct mmc *mmc)
{
}
#else
struct mmc *mmc_create(const struct mmc_config *cfg, void *priv)
{
	struct blk_desc *bdesc;
	struct mmc *mmc;

	/* quick validation */
	if (cfg == NULL || cfg->f_min == 0 ||
	    cfg->f_max == 0 || cfg->b_max == 0)
		return NULL;

#if !CONFIG_IS_ENABLED(DM_MMC)
	if (cfg->ops == NULL || cfg->ops->send_cmd == NULL)
		return NULL;
#endif

	mmc = calloc(1, sizeof(*mmc));
	if (mmc == NULL)
		return NULL;

	mmc->cfg = cfg;
	mmc->priv = priv;

	/* the following chunk was mmc_register() */

	/* Setup dsr related values */
	mmc->dsr_imp = 0;
	mmc->dsr = 0xffffffff;
	/* Setup the universal parts of the block interface just once */
	bdesc = mmc_get_blk_desc(mmc);
	bdesc->if_type = IF_TYPE_MMC;
	bdesc->removable = 1;
	bdesc->devnum = mmc_get_next_devnum();
	bdesc->block_read = mmc_bread;
	bdesc->block_write = mmc_bwrite;
	bdesc->block_erase = mmc_berase;

	/* setup initial part type */
	bdesc->part_type = mmc->cfg->part_type;
	mmc_list_add(mmc);

	return mmc;
}

void mmc_destroy(struct mmc *mmc)
{
	/* only freeing memory for now */
	free(mmc);
}
#endif

static int mmc_select_hwpartp(struct blk_desc *desc, int hwpart)
{
	struct mmc *mmc = find_mmc_device(desc->devnum);
	int ret;

	if (!mmc)
		return -ENODEV;

	if (mmc->block_dev.hwpart == hwpart)
		return 0;

	if (mmc->part_config == MMCPART_NOAVAILABLE)
		return -EMEDIUMTYPE;

	ret = mmc_switch_part(mmc, hwpart);
	if (ret)
		return ret;

	return 0;
}

static int mmc_get_dev(int dev, struct blk_desc **descp)
{
	struct mmc *mmc = find_mmc_device(dev);
	int ret;

	if (!mmc)
		return -ENODEV;
	ret = mmc_init(mmc);
	if (ret)
		return ret;

	*descp = &mmc->block_dev;

	return 0;
}

U_BOOT_LEGACY_BLK(mmc) = {
	.if_typename	= "mmc",
	.if_type	= IF_TYPE_MMC,
	.max_devs	= -1,
	.get_dev	= mmc_get_dev,
	.select_hwpart	= mmc_select_hwpartp,
};
