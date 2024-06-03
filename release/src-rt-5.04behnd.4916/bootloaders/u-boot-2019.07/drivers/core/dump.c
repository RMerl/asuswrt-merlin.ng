// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <dm/root.h>
#include <dm/util.h>
#include <dm/uclass-internal.h>

static void show_devices(struct udevice *dev, int depth, int last_flag)
{
	int i, is_last;
	struct udevice *child;

	/* print the first 20 characters to not break the tree-format. */
	printf(" %-10.10s  %2d  [ %c ]   %-20.20s  ", dev->uclass->uc_drv->name,
	       dev_get_uclass_index(dev, NULL),
	       dev->flags & DM_FLAG_ACTIVATED ? '+' : ' ', dev->driver->name);

	for (i = depth; i >= 0; i--) {
		is_last = (last_flag >> i) & 1;
		if (i) {
			if (is_last)
				printf("    ");
			else
				printf("|   ");
		} else {
			if (is_last)
				printf("`-- ");
			else
				printf("|-- ");
		}
	}

	printf("%s\n", dev->name);

	list_for_each_entry(child, &dev->child_head, sibling_node) {
		is_last = list_is_last(&child->sibling_node, &dev->child_head);
		show_devices(child, depth + 1, (last_flag << 1) | is_last);
	}
}

void dm_dump_all(void)
{
	struct udevice *root;

	root = dm_root();
	if (root) {
		printf(" Class     Index  Probed  Driver                Name\n");
		printf("-----------------------------------------------------------\n");
		show_devices(root, -1, 0);
	}
}

/**
 * dm_display_line() - Display information about a single device
 *
 * Displays a single line of information with an option prefix
 *
 * @dev:	Device to display
 */
static void dm_display_line(struct udevice *dev, int index)
{
	printf("%i %c %s @ %08lx", index,
	       dev->flags & DM_FLAG_ACTIVATED ? '*' : ' ',
	       dev->name, (ulong)map_to_sysmem(dev));
	if (dev->seq != -1 || dev->req_seq != -1)
		printf(", seq %d, (req %d)", dev->seq, dev->req_seq);
	puts("\n");
}

void dm_dump_uclass(void)
{
	struct uclass *uc;
	int ret;
	int id;

	for (id = 0; id < UCLASS_COUNT; id++) {
		struct udevice *dev;
		int i = 0;

		ret = uclass_get(id, &uc);
		if (ret)
			continue;

		printf("uclass %d: %s\n", id, uc->uc_drv->name);
		if (list_empty(&uc->dev_head))
			continue;
		uclass_foreach_dev(dev, uc) {
			dm_display_line(dev, i);
			i++;
		}
		puts("\n");
	}
}
