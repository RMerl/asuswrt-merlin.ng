// SPDX-License-Identifier: GPL-2.0
/*
 * Verified Boot for Embedded (VBE) loading firmware phases
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY LOGC_BOOT

#include <common.h>
#include <dm.h>
#include <bootflow.h>
#include <vbe.h>
#include <version_string.h>
#include <dm/device-internal.h>
#include "vbe_simple.h"

int vbe_simple_fixup_node(ofnode node, struct simple_state *state)
{
	const char *version, *str;
	int ret;

	version = strdup(state->fw_version);
	if (!version)
		return log_msg_ret("dup", -ENOMEM);

	ret = ofnode_write_string(node, "cur-version", version);
	if (ret)
		return log_msg_ret("ver", ret);
	ret = ofnode_write_u32(node, "cur-vernum", state->fw_vernum);
	if (ret)
		return log_msg_ret("num", ret);

	/* Drop the 'U-Boot ' at the start */
	str = version_string;
	if (!strncmp("U-Boot ", str, 7))
		str += 7;
	ret = ofnode_write_string(node, "bootloader-version", str);
	if (ret)
		return log_msg_ret("bl", ret);

	return 0;
}

/**
 * bootmeth_vbe_simple_ft_fixup() - Write out all VBE simple data to the DT
 *
 * @ctx: Context for event
 * @event: Event to process
 * @return 0 if OK, -ve on error
 */
static int bootmeth_vbe_simple_ft_fixup(void *ctx, struct event *event)
{
	oftree tree = event->data.ft_fixup.tree;
	struct udevice *dev;

	/*
	 * Ideally we would have driver model support for fixups, but that does
	 * not exist yet. It is a step too far to try to do this before VBE is
	 * in place.
	 */
	for (vbe_find_first_device(&dev); dev; vbe_find_next_device(&dev)) {
		struct simple_state state;
		ofnode node, subnode, chosen;
		int ret;

		if (strcmp("vbe_simple", dev->driver->name))
			continue;

		/* Check if there is a node to fix up, adding if not */
		chosen = oftree_path(tree, "/chosen");
		if (!ofnode_valid(chosen))
			continue;

		ret = device_probe(dev);
		if (ret) {
			/*
			 * This should become an error when VBE is updated to
			 * only bind this device when a node exists
			 */
			log_debug("VBE device '%s' failed to probe (err=%d)",
				  dev->name, ret);
			return 0;
		}

		ret = ofnode_add_subnode(chosen, "fwupd", &node);
		if (ret && ret != -EEXIST)
			return log_msg_ret("fwu", ret);

		ret = ofnode_add_subnode(node, dev->name, &subnode);
		if (ret && ret != -EEXIST)
			return log_msg_ret("dev", ret);

		/* Copy over the vbe properties for fwupd */
		log_debug("Fixing up: %s\n", dev->name);
		ret = ofnode_copy_props(subnode, dev_ofnode(dev));
		if (ret)
			return log_msg_ret("cp", ret);

		ret = vbe_simple_read_state(dev, &state);
		if (ret)
			return log_msg_ret("read", ret);

		ret = vbe_simple_fixup_node(subnode, &state);
		if (ret)
			return log_msg_ret("fix", ret);
	}

	return 0;
}
EVENT_SPY_FULL(EVT_FT_FIXUP, bootmeth_vbe_simple_ft_fixup);
