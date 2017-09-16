/*
 * dm-device.c: create or remove device via device mapper API.
 *
 * Copyright (c) 2010 EMC Corporation, Haiying Tang <Tang_Haiying@emc.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <linux/kdev_t.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>
#include <errno.h>
#include <libdevmapper.h>

#include "device-discovery.h"

#define DM_DEV_NAME_LEN		256

#ifndef DM_MAX_TYPE_NAME
#define DM_MAX_TYPE_NAME	16
#endif

#define DM_PARAMS_LEN		512	/* XXX: is this enough for target? */
#define TYPE_HAS_DEV(type)	((type == BLOCK_VOLUME_SIMPLE) || \
			 (type == BLOCK_VOLUME_PSEUDO))

struct bl_dm_table {
	uint64_t offset;
	uint64_t size;
	char target_type[DM_MAX_TYPE_NAME];
	char params[DM_PARAMS_LEN];
	struct bl_dm_table *next;
};

struct bl_dm_tree {
	uint64_t dev;
	struct dm_tree *tree;
	struct bl_dm_tree *next;
};

static const char dm_name[] = "pnfs_vol_%u";

static unsigned int dev_count;

static inline struct bl_dm_table *bl_dm_table_alloc(void)
{
	return (struct bl_dm_table *)calloc(1, sizeof(struct bl_dm_table));
}

static void bl_dm_table_free(struct bl_dm_table *bl_table_head)
{
	struct bl_dm_table *p;

	while (bl_table_head) {
		p = bl_table_head->next;
		free(bl_table_head);
		bl_table_head = p;
	}
}

static void add_to_bl_dm_table(struct bl_dm_table **bl_table_head,
			struct bl_dm_table *table)
{
	struct bl_dm_table *p;

	if (!*bl_table_head) {
		*bl_table_head = table;
		return;
	}
	p = *bl_table_head;
	while (p->next)
		p = p->next;
	p->next = table;
}

struct bl_dm_tree *bl_tree_head;

static struct bl_dm_tree *find_bl_dm_tree(uint64_t dev)
{
	struct bl_dm_tree *p;

	for (p = bl_tree_head; p; p = p->next) {
		if (p->dev == dev)
			break;
	}
	return p;
}

static void del_from_bl_dm_tree(uint64_t dev)
{
	struct bl_dm_tree *p, *pre = bl_tree_head;

	for (p = pre; p; p = p->next) {
		if (p->dev == dev) {
			pre->next = p->next;
			if (p == bl_tree_head)
				bl_tree_head = bl_tree_head->next;
			free(p);
			break;
		}
		pre = p;
	}
}

static void add_to_bl_dm_tree(struct bl_dm_tree *tree)
{
	struct bl_dm_tree *p;

	if (!bl_tree_head) {
		bl_tree_head = tree;
		return;
	}
	p = bl_tree_head;
	while (p->next)
		p = p->next;
	p->next = tree;
	return;
}

/*
 * Create device via device mapper
 * return 0 when creation failed
 * return dev no for created device
 */
static uint64_t
dm_device_create_mapped(const char *dev_name, struct bl_dm_table *p)
{
	struct dm_task *dmt;
	struct dm_info dminfo;
	int ret = 0;

	dmt = dm_task_create(DM_DEVICE_CREATE);
	if (!dmt) {
		BL_LOG_ERR("Create dm_task for %s failed\n", dev_name);
		return 0;
	}
	ret = dm_task_set_name(dmt, dev_name);
	if (!ret)
		goto err_out;

	while (p) {
		ret =
		    dm_task_add_target(dmt, p->offset, p->size, p->target_type,
				       p->params);
		if (!ret)
			goto err_out;
		p = p->next;
	}

	ret = dm_task_run(dmt) && dm_task_get_info(dmt, &dminfo)
	    && dminfo.exists;

	if (!ret)
		goto err_out;

	dm_task_update_nodes();

 err_out:
	dm_task_destroy(dmt);

	if (!ret) {
		BL_LOG_ERR("Create device %s failed\n", dev_name);
		return 0;
	}
	return MKDEV(dminfo.major, dminfo.minor);
}

static int dm_device_remove_byname(const char *dev_name)
{
	struct dm_task *dmt;
	int ret = 0;

	BL_LOG_INFO("%s: %s\n", __func__, dev_name);

	dmt = dm_task_create(DM_DEVICE_REMOVE);
	if (!dmt)
		return 0;

	ret = dm_task_set_name(dmt, dev_name) && dm_task_run(dmt);

	dm_task_update_nodes();
	dm_task_destroy(dmt);

	return ret;
}

int dm_device_remove(uint64_t dev)
{
	struct dm_task *dmt;
	struct dm_names *dmnames;
	char *name = NULL;
	int ret = 0;

	/* Look for dev_name via dev, if dev_name could be transferred here,
	   we could jump to DM_DEVICE_REMOVE directly */

	dmt = dm_task_create(DM_DEVICE_LIST);
	if (!dmt) {
		BL_LOG_ERR("dm_task creation failed\n");
		goto out;
	}

	ret = dm_task_run(dmt);
	if (!ret) {
		BL_LOG_ERR("dm_task_run failed\n");
		goto out;
	}

	dmnames = dm_task_get_names(dmt);
	if (!dmnames || !dmnames->dev) {
		BL_LOG_ERR("dm_task_get_names failed\n");
		goto out;
	}

	while (dmnames) {
		if (dmnames->dev == dev) {
			name = strdup(dmnames->name);
			break;
		}
		dmnames = (void *)dmnames + dmnames->next;
	}

	if (!name) {
		BL_LOG_ERR("Could not find device\n");
		goto out;
	}

	dm_task_update_nodes();

 out:
	if (dmt)
		dm_task_destroy(dmt);

	/* Start to remove device */
	if (name) {
		ret = dm_device_remove_byname(name);
		free(name);
	}

	return ret;
}

static void dm_devicelist_remove(unsigned int start, unsigned int end)
{
	char dev_name[DM_DEV_NAME_LEN];
	unsigned int count;

	if (start >= dev_count || end <= 1 || start >= end - 1)
		return;

	for (count = end - 1; count > start; count--) {
		snprintf(dev_name, sizeof dev_name, dm_name, count - 1);
		dm_device_remove_byname(dev_name);
	}

	return;
}

static void bl_dm_remove_tree(uint64_t dev)
{
	struct bl_dm_tree *p;

	p = find_bl_dm_tree(dev);
	if (!p)
		return;

	dm_tree_free(p->tree);
	del_from_bl_dm_tree(dev);
}

static int bl_dm_create_tree(uint64_t dev)
{
	struct dm_tree *tree;
	struct bl_dm_tree *bl_tree;

	bl_tree = find_bl_dm_tree(dev);
	if (bl_tree)
		return 1;

	tree = dm_tree_create();
	if (!tree)
		return 0;

	if (!dm_tree_add_dev(tree, MAJOR(dev), MINOR(dev))) {
		dm_tree_free(tree);
		return 0;
	}

	bl_tree = malloc(sizeof(struct bl_dm_tree));
	if (!bl_tree) {
		dm_tree_free(tree);
		return 0;
	}

	bl_tree->dev = dev;
	bl_tree->tree = tree;
	bl_tree->next = NULL;
	add_to_bl_dm_tree(bl_tree);

	return 1;
}

int dm_device_remove_all(uint64_t *dev)
{
	struct bl_dm_tree *p;
	struct dm_tree_node *node;
	const char *uuid;
	int ret = 0;
	uint32_t major, minor;
	uint64_t bl_dev;

	memcpy(&major, dev, sizeof(uint32_t));
	memcpy(&minor, (void *)dev + sizeof(uint32_t), sizeof(uint32_t));
	bl_dev = MKDEV(major, minor);
	p = find_bl_dm_tree(bl_dev);
	if (!p)
		return ret;

	node = dm_tree_find_node(p->tree, MAJOR(bl_dev), MINOR(bl_dev));
	if (!node)
		return ret;

	uuid = dm_tree_node_get_uuid(node);
	if (!uuid)
		return ret;

	dm_device_remove(bl_dev);
	ret = dm_tree_deactivate_children(node, uuid, strlen(uuid));
	dm_task_update_nodes();
	bl_dm_remove_tree(bl_dev);

	return ret;
}

static int dm_device_exists(char *dev_name)
{
	char fullname[DM_DEV_NAME_LEN];

	snprintf(fullname, sizeof fullname, "/dev/mapper/%s", dev_name);
	return (access(fullname, F_OK) >= 0);
}

/* TODO: check the value for DM_DEV_NAME_LEN, DM_TYPE_LEN, DM_PARAMS_LEN */
uint64_t dm_device_create(struct bl_volume *vols, int num_vols)
{
	uint64_t size, stripe_unit, dev = 0;
	unsigned int count = dev_count;
	int volnum, i, pos;
	struct bl_volume *node;
	char *tmp;
	struct bl_dm_table *table = NULL;
	struct bl_dm_table *bl_table_head = NULL;
	unsigned int len;
	char *dev_name = NULL;

	/* Create pseudo device here */
	for (volnum = 0; volnum < num_vols; volnum++) {
		node = &vols[volnum];
		switch (node->bv_type) {
		case BLOCK_VOLUME_SIMPLE:
			/* Do not need to create device here */
			dev = node->param.bv_dev;
			goto continued;
		case BLOCK_VOLUME_SLICE:
			table = bl_dm_table_alloc();
			if (!table)
				goto out;
			table->offset = 0;
			table->size = node->bv_size;
			strcpy(table->target_type, "linear");
			if (!TYPE_HAS_DEV(node->bv_vols[0]->bv_type)) {
				free(table);
				goto out;
			}
			dev = node->bv_vols[0]->param.bv_dev;
			tmp = table->params;
			BL_LOG_INFO("%s: major %lu minor %lu", __func__,
					MAJOR(dev), MINOR(dev));
			if (!dm_format_dev(tmp, DM_PARAMS_LEN,
					   MAJOR(dev), MINOR(dev))) {
				free(table);
				goto out;
			}
			tmp += strlen(tmp);
			sprintf(tmp, " %lu", node->param.bv_offset);
			add_to_bl_dm_table(&bl_table_head, table);
			break;
		case BLOCK_VOLUME_STRIPE:
			table = bl_dm_table_alloc();
			if (!table)
				goto out;
			table->offset = 0;
			/* Truncate size to a stripe unit boundary */
			stripe_unit = node->param.bv_stripe_unit;
			table->size =
			    node->bv_size - (node->bv_size % stripe_unit);
			strcpy(table->target_type, "striped");
			sprintf(table->params, "%d %llu %n", node->bv_vol_n,
				(long long unsigned) stripe_unit, &pos);
			/* Copy subdev major:minor to params */
			tmp = table->params + pos;
			len = DM_PARAMS_LEN - pos;
			for (i = 0; i < node->bv_vol_n; i++) {
				if (!TYPE_HAS_DEV(node->bv_vols[i]->bv_type)) {
					free(table);
					goto out;
				}
				dev = node->bv_vols[i]->param.bv_dev;
				if (!dm_format_dev(tmp, len, MAJOR(dev),
						   MINOR(dev))) {
					free(table);
					goto out;
				}
				pos = strlen(tmp);
				tmp += pos;
				len -= pos;
				sprintf(tmp, " %d ", 0);
				tmp += 3;
				len -= 3;
			}
			add_to_bl_dm_table(&bl_table_head, table);
			break;
		case BLOCK_VOLUME_CONCAT:
			size = 0;
			for (i = 0; i < node->bv_vol_n; i++) {
				table = bl_dm_table_alloc();
				if (!table)
					goto out;
				table->offset = size;
				table->size = node->bv_vols[i]->bv_size;
				if (!TYPE_HAS_DEV(node->bv_vols[i]->bv_type)) {
					free(table);
					goto out;
				}
				strcpy(table->target_type, "linear");
				tmp = table->params;
				dev = node->bv_vols[i]->param.bv_dev;
				BL_LOG_INFO("%s: major %lu minor %lu", __func__,
					MAJOR(dev), MINOR(dev));
				if (!dm_format_dev(tmp, DM_PARAMS_LEN,
						   MAJOR(dev), MINOR(dev))) {
					free(table);
					goto out;
				}
				tmp += strlen(tmp);
				sprintf(tmp, " %d", 0);
				size += table->size;
				add_to_bl_dm_table(&bl_table_head, table);
			}
			break;
		default:
			/* Delete previous temporary devices */
			dm_devicelist_remove(count, dev_count);
			goto out;
		}		/* end of swtich */
		/* Create dev_name here. Name of device is pnfs_vol_XXX */
		if (dev_name)
			free(dev_name);
		dev_name = (char *)calloc(DM_DEV_NAME_LEN, sizeof(char));
		if (!dev_name) {
			BL_LOG_ERR("%s: Out of memory\n", __func__);
			goto out;
		}
		do {
			snprintf(dev_name, DM_DEV_NAME_LEN, dm_name,
				 dev_count++);
		} while (dm_device_exists(dev_name));

		dev = dm_device_create_mapped(dev_name, bl_table_head);
		BL_LOG_INFO("%s: %d %s %d:%d\n", __func__, volnum, dev_name,
			    (int) MAJOR(dev), (int) MINOR(dev));
		if (!dev) {
			/* Delete previous temporary devices */
			dm_devicelist_remove(count, dev_count);
			goto out;
		}
		node->param.bv_dev = dev;
		/* TODO: extend use with PSEUDO later */
		node->bv_type = BLOCK_VOLUME_PSEUDO;

 continued:
		if (bl_table_head)
			bl_dm_table_free(bl_table_head);
		bl_table_head = NULL;
	}
 out:
	if (bl_table_head) {
		bl_dm_table_free(bl_table_head);
		bl_table_head = NULL;
	}
	if (dev)
		bl_dm_create_tree(dev);
	if (dev_name)
		free(dev_name);
	return dev;
}
