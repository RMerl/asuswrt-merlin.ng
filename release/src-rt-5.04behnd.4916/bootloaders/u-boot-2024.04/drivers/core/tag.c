// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Linaro Limited
 *			Author: AKASHI Takahiro
 */

#include <malloc.h>
#include <asm/global_data.h>
#include <dm/root.h>
#include <dm/tag.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/types.h>

struct udevice;

DECLARE_GLOBAL_DATA_PTR;

static const char *const tag_name[] = {
	[DM_TAG_PLAT]		= "plat",
	[DM_TAG_PARENT_PLAT]	= "parent_plat",
	[DM_TAG_UC_PLAT]	= "uclass_plat",

	[DM_TAG_PRIV]		= "priv",
	[DM_TAG_PARENT_PRIV]	= "parent_priv",
	[DM_TAG_UC_PRIV]	= "uclass_priv",
	[DM_TAG_DRIVER_DATA]	= "driver_data",

	[DM_TAG_EFI]		= "efi",
};

const char *tag_get_name(enum dm_tag_t tag)
{
	return tag_name[tag];
}

int dev_tag_set_ptr(struct udevice *dev, enum dm_tag_t tag, void *ptr)
{
	struct dmtag_node *node;

	if (!dev || tag >= DM_TAG_COUNT)
		return -EINVAL;

	list_for_each_entry(node, &gd->dmtag_list, sibling) {
		if (node->dev == dev && node->tag == tag)
			return -EEXIST;
	}

	node = calloc(sizeof(*node), 1);
	if (!node)
		return -ENOMEM;

	node->dev = dev;
	node->tag = tag;
	node->ptr = ptr;
	list_add_tail(&node->sibling, (struct list_head *)&gd->dmtag_list);

	return 0;
}

int dev_tag_set_val(struct udevice *dev, enum dm_tag_t tag, ulong val)
{
	struct dmtag_node *node;

	if (!dev || tag >= DM_TAG_COUNT)
		return -EINVAL;

	list_for_each_entry(node, &gd->dmtag_list, sibling) {
		if (node->dev == dev && node->tag == tag)
			return -EEXIST;
	}

	node = calloc(sizeof(*node), 1);
	if (!node)
		return -ENOMEM;

	node->dev = dev;
	node->tag = tag;
	node->val = val;
	list_add_tail(&node->sibling, (struct list_head *)&gd->dmtag_list);

	return 0;
}

int dev_tag_get_ptr(struct udevice *dev, enum dm_tag_t tag, void **ptrp)
{
	struct dmtag_node *node;

	if (!dev || tag >= DM_TAG_COUNT)
		return -EINVAL;

	list_for_each_entry(node, &gd->dmtag_list, sibling) {
		if (node->dev == dev && node->tag == tag) {
			*ptrp = node->ptr;
			return 0;
		}
	}

	return -ENOENT;
}

int dev_tag_get_val(struct udevice *dev, enum dm_tag_t tag, ulong *valp)
{
	struct dmtag_node *node;

	if (!dev || tag >= DM_TAG_COUNT)
		return -EINVAL;

	list_for_each_entry(node, &gd->dmtag_list, sibling) {
		if (node->dev == dev && node->tag == tag) {
			*valp = node->val;
			return 0;
		}
	}

	return -ENOENT;
}

int dev_tag_del(struct udevice *dev, enum dm_tag_t tag)
{
	struct dmtag_node *node, *tmp;

	if (!dev || tag >= DM_TAG_COUNT)
		return -EINVAL;

	list_for_each_entry_safe(node, tmp, &gd->dmtag_list, sibling) {
		if (node->dev == dev && node->tag == tag) {
			list_del(&node->sibling);
			free(node);

			return 0;
		}
	}

	return -ENOENT;
}

int dev_tag_del_all(struct udevice *dev)
{
	struct dmtag_node *node, *tmp;
	bool found = false;

	if (!dev)
		return -EINVAL;

	list_for_each_entry_safe(node, tmp, &gd->dmtag_list, sibling) {
		if (node->dev == dev) {
			list_del(&node->sibling);
			free(node);
			found = true;
		}
	}

	if (found)
		return 0;

	return -ENOENT;
}

void dev_tag_collect_stats(struct dm_stats *stats)
{
	struct dmtag_node *node;

	list_for_each_entry(node, &gd->dmtag_list, sibling) {
		stats->tag_count++;
		stats->tag_size += sizeof(struct dmtag_node);
	}
}
