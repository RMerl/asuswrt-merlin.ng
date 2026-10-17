// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Marek Vasut <marex@denx.de>
 */

#define LOG_CATEGORY LOGC_DM

#include <common.h>
#include <errno.h>
#include <log.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/platdata.h>
#include <dm/uclass.h>
#include <dm/util.h>
#include <fdtdec.h>
#include <linux/compiler.h>

struct driver *lists_driver_lookup_name(const char *name)
{
	struct driver *drv =
		ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct driver *entry;

	for (entry = drv; entry != drv + n_ents; entry++) {
		if (!strcmp(name, entry->name))
			return entry;
	}

	/* Not found */
	return NULL;
}

struct uclass_driver *lists_uclass_lookup(enum uclass_id id)
{
	struct uclass_driver *uclass =
		ll_entry_start(struct uclass_driver, uclass_driver);
	const int n_ents = ll_entry_count(struct uclass_driver, uclass_driver);
	struct uclass_driver *entry;

	for (entry = uclass; entry != uclass + n_ents; entry++) {
		if (entry->id == id)
			return entry;
	}

	return NULL;
}

static int bind_drivers_pass(struct udevice *parent, bool pre_reloc_only)
{
	struct driver_info *info =
		ll_entry_start(struct driver_info, driver_info);
	const int n_ents = ll_entry_count(struct driver_info, driver_info);
	bool missing_parent = false;
	int result = 0;
	int idx;

	/*
	 * Do one iteration through the driver_info records. For of-platdata,
	 * bind only devices whose parent is already bound. If we find any
	 * device we can't bind, set missing_parent to true, which will cause
	 * this function to be called again.
	 */
	for (idx = 0; idx < n_ents; idx++) {
		struct udevice *par = parent;
		const struct driver_info *entry = info + idx;
		struct driver_rt *drt = gd_dm_driver_rt() + idx;
		struct udevice *dev;
		int ret;

		if (CONFIG_IS_ENABLED(OF_PLATDATA)) {
			int parent_idx = driver_info_parent_id(entry);

			if (drt->dev)
				continue;

			if (CONFIG_IS_ENABLED(OF_PLATDATA_PARENT) &&
			    parent_idx != -1) {
				struct driver_rt *parent_drt;

				parent_drt = gd_dm_driver_rt() + parent_idx;
				if (!parent_drt->dev) {
					missing_parent = true;
					continue;
				}

				par = parent_drt->dev;
			}
		}
		ret = device_bind_by_name(par, pre_reloc_only, entry, &dev);
		if (!ret) {
			if (CONFIG_IS_ENABLED(OF_PLATDATA))
				drt->dev = dev;
		} else if (ret != -EPERM) {
			dm_warn("No match for driver '%s'\n", entry->name);
			if (!result || ret != -ENOENT)
				result = ret;
		}
	}

	return result ? result : missing_parent ? -EAGAIN : 0;
}

int lists_bind_drivers(struct udevice *parent, bool pre_reloc_only)
{
	int result = 0;
	int pass;

	/*
	 * 10 passes is 10 levels deep in the devicetree, which is plenty. If
	 * OF_PLATDATA_PARENT is not enabled, then bind_drivers_pass() will
	 * always succeed on the first pass.
	 */
	for (pass = 0; pass < 10; pass++) {
		int ret;

		ret = bind_drivers_pass(parent, pre_reloc_only);
		if (!result || result == -EAGAIN)
			result = ret;
		if (ret != -EAGAIN)
			break;
	}

	return result;
}

int device_bind_driver(struct udevice *parent, const char *drv_name,
		       const char *dev_name, struct udevice **devp)
{
	return device_bind_driver_to_node(parent, drv_name, dev_name,
					  ofnode_null(), devp);
}

int device_bind_driver_to_node(struct udevice *parent, const char *drv_name,
			       const char *dev_name, ofnode node,
			       struct udevice **devp)
{
	struct driver *drv;
	int ret;

	drv = lists_driver_lookup_name(drv_name);
	if (!drv) {
		debug("Cannot find driver '%s'\n", drv_name);
		return -ENOENT;
	}
	ret = device_bind_with_driver_data(parent, drv, dev_name, 0 /* data */,
					   node, devp);

	return ret;
}

#if CONFIG_IS_ENABLED(OF_REAL)
/**
 * driver_check_compatible() - Check if a driver matches a compatible string
 *
 * @param of_match:	List of compatible strings to match
 * @param of_idp:	Returns the match that was found
 * @param compat:	The compatible string to search for
 * Return: 0 if there is a match, -ENOENT if no match
 */
static int driver_check_compatible(const struct udevice_id *of_match,
				   const struct udevice_id **of_idp,
				   const char *compat)
{
	if (!of_match)
		return -ENOENT;

	while (of_match->compatible) {
		if (!strcmp(of_match->compatible, compat)) {
			*of_idp = of_match;
			return 0;
		}
		of_match++;
	}

	return -ENOENT;
}

int lists_bind_fdt(struct udevice *parent, ofnode node, struct udevice **devp,
		   struct driver *drv, bool pre_reloc_only)
{
	struct driver *driver = ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	const struct udevice_id *id;
	struct driver *entry;
	struct udevice *dev;
	bool found = false;
	const char *name, *compat_list, *compat;
	int compat_length, i;
	int result = 0;
	int ret = 0;

	if (devp)
		*devp = NULL;
	name = ofnode_get_name(node);
	log_debug("bind node %s\n", name);

	compat_list = ofnode_get_property(node, "compatible", &compat_length);
	if (!compat_list) {
		if (compat_length == -FDT_ERR_NOTFOUND) {
			log_debug("Device '%s' has no compatible string\n",
				  name);
			return 0;
		}

		dm_warn("Device tree error at node '%s'\n", name);
		return compat_length;
	}

	/*
	 * Walk through the compatible string list, attempting to match each
	 * compatible string in order such that we match in order of priority
	 * from the first string to the last.
	 */
	for (i = 0; i < compat_length; i += strlen(compat) + 1) {
		compat = compat_list + i;
		log_debug("   - attempt to match compatible string '%s'\n",
			  compat);

		id = NULL;
		for (entry = driver; entry != driver + n_ents; entry++) {
			if (drv) {
				if (drv != entry)
					continue;
				if (!entry->of_match)
					break;
			}
			ret = driver_check_compatible(entry->of_match, &id,
						      compat);
			if (!ret)
				break;
		}
		if (entry == driver + n_ents)
			continue;

		if (pre_reloc_only) {
			if (!ofnode_pre_reloc(node) &&
			    !(entry->flags & DM_FLAG_PRE_RELOC)) {
				log_debug("Skipping device pre-relocation\n");
				return 0;
			}
		}

		if (entry->of_match)
			log_debug("   - found match at '%s': '%s' matches '%s'\n",
				  entry->name, entry->of_match->compatible,
				  id->compatible);
		ret = device_bind_with_driver_data(parent, entry, name,
						   id ? id->data : 0, node,
						   &dev);
		if (ret == -ENODEV) {
			log_debug("Driver '%s' refuses to bind\n", entry->name);
			continue;
		}
		if (ret) {
			dm_warn("Error binding driver '%s': %d\n", entry->name,
				ret);
			return log_msg_ret("bind", ret);
		} else {
			found = true;
			if (devp)
				*devp = dev;
		}
		break;
	}

	if (!found && !result && ret != -ENODEV)
		log_debug("No match for node '%s'\n", name);

	return result;
}
#endif
