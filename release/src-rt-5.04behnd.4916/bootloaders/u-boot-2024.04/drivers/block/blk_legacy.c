// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <blk.h>
#include <part.h>
#include <linux/err.h>

struct blk_driver *blk_driver_lookup_type(int uclass_id)
{
	struct blk_driver *drv = ll_entry_start(struct blk_driver, blk_driver);
	const int n_ents = ll_entry_count(struct blk_driver, blk_driver);
	struct blk_driver *entry;

	for (entry = drv; entry != drv + n_ents; entry++) {
		if (uclass_id == entry->uclass_id)
			return entry;
	}

	/* Not found */
	return NULL;
}

static struct blk_driver *blk_driver_lookup_typename(const char *uclass_idname)
{
	struct blk_driver *drv = ll_entry_start(struct blk_driver, blk_driver);
	const int n_ents = ll_entry_count(struct blk_driver, blk_driver);
	struct blk_driver *entry;

	for (entry = drv; entry != drv + n_ents; entry++) {
		if (!strcmp(uclass_idname, entry->uclass_idname))
			return entry;
	}

	/* Not found */
	return NULL;
}

const char *blk_get_uclass_name(enum uclass_id uclass_id)
{
	struct blk_driver *drv = blk_driver_lookup_type(uclass_id);

	return drv ? drv->uclass_idname : NULL;
}

/**
 * get_desc() - Get the block device descriptor for the given device number
 *
 * @drv:	Legacy block driver
 * @devnum:	Device number (0 = first)
 * @descp:	Returns block device descriptor on success
 * Return: 0 on success, -ENODEV if there is no such device, -ENOSYS if the
 * driver does not provide a way to find a device, or other -ve on other
 * error.
 */
static int get_desc(struct blk_driver *drv, int devnum, struct blk_desc **descp)
{
	if (drv->desc) {
		if (devnum < 0 || devnum >= drv->max_devs)
			return -ENODEV;
		*descp = &drv->desc[devnum];
		return 0;
	}
	if (!drv->get_dev)
		return -ENOSYS;

	return drv->get_dev(devnum, descp);
}

int blk_list_part(enum uclass_id uclass_id)
{
	struct blk_driver *drv;
	struct blk_desc *desc;
	int devnum, ok;
	bool first = true;

	drv = blk_driver_lookup_type(uclass_id);
	if (!drv)
		return -ENOSYS;
	for (ok = 0, devnum = 0; devnum < drv->max_devs; ++devnum) {
		if (get_desc(drv, devnum, &desc))
			continue;
		if (desc->part_type != PART_TYPE_UNKNOWN) {
			++ok;
			if (!first)
				putc('\n');
			part_print(desc);
			first = false;
		}
	}
	if (!ok)
		return -ENODEV;

	return 0;
}

int blk_print_part_devnum(enum uclass_id uclass_id, int devnum)
{
	struct blk_driver *drv = blk_driver_lookup_type(uclass_id);
	struct blk_desc *desc;
	int ret;

	if (!drv)
		return -ENOSYS;
	ret = get_desc(drv, devnum, &desc);
	if (ret)
		return ret;
	if (desc->type == DEV_TYPE_UNKNOWN)
		return -ENOENT;
	part_print(desc);

	return 0;
}

void blk_list_devices(enum uclass_id uclass_id)
{
	struct blk_driver *drv = blk_driver_lookup_type(uclass_id);
	struct blk_desc *desc;
	int i;

	if (!drv)
		return;
	for (i = 0; i < drv->max_devs; ++i) {
		if (get_desc(drv, i, &desc))
			continue;
		if (desc->type == DEV_TYPE_UNKNOWN)
			continue;  /* list only known devices */
		printf("Device %d: ", i);
		dev_print(desc);
	}
}

int blk_print_device_num(enum uclass_id uclass_id, int devnum)
{
	struct blk_driver *drv = blk_driver_lookup_type(uclass_id);
	struct blk_desc *desc;
	int ret;

	if (!drv)
		return -ENOSYS;
	ret = get_desc(drv, devnum, &desc);
	if (ret)
		return ret;
	printf("\n%s device %d: ", drv->uclass_idname, devnum);
	dev_print(desc);

	return 0;
}

int blk_show_device(enum uclass_id uclass_id, int devnum)
{
	struct blk_driver *drv = blk_driver_lookup_type(uclass_id);
	struct blk_desc *desc;
	int ret;

	if (!drv)
		return -ENOSYS;
	printf("\nDevice %d: ", devnum);
	if (devnum >= drv->max_devs) {
		puts("unknown device\n");
		return -ENODEV;
	}
	ret = get_desc(drv, devnum, &desc);
	if (ret)
		return ret;
	dev_print(desc);

	if (desc->type == DEV_TYPE_UNKNOWN)
		return -ENOENT;

	return 0;
}

struct blk_desc *blk_get_devnum_by_uclass_id(enum uclass_id uclass_id, int devnum)
{
	struct blk_driver *drv = blk_driver_lookup_type(uclass_id);
	struct blk_desc *desc;

	if (!drv)
		return NULL;

	if (get_desc(drv, devnum, &desc))
		return NULL;

	return desc;
}

int blk_dselect_hwpart(struct blk_desc *desc, int hwpart)
{
	struct blk_driver *drv = blk_driver_lookup_type(desc->uclass_id);

	if (!drv)
		return -ENOSYS;
	if (drv->select_hwpart)
		return drv->select_hwpart(desc, hwpart);

	return 0;
}

struct blk_desc *blk_get_devnum_by_uclass_idname(const char *uclass_idname, int devnum)
{
	struct blk_driver *drv = blk_driver_lookup_typename(uclass_idname);
	struct blk_desc *desc;

	if (!drv)
		return NULL;

	if (get_desc(drv, devnum, &desc))
		return NULL;

	return desc;
}

ulong blk_read_devnum(enum uclass_id uclass_id, int devnum, lbaint_t start,
		      lbaint_t blkcnt, void *buffer)
{
	struct blk_driver *drv = blk_driver_lookup_type(uclass_id);
	struct blk_desc *desc;
	ulong n;
	int ret;

	if (!drv)
		return -ENOSYS;
	ret = get_desc(drv, devnum, &desc);
	if (ret)
		return ret;
	n = desc->block_read(desc, start, blkcnt, buffer);
	if (IS_ERR_VALUE(n))
		return n;

	return n;
}

ulong blk_write_devnum(enum uclass_id uclass_id, int devnum, lbaint_t start,
		       lbaint_t blkcnt, const void *buffer)
{
	struct blk_driver *drv = blk_driver_lookup_type(uclass_id);
	struct blk_desc *desc;
	int ret;

	if (!drv)
		return -ENOSYS;
	ret = get_desc(drv, devnum, &desc);
	if (ret)
		return ret;
	return desc->block_write(desc, start, blkcnt, buffer);
}

int blk_select_hwpart_devnum(enum uclass_id uclass_id, int devnum, int hwpart)
{
	struct blk_driver *drv = blk_driver_lookup_type(uclass_id);
	struct blk_desc *desc;
	int ret;

	if (!drv)
		return -ENOSYS;
	ret = get_desc(drv, devnum, &desc);
	if (ret)
		return ret;
	return drv->select_hwpart(desc, hwpart);
}
