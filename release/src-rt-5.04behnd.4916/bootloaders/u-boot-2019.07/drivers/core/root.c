// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <linux/libfdt.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/of.h>
#include <dm/of_access.h>
#include <dm/platdata.h>
#include <dm/read.h>
#include <dm/root.h>
#include <dm/uclass.h>
#include <dm/util.h>
#include <linux/list.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct driver_info root_info = {
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
		new_gd->uclass_root.next->prev = &new_gd->uclass_root;
		new_gd->uclass_root.prev->next = &new_gd->uclass_root;
	}
}

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
void fix_drivers(void)
{
	struct driver *drv =
		ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct driver *entry;

	for (entry = drv; entry != drv + n_ents; entry++) {
		if (entry->of_match)
			entry->of_match = (const struct udevice_id *)
				((u32)entry->of_match + gd->reloc_off);
		if (entry->bind)
			entry->bind += gd->reloc_off;
		if (entry->probe)
			entry->probe += gd->reloc_off;
		if (entry->remove)
			entry->remove += gd->reloc_off;
		if (entry->unbind)
			entry->unbind += gd->reloc_off;
		if (entry->ofdata_to_platdata)
			entry->ofdata_to_platdata += gd->reloc_off;
		if (entry->child_post_bind)
			entry->child_post_bind += gd->reloc_off;
		if (entry->child_pre_probe)
			entry->child_pre_probe += gd->reloc_off;
		if (entry->child_post_remove)
			entry->child_post_remove += gd->reloc_off;
		/* OPS are fixed in every uclass post_probe function */
		if (entry->ops)
			entry->ops += gd->reloc_off;
	}
}

void fix_uclass(void)
{
	struct uclass_driver *uclass =
		ll_entry_start(struct uclass_driver, uclass);
	const int n_ents = ll_entry_count(struct uclass_driver, uclass);
	struct uclass_driver *entry;

	for (entry = uclass; entry != uclass + n_ents; entry++) {
		if (entry->post_bind)
			entry->post_bind += gd->reloc_off;
		if (entry->pre_unbind)
			entry->pre_unbind += gd->reloc_off;
		if (entry->pre_probe)
			entry->pre_probe += gd->reloc_off;
		if (entry->post_probe)
			entry->post_probe += gd->reloc_off;
		if (entry->pre_remove)
			entry->pre_remove += gd->reloc_off;
		if (entry->child_post_bind)
			entry->child_post_bind += gd->reloc_off;
		if (entry->child_pre_probe)
			entry->child_pre_probe += gd->reloc_off;
		if (entry->init)
			entry->init += gd->reloc_off;
		if (entry->destroy)
			entry->destroy += gd->reloc_off;
		/* FIXME maybe also need to fix these ops */
		if (entry->ops)
			entry->ops += gd->reloc_off;
	}
}

void fix_devices(void)
{
	struct driver_info *dev =
		ll_entry_start(struct driver_info, driver_info);
	const int n_ents = ll_entry_count(struct driver_info, driver_info);
	struct driver_info *entry;

	for (entry = dev; entry != dev + n_ents; entry++) {
		if (entry->platdata)
			entry->platdata += gd->reloc_off;
	}
}

#endif

int dm_init(bool of_live)
{
	int ret;

	if (gd->dm_root) {
		dm_warn("Virtual root driver already exists!\n");
		return -EINVAL;
	}
	INIT_LIST_HEAD(&DM_UCLASS_ROOT_NON_CONST);

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	fix_drivers();
	fix_uclass();
	fix_devices();
#endif

	ret = device_bind_by_name(NULL, false, &root_info, &DM_ROOT_NON_CONST);
	if (ret)
		return ret;
#if CONFIG_IS_ENABLED(OF_CONTROL)
# if CONFIG_IS_ENABLED(OF_LIVE)
	if (of_live)
		DM_ROOT_NON_CONST->node = np_to_ofnode(gd->of_root);
	else
#endif
		DM_ROOT_NON_CONST->node = offset_to_ofnode(0);
#endif
	ret = device_probe(DM_ROOT_NON_CONST);
	if (ret)
		return ret;

	return 0;
}

int dm_uninit(void)
{
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

int dm_scan_platdata(bool pre_reloc_only)
{
	int ret;

	ret = lists_bind_drivers(DM_ROOT_NON_CONST, pre_reloc_only);
	if (ret == -ENOENT) {
		dm_warn("Some drivers were not found\n");
		ret = 0;
	}

	return ret;
}

#if CONFIG_IS_ENABLED(OF_LIVE)
static int dm_scan_fdt_live(struct udevice *parent,
			    const struct device_node *node_parent,
			    bool pre_reloc_only)
{
	struct device_node *np;
	int ret = 0, err;

	for (np = node_parent->child; np; np = np->sibling) {
		/* "chosen" node isn't a device itself but may contain some: */
		if (!strcmp(np->name, "chosen")) {
			pr_debug("parsing subnodes of \"chosen\"\n");

			err = dm_scan_fdt_live(parent, np, pre_reloc_only);
			if (err && !ret)
				ret = err;
			continue;
		}

		if (!of_device_is_available(np)) {
			pr_debug("   - ignoring disabled device\n");
			continue;
		}
		err = lists_bind_fdt(parent, np_to_ofnode(np), NULL,
				     pre_reloc_only);
		if (err && !ret) {
			ret = err;
			debug("%s: ret=%d\n", np->name, ret);
		}
	}

	if (ret)
		dm_warn("Some drivers failed to bind\n");

	return ret;
}
#endif /* CONFIG_IS_ENABLED(OF_LIVE) */

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
/**
 * dm_scan_fdt_node() - Scan the device tree and bind drivers for a node
 *
 * This scans the subnodes of a device tree node and and creates a driver
 * for each one.
 *
 * @parent: Parent device for the devices that will be created
 * @blob: Pointer to device tree blob
 * @offset: Offset of node to scan
 * @pre_reloc_only: If true, bind only drivers with the DM_FLAG_PRE_RELOC
 * flag. If false bind all drivers.
 * @return 0 if OK, -ve on error
 */
static int dm_scan_fdt_node(struct udevice *parent, const void *blob,
			    int offset, bool pre_reloc_only)
{
	int ret = 0, err;

	for (offset = fdt_first_subnode(blob, offset);
	     offset > 0;
	     offset = fdt_next_subnode(blob, offset)) {
		const char *node_name = fdt_get_name(blob, offset, NULL);

		/*
		 * The "chosen" and "firmware" nodes aren't devices
		 * themselves but may contain some:
		 */
		if (!strcmp(node_name, "chosen") ||
		    !strcmp(node_name, "firmware")) {
			pr_debug("parsing subnodes of \"%s\"\n", node_name);

			err = dm_scan_fdt_node(parent, blob, offset,
					       pre_reloc_only);
			if (err && !ret)
				ret = err;
			continue;
		}

		if (!fdtdec_get_is_enabled(blob, offset)) {
			pr_debug("   - ignoring disabled device\n");
			continue;
		}
		err = lists_bind_fdt(parent, offset_to_ofnode(offset), NULL,
				     pre_reloc_only);
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
	if (!dev_of_valid(dev))
		return 0;

#if CONFIG_IS_ENABLED(OF_LIVE)
	if (of_live_active())
		return dm_scan_fdt_live(dev, dev_np(dev),
				gd->flags & GD_FLG_RELOC ? false : true);
	else
#endif
	return dm_scan_fdt_node(dev, gd->fdt_blob, dev_of_offset(dev),
				gd->flags & GD_FLG_RELOC ? false : true);
}

int dm_scan_fdt(const void *blob, bool pre_reloc_only)
{
#if CONFIG_IS_ENABLED(OF_LIVE)
	if (of_live_active())
		return dm_scan_fdt_live(gd->dm_root, gd->of_root,
					pre_reloc_only);
	else
#endif
	return dm_scan_fdt_node(gd->dm_root, blob, 0, pre_reloc_only);
}
#else
static int dm_scan_fdt_node(struct udevice *parent, const void *blob,
			    int offset, bool pre_reloc_only)
{
	return 0;
}
#endif

static int dm_scan_fdt_ofnode_path(const char *path, bool pre_reloc_only)
{
	ofnode node;

	node = ofnode_path(path);
	if (!ofnode_valid(node))
		return 0;

#if CONFIG_IS_ENABLED(OF_LIVE)
	if (of_live_active())
		return dm_scan_fdt_live(gd->dm_root, node.np, pre_reloc_only);
#endif
	return dm_scan_fdt_node(gd->dm_root, gd->fdt_blob, node.of_offset,
				pre_reloc_only);
}

int dm_extended_scan_fdt(const void *blob, bool pre_reloc_only)
{
	int ret;

	ret = dm_scan_fdt(blob, pre_reloc_only);
	if (ret) {
		debug("dm_scan_fdt() failed: %d\n", ret);
		return ret;
	}

	ret = dm_scan_fdt_ofnode_path("/clocks", pre_reloc_only);
	if (ret) {
		debug("scan for /clocks failed: %d\n", ret);
		return ret;
	}

	ret = dm_scan_fdt_ofnode_path("/firmware", pre_reloc_only);
	if (ret)
		debug("scan for /firmware failed: %d\n", ret);

	return ret;
}

__weak int dm_scan_other(bool pre_reloc_only)
{
	return 0;
}

int dm_init_and_scan(bool pre_reloc_only)
{
	int ret;

	ret = dm_init(IS_ENABLED(CONFIG_OF_LIVE));
	if (ret) {
		debug("dm_init() failed: %d\n", ret);
		return ret;
	}
	ret = dm_scan_platdata(pre_reloc_only);
	if (ret) {
		debug("dm_scan_platdata() failed: %d\n", ret);
		return ret;
	}

	if (CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)) {
		ret = dm_extended_scan_fdt(gd->fdt_blob, pre_reloc_only);
		if (ret) {
			debug("dm_extended_scan_dt() failed: %d\n", ret);
			return ret;
		}
	}

	ret = dm_scan_other(pre_reloc_only);
	if (ret)
		return ret;

	return 0;
}

/* This is the root driver - all drivers are children of this */
U_BOOT_DRIVER(root_driver) = {
	.name	= "root_driver",
	.id	= UCLASS_ROOT,
};

/* This is the root uclass */
UCLASS_DRIVER(root) = {
	.name	= "root",
	.id	= UCLASS_ROOT,
};
