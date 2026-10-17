// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#define LOG_CATEGORY UCLASS_ROOT

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <asm-generic/sections.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <dm/acpi.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/of.h>
#include <dm/of_access.h>
#include <dm/platdata.h>
#include <dm/read.h>
#include <dm/root.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>
#include <linux/list.h>
#include <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

static struct driver_info root_info = {
	.name		= "root_driver",
};

struct udevice *dm_root(void)
{
	if (!gd->dm_root) {
		dm_warn("Virtual root driver does not exist!\n");
		return NULL;
	}

	return gd->dm_root;
}

void dm_fixup_for_gd_move(struct global_data *new_gd)
{
	/* The sentinel node has moved, so update things that point to it */
	if (gd->dm_root) {
		new_gd->uclass_root->next->prev = new_gd->uclass_root;
		new_gd->uclass_root->prev->next = new_gd->uclass_root;
	}
}

static int dm_setup_inst(void)
{
	DM_ROOT_NON_CONST = DM_DEVICE_GET(root);

	if (CONFIG_IS_ENABLED(OF_PLATDATA_RT)) {
		struct udevice_rt *urt;
		void *start, *end;
		int each_size;
		void *base;
		int n_ents;
		uint size;

		/* Allocate the udevice_rt table */
		each_size = dm_udevice_size();
		start = ll_entry_start(struct udevice, udevice);
		end = ll_entry_end(struct udevice, udevice);
		size = end - start;
		n_ents = size / each_size;
		urt = calloc(n_ents, sizeof(struct udevice_rt));
		if (!urt)
			return log_msg_ret("urt", -ENOMEM);
		gd_set_dm_udevice_rt(urt);

		/* Now allocate space for the priv/plat data, and copy it in */
		size = __priv_data_end - __priv_data_start;

		base = calloc(1, size);
		if (!base)
			return log_msg_ret("priv", -ENOMEM);
		memcpy(base, __priv_data_start, size);
		gd_set_dm_priv_base(base);
	}

	return 0;
}

int dm_init(bool of_live)
{
	int ret;

	if (gd->dm_root) {
		dm_warn("Virtual root driver already exists!\n");
		return -EINVAL;
	}
	if (CONFIG_IS_ENABLED(OF_PLATDATA_INST)) {
		gd->uclass_root = &uclass_head;
	} else {
		gd->uclass_root = &DM_UCLASS_ROOT_S_NON_CONST;
		INIT_LIST_HEAD(DM_UCLASS_ROOT_NON_CONST);
	}

	if (CONFIG_IS_ENABLED(OF_PLATDATA_INST)) {
		ret = dm_setup_inst();
		if (ret) {
			log_debug("dm_setup_inst() failed: %d\n", ret);
			return ret;
		}
	} else {
		ret = device_bind_by_name(NULL, false, &root_info,
					  &DM_ROOT_NON_CONST);
		if (ret)
			return ret;
		if (CONFIG_IS_ENABLED(OF_CONTROL))
			dev_set_ofnode(DM_ROOT_NON_CONST, ofnode_root());
		ret = device_probe(DM_ROOT_NON_CONST);
		if (ret)
			return ret;
	}

	INIT_LIST_HEAD((struct list_head *)&gd->dmtag_list);

	return 0;
}

int dm_uninit(void)
{
	/* Remove non-vital devices first */
	device_remove(dm_root(), DM_REMOVE_NON_VITAL);
	device_remove(dm_root(), DM_REMOVE_NORMAL);
	device_unbind(dm_root());
	gd->dm_root = NULL;

	return 0;
}

#if CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)
int dm_remove_devices_flags(uint flags)
{
	device_remove(dm_root(), flags);

	return 0;
}
#endif

int dm_scan_plat(bool pre_reloc_only)
{
	int ret;

	if (CONFIG_IS_ENABLED(OF_PLATDATA_DRIVER_RT)) {
		struct driver_rt *dyn;
		int n_ents;

		n_ents = ll_entry_count(struct driver_info, driver_info);
		dyn = calloc(n_ents, sizeof(struct driver_rt));
		if (!dyn)
			return -ENOMEM;
		gd_set_dm_driver_rt(dyn);
	}

	ret = lists_bind_drivers(DM_ROOT_NON_CONST, pre_reloc_only);
	if (ret == -ENOENT) {
		dm_warn("Some drivers were not found\n");
		ret = 0;
	}

	return ret;
}

#if CONFIG_IS_ENABLED(OF_REAL)
/**
 * dm_scan_fdt_node() - Scan the device tree and bind drivers for a node
 *
 * This scans the subnodes of a device tree node and and creates a driver
 * for each one.
 *
 * @parent: Parent device for the devices that will be created
 * @node: Node to scan
 * @pre_reloc_only: If true, bind only drivers with the DM_FLAG_PRE_RELOC
 * flag. If false bind all drivers.
 * Return: 0 if OK, -ve on error
 */
static int dm_scan_fdt_node(struct udevice *parent, ofnode parent_node,
			    bool pre_reloc_only)
{
	int ret = 0, err = 0;
	ofnode node;

	if (!ofnode_valid(parent_node))
		return 0;

	for (node = ofnode_first_subnode(parent_node);
	     ofnode_valid(node);
	     node = ofnode_next_subnode(node)) {
		const char *node_name = ofnode_get_name(node);

		if (!ofnode_is_enabled(node)) {
			pr_debug("   - ignoring disabled device\n");
			continue;
		}
		err = lists_bind_fdt(parent, node, NULL, NULL, pre_reloc_only);
		if (err && !ret) {
			ret = err;
			debug("%s: ret=%d\n", node_name, ret);
		}
	}

	if (ret)
		dm_warn("Some drivers failed to bind\n");

	return ret;
}

int dm_scan_fdt_dev(struct udevice *dev)
{
	return dm_scan_fdt_node(dev, dev_ofnode(dev),
				gd->flags & GD_FLG_RELOC ? false : true);
}

int dm_scan_fdt(bool pre_reloc_only)
{
	return dm_scan_fdt_node(gd->dm_root, ofnode_root(), pre_reloc_only);
}

static int dm_scan_fdt_ofnode_path(const char *path, bool pre_reloc_only)
{
	ofnode node;

	node = ofnode_path(path);

	return dm_scan_fdt_node(gd->dm_root, node, pre_reloc_only);
}

int dm_extended_scan(bool pre_reloc_only)
{
	int ret, i;
	const char * const nodes[] = {
		"/chosen",
		"/clocks",
		"/firmware"
	};

	ret = dm_scan_fdt(pre_reloc_only);
	if (ret) {
		debug("dm_scan_fdt() failed: %d\n", ret);
		return ret;
	}

	/* Some nodes aren't devices themselves but may contain some */
	for (i = 0; i < ARRAY_SIZE(nodes); i++) {
		ret = dm_scan_fdt_ofnode_path(nodes[i], pre_reloc_only);
		if (ret) {
			debug("dm_scan_fdt() scan for %s failed: %d\n",
			      nodes[i], ret);
			return ret;
		}
	}

	return ret;
}
#endif

__weak int dm_scan_other(bool pre_reloc_only)
{
	return 0;
}

#if CONFIG_IS_ENABLED(OF_PLATDATA_INST) && CONFIG_IS_ENABLED(READ_ONLY)
void *dm_priv_to_rw(void *priv)
{
	long offset = priv - (void *)__priv_data_start;

	return gd_dm_priv_base() + offset;
}
#endif

static int dm_probe_devices(struct udevice *dev, bool pre_reloc_only)
{
	ofnode node = dev_ofnode(dev);
	struct udevice *child;
	int ret;

	if (pre_reloc_only &&
	    (!ofnode_valid(node) || !ofnode_pre_reloc(node)) &&
	    !(dev->driver->flags & DM_FLAG_PRE_RELOC))
		goto probe_children;

	if (dev_get_flags(dev) & DM_FLAG_PROBE_AFTER_BIND) {
		ret = device_probe(dev);
		if (ret)
			return ret;
	}

probe_children:
	list_for_each_entry(child, &dev->child_head, sibling_node)
		dm_probe_devices(child, pre_reloc_only);

	return 0;
}

/**
 * dm_scan() - Scan tables to bind devices
 *
 * Runs through the driver_info tables and binds the devices it finds. Then runs
 * through the devicetree nodes. Finally calls dm_scan_other() to add any
 * special devices
 *
 * @pre_reloc_only: If true, bind only nodes with special devicetree properties,
 * or drivers with the DM_FLAG_PRE_RELOC flag. If false bind all drivers.
 */
static int dm_scan(bool pre_reloc_only)
{
	int ret;

	ret = dm_scan_plat(pre_reloc_only);
	if (ret) {
		debug("dm_scan_plat() failed: %d\n", ret);
		return ret;
	}

	if (CONFIG_IS_ENABLED(OF_REAL)) {
		ret = dm_extended_scan(pre_reloc_only);
		if (ret) {
			debug("dm_extended_scan() failed: %d\n", ret);
			return ret;
		}
	}

	ret = dm_scan_other(pre_reloc_only);
	if (ret)
		return ret;

	return dm_probe_devices(gd->dm_root, pre_reloc_only);
}

int dm_init_and_scan(bool pre_reloc_only)
{
	int ret;

	ret = dm_init(CONFIG_IS_ENABLED(OF_LIVE));
	if (ret) {
		debug("dm_init() failed: %d\n", ret);
		return ret;
	}
	if (!CONFIG_IS_ENABLED(OF_PLATDATA_INST)) {
		ret = dm_scan(pre_reloc_only);
		if (ret) {
			log_debug("dm_scan() failed: %d\n", ret);
			return ret;
		}
	}
	if (CONFIG_IS_ENABLED(DM_EVENT)) {
		ret = event_notify_null(gd->flags & GD_FLG_RELOC ?
					EVT_DM_POST_INIT_R :
					EVT_DM_POST_INIT_F);
		if (ret)
			return log_msg_ret("ev", ret);
	}

	return 0;
}

void dm_get_stats(int *device_countp, int *uclass_countp)
{
	*device_countp = device_get_decendent_count(gd->dm_root);
	*uclass_countp = uclass_get_count();
}

void dev_collect_stats(struct dm_stats *stats, const struct udevice *parent)
{
	const struct udevice *dev;
	int i;

	stats->dev_count++;
	stats->dev_size += sizeof(struct udevice);
	stats->dev_name_size += strlen(parent->name) + 1;
	for (i = 0; i < DM_TAG_ATTACH_COUNT; i++) {
		int size = dev_get_attach_size(parent, i);

		if (size ||
		    (i == DM_TAG_DRIVER_DATA && parent->driver_data)) {
			stats->attach_count[i]++;
			stats->attach_size[i] += size;
			stats->attach_count_total++;
			stats->attach_size_total += size;
		}
	}

	list_for_each_entry(dev, &parent->child_head, sibling_node)
		dev_collect_stats(stats, dev);
}

void uclass_collect_stats(struct dm_stats *stats)
{
	struct uclass *uc;

	list_for_each_entry(uc, gd->uclass_root, sibling_node) {
		int size;

		stats->uc_count++;
		stats->uc_size += sizeof(struct uclass);
		size = uc->uc_drv->priv_auto;
		if (size) {
			stats->uc_attach_count++;
			stats->uc_attach_size += size;
		}
	}
}

void dm_get_mem(struct dm_stats *stats)
{
	memset(stats, '\0', sizeof(*stats));
	dev_collect_stats(stats, gd->dm_root);
	uclass_collect_stats(stats);
	dev_tag_collect_stats(stats);

	stats->total_size = stats->dev_size + stats->uc_size +
		stats->attach_size_total + stats->uc_attach_size +
		stats->tag_size;
}

#if CONFIG_IS_ENABLED(ACPIGEN)
static int root_acpi_get_name(const struct udevice *dev, char *out_name)
{
	return acpi_copy_name(out_name, "\\_SB");
}

struct acpi_ops root_acpi_ops = {
	.get_name	= root_acpi_get_name,
};
#endif

/* This is the root driver - all drivers are children of this */
U_BOOT_DRIVER(root_driver) = {
	.name	= "root_driver",
	.id	= UCLASS_ROOT,
	ACPI_OPS_PTR(&root_acpi_ops)
};

/* This is the root uclass */
UCLASS_DRIVER(root) = {
	.name	= "root",
	.id	= UCLASS_ROOT,
};
