// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <mapmem.h>
#include <sort.h>
#include <dm/root.h>
#include <dm/util.h>
#include <dm/uclass-internal.h>

/**
 * struct sort_info - information used for sorting
 *
 * @dev: List of devices
 * @alloced: Maximum number of devices in @dev
 */
struct sort_info {
	struct udevice **dev;
	int size;
};

static int h_cmp_uclass_id(const void *d1, const void *d2)
{
	const struct udevice *const *dev1 = d1;
	const struct udevice *const *dev2 = d2;

	return device_get_uclass_id(*dev1) - device_get_uclass_id(*dev2);
}

static void show_devices(struct udevice *dev, int depth, int last_flag,
			 struct udevice **devs)
{
	int i, is_last;
	struct udevice *child;
	u32 flags = dev_get_flags(dev);

	/* print the first 20 characters to not break the tree-format. */
	printf(CONFIG_IS_ENABLED(USE_TINY_PRINTF) ? " %s  %d  [ %c ]   %s  " :
	       " %-10.10s  %3d  [ %c ]   %-20.20s  ", dev->uclass->uc_drv->name,
	       dev_get_uclass_index(dev, NULL),
	       flags & DM_FLAG_ACTIVATED ? '+' : ' ', dev->driver->name);

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

	if (devs) {
		int count;
		int i;

		count = 0;
		device_foreach_child(child, dev)
			devs[count++] = child;
		qsort(devs, count, sizeof(struct udevice *), h_cmp_uclass_id);

		for (i = 0; i < count; i++) {
			show_devices(devs[i], depth + 1,
				     (last_flag << 1) | (i == count - 1),
				     devs + count);
		}
	} else {
		device_foreach_child(child, dev) {
			is_last = list_is_last(&child->sibling_node,
					       &dev->child_head);
			show_devices(child, depth + 1,
				     (last_flag << 1) | is_last, NULL);
		}
	}
}

static void dm_dump_tree_single(struct udevice *dev, bool sort)
{
	int dev_count, uclasses;
	struct udevice **devs = NULL;

	if (sort) {
		dm_get_stats(&dev_count, &uclasses);
		devs = calloc(dev_count, sizeof(struct udevice *));
		if (!devs) {
			printf("(out of memory)\n");
			return;
		}
	}
	show_devices(dev, -1, 0, devs);
	free(devs);
}

static void dm_dump_tree_recursive(struct udevice *dev, char *dev_name,
				   bool extended, bool sort)
{
	struct udevice *child;
	size_t len;

	len = strlen(dev_name);

	device_foreach_child(child, dev) {
		if (extended) {
			if (!strncmp(child->name, dev_name, len)) {
				dm_dump_tree_single(child, sort);
				continue;
			}
		} else {
			if (!strcmp(child->name, dev_name)) {
				dm_dump_tree_single(child, sort);
				continue;
			}
		}
		dm_dump_tree_recursive(child, dev_name, extended, sort);
	}
}

void dm_dump_tree(char *dev_name, bool extended, bool sort)
{
	struct udevice *root;

	printf(" Class     Index  Probed  Driver                Name\n");
	printf("-----------------------------------------------------------\n");

	root = dm_root();
	if (!root)
		return;

	if (!dev_name || !strcmp(dev_name, "root")) {
		dm_dump_tree_single(root, sort);
		return;
	}

	dm_dump_tree_recursive(root, dev_name, extended, sort);
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
	printf("%-3i %c %s @ %08lx", index,
	       dev_get_flags(dev) & DM_FLAG_ACTIVATED ? '*' : ' ',
	       dev->name, (ulong)map_to_sysmem(dev));
	if (dev->seq_ != -1)
		printf(", seq %d", dev_seq(dev));
	puts("\n");
}

static void dm_dump_uclass_single(enum uclass_id id)
{
	struct uclass *uc;
	struct udevice *dev;
	int i = 0, ret;

	ret = uclass_get(id, &uc);
	if (ret)
		return;

	printf("uclass %d: %s\n", id, uc->uc_drv->name);
	uclass_foreach_dev(dev, uc) {
		dm_display_line(dev, i);
		i++;
	}
	puts("\n");
}

void dm_dump_uclass(char *uclass, bool extended)
{
	struct uclass *uc;
	enum uclass_id id;
	bool matching;
	int ret;

	matching = !!(uclass && strcmp(uclass, "root"));

	for (id = 0; id < UCLASS_COUNT; id++) {
		ret = uclass_get(id, &uc);
		if (ret)
			continue;

		if (matching) {
			if (extended) {
				if (!strncmp(uc->uc_drv->name, uclass,
					     strlen(uclass)))
					dm_dump_uclass_single(id);
			} else {
				if (!strcmp(uc->uc_drv->name, uclass))
					dm_dump_uclass_single(id);
			}
		} else {
			dm_dump_uclass_single(id);
		}
	}
}

void dm_dump_driver_compat(void)
{
	struct driver *d = ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct driver *entry;
	const struct udevice_id *match;

	puts("Driver                Compatible\n");
	puts("--------------------------------\n");
	for (entry = d; entry < d + n_ents; entry++) {
		match = entry->of_match;

		printf("%-20.20s", entry->name);
		if (match) {
			printf("  %s", match->compatible);
			match++;
		}
		printf("\n");

		for (; match && match->compatible; match++)
			printf("%-20.20s  %s\n", "", match->compatible);
	}
}

void dm_dump_drivers(void)
{
	struct driver *d = ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct driver *entry;
	struct udevice *udev;
	struct uclass *uc;
	int ret;
	int i;

	puts("Driver                    uid uclass               Devices\n");
	puts("----------------------------------------------------------\n");

	for (entry = d; entry < d + n_ents; entry++) {
		ret = uclass_get(entry->id, &uc);

		printf("%-25.25s %-3.3d %-20.20s ", entry->name, entry->id,
		       !ret ? uc->uc_drv->name : "<no uclass>");

		if (ret) {
			puts("\n");
			continue;
		}

		i = 0;
		uclass_foreach_dev(udev, uc) {
			if (udev->driver != entry)
				continue;
			if (i)
				printf("%-51.51s", "");

			printf("%-25.25s\n", udev->name);
			i++;
		}
		if (!i)
			puts("<none>\n");
	}
}

void dm_dump_static_driver_info(void)
{
	struct driver_info *drv = ll_entry_start(struct driver_info,
						 driver_info);
	const int n_ents = ll_entry_count(struct driver_info, driver_info);
	struct driver_info *entry;

	puts("Driver                    Address\n");
	puts("---------------------------------\n");
	for (entry = drv; entry != drv + n_ents; entry++)
		printf("%-25.25s %p\n", entry->name, entry->plat);
}

void dm_dump_mem(struct dm_stats *stats)
{
	int total, total_delta;
	int i;

	/* Support SPL printf() */
	printf("Struct sizes: udevice %x, driver %x, uclass %x, uc_driver %x\n",
	       (int)sizeof(struct udevice), (int)sizeof(struct driver),
	       (int)sizeof(struct uclass), (int)sizeof(struct uclass_driver));
	printf("Memory: device %x:%x, device names %x, uclass %x:%x\n",
	       stats->dev_count, stats->dev_size, stats->dev_name_size,
	       stats->uc_count, stats->uc_size);
	printf("\n");
	printf("%-15s  %5s  %5s  %5s  %5s  %5s\n", "Attached type", "Count",
	       "Size", "Cur", "Tags", "Save");
	printf("%-15s  %5s  %5s  %5s  %5s  %5s\n", "---------------", "-----",
	       "-----", "-----", "-----", "-----");
	total_delta = 0;
	for (i = 0; i < DM_TAG_ATTACH_COUNT; i++) {
		int cur_size, new_size, delta;

		cur_size = stats->dev_count * sizeof(struct udevice);
		new_size = stats->dev_count * (sizeof(struct udevice) -
			sizeof(void *));
		/*
		 * Let's assume we can fit each dmtag_node into 32 bits. We can
		 * limit the 'tiny tags' feature to SPL with
		 * CONFIG_SPL_SYS_MALLOC_F_LEN <= 64KB, so needing 14 bits to
		 * point to anything in that region (with 4-byte alignment).
		 * So:
		 *    4 bits for tag
		 *    14 bits for offset of dev
		 *    14 bits for offset of data
		 */
		new_size += stats->attach_count[i] * sizeof(u32);
		delta = cur_size - new_size;
		total_delta += delta;
		printf("%-16s %5x %6x %6x %6x %6x (%d)\n", tag_get_name(i),
		       stats->attach_count[i], stats->attach_size[i],
		       cur_size, new_size, delta > 0 ? delta : 0, delta);
	}
	printf("%-16s %5x %6x\n", "uclass", stats->uc_attach_count,
	       stats->uc_attach_size);
	printf("%-16s %5x %6x  %5s  %5s  %6x (%d)\n", "Attached total",
	       stats->attach_count_total + stats->uc_attach_count,
	       stats->attach_size_total + stats->uc_attach_size, "", "",
	       total_delta > 0 ? total_delta : 0, total_delta);
	printf("%-16s %5x %6x\n", "tags", stats->tag_count, stats->tag_size);
	printf("\n");
	printf("Total size: %x (%d)\n", stats->total_size, stats->total_size);
	printf("\n");

	total = stats->total_size;
	total -= total_delta;
	printf("With tags:       %x (%d)\n", total, total);

	/* Use singly linked lists in struct udevice (3 nodes in each) */
	total -= sizeof(void *) * 3 * stats->dev_count;
	printf("- singly-linked: %x (%d)\n", total, total);

	/* Use an index into the struct_driver list instead of a pointer */
	total = total + stats->dev_count * (1 - sizeof(void *));
	printf("- driver index:  %x (%d)\n", total, total);

	/* Same with the uclass */
	total = total + stats->dev_count * (1 - sizeof(void *));
	printf("- uclass index:  %x (%d)\n", total, total);

	/* Drop the device name */
	printf("Drop device name (not SRAM): %x (%d)\n", stats->dev_name_size,
	       stats->dev_name_size);
}
