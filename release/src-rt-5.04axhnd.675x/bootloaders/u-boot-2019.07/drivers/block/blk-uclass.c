// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>

static const char *if_typename_str[IF_TYPE_COUNT] = {
	[IF_TYPE_IDE]		= "ide",
	[IF_TYPE_SCSI]		= "scsi",
	[IF_TYPE_ATAPI]		= "atapi",
	[IF_TYPE_USB]		= "usb",
	[IF_TYPE_DOC]		= "doc",
	[IF_TYPE_MMC]		= "mmc",
	[IF_TYPE_SD]		= "sd",
	[IF_TYPE_SATA]		= "sata",
	[IF_TYPE_HOST]		= "host",
	[IF_TYPE_NVME]		= "nvme",
	[IF_TYPE_EFI]		= "efi",
	[IF_TYPE_VIRTIO]	= "virtio",
};

static enum uclass_id if_type_uclass_id[IF_TYPE_COUNT] = {
	[IF_TYPE_IDE]		= UCLASS_IDE,
	[IF_TYPE_SCSI]		= UCLASS_SCSI,
	[IF_TYPE_ATAPI]		= UCLASS_INVALID,
	[IF_TYPE_USB]		= UCLASS_MASS_STORAGE,
	[IF_TYPE_DOC]		= UCLASS_INVALID,
	[IF_TYPE_MMC]		= UCLASS_MMC,
	[IF_TYPE_SD]		= UCLASS_INVALID,
	[IF_TYPE_SATA]		= UCLASS_AHCI,
	[IF_TYPE_HOST]		= UCLASS_ROOT,
	[IF_TYPE_NVME]		= UCLASS_NVME,
	[IF_TYPE_EFI]		= UCLASS_EFI,
	[IF_TYPE_VIRTIO]	= UCLASS_VIRTIO,
};

static enum if_type if_typename_to_iftype(const char *if_typename)
{
	int i;

	for (i = 0; i < IF_TYPE_COUNT; i++) {
		if (if_typename_str[i] &&
		    !strcmp(if_typename, if_typename_str[i]))
			return i;
	}

	return IF_TYPE_UNKNOWN;
}

static enum uclass_id if_type_to_uclass_id(enum if_type if_type)
{
	return if_type_uclass_id[if_type];
}

const char *blk_get_if_type_name(enum if_type if_type)
{
	return if_typename_str[if_type];
}

struct blk_desc *blk_get_devnum_by_type(enum if_type if_type, int devnum)
{
	struct blk_desc *desc;
	struct udevice *dev;
	int ret;

	ret = blk_get_device(if_type, devnum, &dev);
	if (ret)
		return NULL;
	desc = dev_get_uclass_platdata(dev);

	return desc;
}

/*
 * This function is complicated with driver model. We look up the interface
 * name in a local table. This gives us an interface type which we can match
 * against the uclass of the block device's parent.
 */
struct blk_desc *blk_get_devnum_by_typename(const char *if_typename, int devnum)
{
	enum uclass_id uclass_id;
	enum if_type if_type;
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	if_type = if_typename_to_iftype(if_typename);
	if (if_type == IF_TYPE_UNKNOWN) {
		debug("%s: Unknown interface type '%s'\n", __func__,
		      if_typename);
		return NULL;
	}
	uclass_id = if_type_to_uclass_id(if_type);
	if (uclass_id == UCLASS_INVALID) {
		debug("%s: Unknown uclass for interface type'\n",
		      if_typename_str[if_type]);
		return NULL;
	}

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return NULL;
	uclass_foreach_dev(dev, uc) {
		struct blk_desc *desc = dev_get_uclass_platdata(dev);

		debug("%s: if_type=%d, devnum=%d: %s, %d, %d\n", __func__,
		      if_type, devnum, dev->name, desc->if_type, desc->devnum);
		if (desc->devnum != devnum)
			continue;

		/* Find out the parent device uclass */
		if (device_get_uclass_id(dev->parent) != uclass_id) {
			debug("%s: parent uclass %d, this dev %d\n", __func__,
			      device_get_uclass_id(dev->parent), uclass_id);
			continue;
		}

		if (device_probe(dev))
			return NULL;

		debug("%s: Device desc %p\n", __func__, desc);
		return desc;
	}
	debug("%s: No device found\n", __func__);

	return NULL;
}

/**
 * blk_get_by_device() - Get the block device descriptor for the given device
 * @dev:	Instance of a storage device
 *
 * Return: With block device descriptor on success , NULL if there is no such
 *	   block device.
 */
struct blk_desc *blk_get_by_device(struct udevice *dev)
{
	struct udevice *child_dev, *next;

	device_foreach_child_safe(child_dev, next, dev) {
		if (device_get_uclass_id(child_dev) != UCLASS_BLK)
			continue;

		return dev_get_uclass_platdata(child_dev);
	}

	debug("%s: No block device found\n", __func__);

	return NULL;
}

/**
 * get_desc() - Get the block device descriptor for the given device number
 *
 * @if_type:	Interface type
 * @devnum:	Device number (0 = first)
 * @descp:	Returns block device descriptor on success
 * @return 0 on success, -ENODEV if there is no such device and no device
 * with a higher device number, -ENOENT if there is no such device but there
 * is one with a higher number, or other -ve on other error.
 */
static int get_desc(enum if_type if_type, int devnum, struct blk_desc **descp)
{
	bool found_more = false;
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	*descp = NULL;
	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		struct blk_desc *desc = dev_get_uclass_platdata(dev);

		debug("%s: if_type=%d, devnum=%d: %s, %d, %d\n", __func__,
		      if_type, devnum, dev->name, desc->if_type, desc->devnum);
		if (desc->if_type == if_type) {
			if (desc->devnum == devnum) {
				ret = device_probe(dev);
				if (ret)
					return ret;

				*descp = desc;
				return 0;
			} else if (desc->devnum > devnum) {
				found_more = true;
			}
		}
	}

	return found_more ? -ENOENT : -ENODEV;
}

int blk_select_hwpart_devnum(enum if_type if_type, int devnum, int hwpart)
{
	struct udevice *dev;
	int ret;

	ret = blk_get_device(if_type, devnum, &dev);
	if (ret)
		return ret;

	return blk_select_hwpart(dev, hwpart);
}

int blk_list_part(enum if_type if_type)
{
	struct blk_desc *desc;
	int devnum, ok;
	int ret;

	for (ok = 0, devnum = 0;; ++devnum) {
		ret = get_desc(if_type, devnum, &desc);
		if (ret == -ENODEV)
			break;
		else if (ret)
			continue;
		if (desc->part_type != PART_TYPE_UNKNOWN) {
			++ok;
			if (devnum)
				putc('\n');
			part_print(desc);
		}
	}
	if (!ok)
		return -ENODEV;

	return 0;
}

int blk_print_part_devnum(enum if_type if_type, int devnum)
{
	struct blk_desc *desc;
	int ret;

	ret = get_desc(if_type, devnum, &desc);
	if (ret)
		return ret;
	if (desc->type == DEV_TYPE_UNKNOWN)
		return -ENOENT;
	part_print(desc);

	return 0;
}

void blk_list_devices(enum if_type if_type)
{
	struct blk_desc *desc;
	int ret;
	int i;

	for (i = 0;; ++i) {
		ret = get_desc(if_type, i, &desc);
		if (ret == -ENODEV)
			break;
		else if (ret)
			continue;
		if (desc->type == DEV_TYPE_UNKNOWN)
			continue;  /* list only known devices */
		printf("Device %d: ", i);
		dev_print(desc);
	}
}

int blk_print_device_num(enum if_type if_type, int devnum)
{
	struct blk_desc *desc;
	int ret;

	ret = get_desc(if_type, devnum, &desc);
	if (ret)
		return ret;
	printf("\nIDE device %d: ", devnum);
	dev_print(desc);

	return 0;
}

int blk_show_device(enum if_type if_type, int devnum)
{
	struct blk_desc *desc;
	int ret;

	printf("\nDevice %d: ", devnum);
	ret = get_desc(if_type, devnum, &desc);
	if (ret == -ENODEV || ret == -ENOENT) {
		printf("unknown device\n");
		return -ENODEV;
	}
	if (ret)
		return ret;
	dev_print(desc);

	if (desc->type == DEV_TYPE_UNKNOWN)
		return -ENOENT;

	return 0;
}

ulong blk_read_devnum(enum if_type if_type, int devnum, lbaint_t start,
		      lbaint_t blkcnt, void *buffer)
{
	struct blk_desc *desc;
	ulong n;
	int ret;

	ret = get_desc(if_type, devnum, &desc);
	if (ret)
		return ret;
	n = blk_dread(desc, start, blkcnt, buffer);
	if (IS_ERR_VALUE(n))
		return n;

	return n;
}

ulong blk_write_devnum(enum if_type if_type, int devnum, lbaint_t start,
		       lbaint_t blkcnt, const void *buffer)
{
	struct blk_desc *desc;
	int ret;

	ret = get_desc(if_type, devnum, &desc);
	if (ret)
		return ret;
	return blk_dwrite(desc, start, blkcnt, buffer);
}

int blk_select_hwpart(struct udevice *dev, int hwpart)
{
	const struct blk_ops *ops = blk_get_ops(dev);

	if (!ops)
		return -ENOSYS;
	if (!ops->select_hwpart)
		return 0;

	return ops->select_hwpart(dev, hwpart);
}

int blk_dselect_hwpart(struct blk_desc *desc, int hwpart)
{
	return blk_select_hwpart(desc->bdev, hwpart);
}

int blk_first_device(int if_type, struct udevice **devp)
{
	struct blk_desc *desc;
	int ret;

	ret = uclass_find_first_device(UCLASS_BLK, devp);
	if (ret)
		return ret;
	if (!*devp)
		return -ENODEV;
	do {
		desc = dev_get_uclass_platdata(*devp);
		if (desc->if_type == if_type)
			return 0;
		ret = uclass_find_next_device(devp);
		if (ret)
			return ret;
	} while (*devp);

	return -ENODEV;
}

int blk_next_device(struct udevice **devp)
{
	struct blk_desc *desc;
	int ret, if_type;

	desc = dev_get_uclass_platdata(*devp);
	if_type = desc->if_type;
	do {
		ret = uclass_find_next_device(devp);
		if (ret)
			return ret;
		if (!*devp)
			return -ENODEV;
		desc = dev_get_uclass_platdata(*devp);
		if (desc->if_type == if_type)
			return 0;
	} while (1);
}

int blk_find_device(int if_type, int devnum, struct udevice **devp)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		struct blk_desc *desc = dev_get_uclass_platdata(dev);

		debug("%s: if_type=%d, devnum=%d: %s, %d, %d\n", __func__,
		      if_type, devnum, dev->name, desc->if_type, desc->devnum);
		if (desc->if_type == if_type && desc->devnum == devnum) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int blk_get_device(int if_type, int devnum, struct udevice **devp)
{
	int ret;

	ret = blk_find_device(if_type, devnum, devp);
	if (ret)
		return ret;

	return device_probe(*devp);
}

unsigned long blk_dread(struct blk_desc *block_dev, lbaint_t start,
			lbaint_t blkcnt, void *buffer)
{
	struct udevice *dev = block_dev->bdev;
	const struct blk_ops *ops = blk_get_ops(dev);
	ulong blks_read;

	if (!ops->read)
		return -ENOSYS;

	if (blkcache_read(block_dev->if_type, block_dev->devnum,
			  start, blkcnt, block_dev->blksz, buffer))
		return blkcnt;
	blks_read = ops->read(dev, start, blkcnt, buffer);
	if (blks_read == blkcnt)
		blkcache_fill(block_dev->if_type, block_dev->devnum,
			      start, blkcnt, block_dev->blksz, buffer);

	return blks_read;
}

unsigned long blk_dwrite(struct blk_desc *block_dev, lbaint_t start,
			 lbaint_t blkcnt, const void *buffer)
{
	struct udevice *dev = block_dev->bdev;
	const struct blk_ops *ops = blk_get_ops(dev);

	if (!ops->write)
		return -ENOSYS;

	blkcache_invalidate(block_dev->if_type, block_dev->devnum);
	return ops->write(dev, start, blkcnt, buffer);
}

unsigned long blk_derase(struct blk_desc *block_dev, lbaint_t start,
			 lbaint_t blkcnt)
{
	struct udevice *dev = block_dev->bdev;
	const struct blk_ops *ops = blk_get_ops(dev);

	if (!ops->erase)
		return -ENOSYS;

	blkcache_invalidate(block_dev->if_type, block_dev->devnum);
	return ops->erase(dev, start, blkcnt);
}

int blk_get_from_parent(struct udevice *parent, struct udevice **devp)
{
	struct udevice *dev;
	enum uclass_id id;
	int ret;

	device_find_first_child(parent, &dev);
	if (!dev) {
		debug("%s: No block device found for parent '%s'\n", __func__,
		      parent->name);
		return -ENODEV;
	}
	id = device_get_uclass_id(dev);
	if (id != UCLASS_BLK) {
		debug("%s: Incorrect uclass %s for block device '%s'\n",
		      __func__, uclass_get_name(id), dev->name);
		return -ENOTBLK;
	}
	ret = device_probe(dev);
	if (ret)
		return ret;
	*devp = dev;

	return 0;
}

int blk_find_max_devnum(enum if_type if_type)
{
	struct udevice *dev;
	int max_devnum = -ENODEV;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		struct blk_desc *desc = dev_get_uclass_platdata(dev);

		if (desc->if_type == if_type && desc->devnum > max_devnum)
			max_devnum = desc->devnum;
	}

	return max_devnum;
}

int blk_next_free_devnum(enum if_type if_type)
{
	int ret;

	ret = blk_find_max_devnum(if_type);
	if (ret == -ENODEV)
		return 0;
	if (ret < 0)
		return ret;

	return ret + 1;
}

static int blk_claim_devnum(enum if_type if_type, int devnum)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		struct blk_desc *desc = dev_get_uclass_platdata(dev);

		if (desc->if_type == if_type && desc->devnum == devnum) {
			int next = blk_next_free_devnum(if_type);

			if (next < 0)
				return next;
			desc->devnum = next;
			return 0;
		}
	}

	return -ENOENT;
}

int blk_create_device(struct udevice *parent, const char *drv_name,
		      const char *name, int if_type, int devnum, int blksz,
		      lbaint_t lba, struct udevice **devp)
{
	struct blk_desc *desc;
	struct udevice *dev;
	int ret;

	if (devnum == -1) {
		devnum = blk_next_free_devnum(if_type);
	} else {
		ret = blk_claim_devnum(if_type, devnum);
		if (ret < 0 && ret != -ENOENT)
			return ret;
	}
	if (devnum < 0)
		return devnum;
	ret = device_bind_driver(parent, drv_name, name, &dev);
	if (ret)
		return ret;
	desc = dev_get_uclass_platdata(dev);
	desc->if_type = if_type;
	desc->blksz = blksz;
	desc->lba = lba;
	desc->part_type = PART_TYPE_UNKNOWN;
	desc->bdev = dev;
	desc->devnum = devnum;
	*devp = dev;

	return 0;
}

int blk_create_devicef(struct udevice *parent, const char *drv_name,
		       const char *name, int if_type, int devnum, int blksz,
		       lbaint_t lba, struct udevice **devp)
{
	char dev_name[30], *str;
	int ret;

	snprintf(dev_name, sizeof(dev_name), "%s.%s", parent->name, name);
	str = strdup(dev_name);
	if (!str)
		return -ENOMEM;

	ret = blk_create_device(parent, drv_name, str, if_type, devnum,
				blksz, lba, devp);
	if (ret) {
		free(str);
		return ret;
	}
	device_set_name_alloced(*devp);

	return 0;
}

int blk_unbind_all(int if_type)
{
	struct uclass *uc;
	struct udevice *dev, *next;
	int ret;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev_safe(dev, next, uc) {
		struct blk_desc *desc = dev_get_uclass_platdata(dev);

		if (desc->if_type == if_type) {
			ret = device_remove(dev, DM_REMOVE_NORMAL);
			if (ret)
				return ret;
			ret = device_unbind(dev);
			if (ret)
				return ret;
		}
	}

	return 0;
}

static int blk_post_probe(struct udevice *dev)
{
#if defined(CONFIG_PARTITIONS) && defined(CONFIG_HAVE_BLOCK_DEVICE)
	struct blk_desc *desc = dev_get_uclass_platdata(dev);

	part_init(desc);
#endif

	return 0;
}

UCLASS_DRIVER(blk) = {
	.id		= UCLASS_BLK,
	.name		= "blk",
	.post_probe	= blk_post_probe,
	.per_device_platdata_auto_alloc_size = sizeof(struct blk_desc),
};
