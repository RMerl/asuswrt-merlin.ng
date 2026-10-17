// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BLK

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <part.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <linux/err.h>

#define blk_get_ops(dev)	((struct blk_ops *)(dev)->driver->ops)

static struct {
	enum uclass_id id;
	const char *name;
} uclass_idname_str[] = {
	{ UCLASS_IDE, "ide" },
	{ UCLASS_SCSI, "scsi" },
	{ UCLASS_USB, "usb" },
	{ UCLASS_MMC,  "mmc" },
	{ UCLASS_AHCI, "sata" },
	{ UCLASS_HOST, "host" },
	{ UCLASS_NVME, "nvme" },
	{ UCLASS_NVMXIP, "nvmxip" },
	{ UCLASS_EFI_MEDIA, "efi" },
	{ UCLASS_EFI_LOADER, "efiloader" },
	{ UCLASS_VIRTIO, "virtio" },
	{ UCLASS_PVBLOCK, "pvblock" },
	{ UCLASS_BLKMAP, "blkmap" },
	{ UCLASS_RKMTD, "rkmtd" },
#if defined(CONFIG_BCMBCA_VFBIO)
	{ UCLASS_VFBIO, "vfbio"}
#endif
};

static enum uclass_id uclass_name_to_iftype(const char *uclass_idname)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(uclass_idname_str); i++) {
		if (!strcmp(uclass_idname, uclass_idname_str[i].name))
			return uclass_idname_str[i].id;
	}

	return UCLASS_INVALID;
}

static enum uclass_id conv_uclass_id(enum uclass_id uclass_id)
{
	/*
	 * This strange adjustment is used because we use UCLASS_MASS_STORAGE
	 * for USB storage devices, so need to return this as the uclass to
	 * use for USB. In fact USB_UCLASS is for USB controllers, not
	 * peripherals.
	 *
	 * The name of the UCLASS_MASS_STORAGE uclass driver is
	 * "usb_mass_storage", but we want to use "usb" in things like the
	 * 'part list' command and when showing interfaces.
	 *
	 * So for now we have this one-way conversion.
	 *
	 * The fix for this is possibly to:
	 *    - rename UCLASS_MASS_STORAGE name to "usb"
	 *    - rename UCLASS_USB name to "usb_ctlr"
	 *    - use UCLASS_MASS_STORAGE instead of UCLASS_USB in if_typename_str
	 */
	if (uclass_id == UCLASS_USB)
		return UCLASS_MASS_STORAGE;
	return uclass_id;
}

const char *blk_get_uclass_name(enum uclass_id uclass_id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(uclass_idname_str); i++) {
		if (uclass_idname_str[i].id == uclass_id)
			return uclass_idname_str[i].name;
	}

	return "(none)";
}

struct blk_desc *blk_get_devnum_by_uclass_id(enum uclass_id uclass_id, int devnum)
{
	struct blk_desc *desc;
	struct udevice *dev;
	int ret;

	ret = blk_get_device(uclass_id, devnum, &dev);
	if (ret)
		return NULL;
	desc = dev_get_uclass_plat(dev);

	return desc;
}

/*
 * This function is complicated with driver model. We look up the interface
 * name in a local table. This gives us an interface type which we can match
 * against the uclass of the block device's parent.
 */
struct blk_desc *blk_get_devnum_by_uclass_idname(const char *uclass_idname, int devnum)
{
	enum uclass_id uclass_id;
	enum uclass_id type;
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	type = uclass_name_to_iftype(uclass_idname);
	if (type == UCLASS_INVALID) {
		debug("%s: Unknown interface type '%s'\n", __func__,
		      uclass_idname);
		return NULL;
	}
	uclass_id = conv_uclass_id(type);
	if (uclass_id == UCLASS_INVALID) {
		debug("%s: Unknown uclass for interface type'\n",
		      blk_get_uclass_name(type));
		return NULL;
	}

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return NULL;
	uclass_foreach_dev(dev, uc) {
		struct blk_desc *desc = dev_get_uclass_plat(dev);

		debug("%s: uclass_id=%d, devnum=%d: %s, %d, %d\n", __func__,
		      type, devnum, dev->name, desc->uclass_id, desc->devnum);
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
	struct udevice *child_dev;

	device_foreach_child(child_dev, dev) {
		if (device_get_uclass_id(child_dev) != UCLASS_BLK)
			continue;

		return dev_get_uclass_plat(child_dev);
	}

	debug("%s: No block device found\n", __func__);

	return NULL;
}

int blk_get_desc(enum uclass_id uclass_id, int devnum, struct blk_desc **descp)
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
		struct blk_desc *desc = dev_get_uclass_plat(dev);

		debug("%s: uclass_id=%d, devnum=%d: %s, %d, %d\n", __func__,
		      uclass_id, devnum, dev->name, desc->uclass_id, desc->devnum);
		if (desc->uclass_id == uclass_id) {
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

int blk_select_hwpart_devnum(enum uclass_id uclass_id, int devnum, int hwpart)
{
	struct udevice *dev;
	int ret;

	ret = blk_get_device(uclass_id, devnum, &dev);
	if (ret)
		return ret;

	return blk_select_hwpart(dev, hwpart);
}

int blk_list_part(enum uclass_id uclass_id)
{
	struct blk_desc *desc;
	int devnum, ok;
	int ret;

	for (ok = 0, devnum = 0;; ++devnum) {
		ret = blk_get_desc(uclass_id, devnum, &desc);
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

int blk_print_part_devnum(enum uclass_id uclass_id, int devnum)
{
	struct blk_desc *desc;
	int ret;

	ret = blk_get_desc(uclass_id, devnum, &desc);
	if (ret)
		return ret;
	if (desc->type == DEV_TYPE_UNKNOWN)
		return -ENOENT;
	part_print(desc);

	return 0;
}

void blk_list_devices(enum uclass_id uclass_id)
{
	struct blk_desc *desc;
	int ret;
	int i;

	for (i = 0;; ++i) {
		ret = blk_get_desc(uclass_id, i, &desc);
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

int blk_print_device_num(enum uclass_id uclass_id, int devnum)
{
	struct blk_desc *desc;
	int ret;

	ret = blk_get_desc(uclass_id, devnum, &desc);
	if (ret)
		return ret;
	printf("\nIDE device %d: ", devnum);
	dev_print(desc);

	return 0;
}

int blk_show_device(enum uclass_id uclass_id, int devnum)
{
	struct blk_desc *desc;
	int ret;

	printf("\nDevice %d: ", devnum);
	ret = blk_get_desc(uclass_id, devnum, &desc);
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

static int _blk_next_device(int uclass_id, struct udevice **devp)
{
	struct blk_desc *desc;
	int ret = 0;

	for (; *devp; uclass_find_next_device(devp)) {
		desc = dev_get_uclass_plat(*devp);
		if (desc->uclass_id == uclass_id) {
			ret = device_probe(*devp);
			if (!ret)
				return 0;
		}
	}

	if (ret)
		return ret;

	return -ENODEV;
}

int blk_first_device(int uclass_id, struct udevice **devp)
{
	uclass_find_first_device(UCLASS_BLK, devp);

	return _blk_next_device(uclass_id, devp);
}

int blk_next_device(struct udevice **devp)
{
	struct blk_desc *desc;
	int uclass_id;

	desc = dev_get_uclass_plat(*devp);
	uclass_id = desc->uclass_id;
	uclass_find_next_device(devp);

	return _blk_next_device(uclass_id, devp);
}

int blk_find_device(int uclass_id, int devnum, struct udevice **devp)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		struct blk_desc *desc = dev_get_uclass_plat(dev);

		debug("%s: uclass_id=%d, devnum=%d: %s, %d, %d\n", __func__,
		      uclass_id, devnum, dev->name, desc->uclass_id, desc->devnum);
		if (desc->uclass_id == uclass_id && desc->devnum == devnum) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int blk_get_device(int uclass_id, int devnum, struct udevice **devp)
{
	int ret;

	ret = blk_find_device(uclass_id, devnum, devp);
	if (ret)
		return ret;

	return device_probe(*devp);
}

struct blk_bounce_buffer {
	struct udevice		*dev;
	struct bounce_buffer	state;
};

static int blk_buffer_aligned(struct bounce_buffer *state)
{
#if IS_ENABLED(CONFIG_BOUNCE_BUFFER)
	struct blk_bounce_buffer *bbstate =
		container_of(state, struct blk_bounce_buffer, state);
	struct udevice *dev = bbstate->dev;
	const struct blk_ops *ops = blk_get_ops(dev);

	if (ops->buffer_aligned)
		return ops->buffer_aligned(dev, state);
#endif	/* CONFIG_BOUNCE_BUFFER */

	return 1;	/* Default, any buffer is OK */
}

long blk_read(struct udevice *dev, lbaint_t start, lbaint_t blkcnt, void *buf)
{
	struct blk_desc *desc = dev_get_uclass_plat(dev);
	const struct blk_ops *ops = blk_get_ops(dev);
	ulong blks_read;

	if (!ops->read)
		return -ENOSYS;

	if (blkcache_read(desc->uclass_id, desc->devnum,
			  start, blkcnt, desc->blksz, buf))
		return blkcnt;

	if (IS_ENABLED(CONFIG_BOUNCE_BUFFER) && desc->bb) {
		struct blk_bounce_buffer bbstate = { .dev = dev };
		int ret;

		ret = bounce_buffer_start_extalign(&bbstate.state, buf,
						   blkcnt * desc->blksz,
						   GEN_BB_WRITE, desc->blksz,
						   blk_buffer_aligned);
		if (ret)
			return ret;

		blks_read = ops->read(dev, start, blkcnt, bbstate.state.bounce_buffer);

		bounce_buffer_stop(&bbstate.state);
	} else {
		blks_read = ops->read(dev, start, blkcnt, buf);
	}

	if (blks_read == blkcnt)
		blkcache_fill(desc->uclass_id, desc->devnum, start, blkcnt,
			      desc->blksz, buf);

	return blks_read;
}

long blk_write(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
	       const void *buf)
{
	struct blk_desc *desc = dev_get_uclass_plat(dev);
	const struct blk_ops *ops = blk_get_ops(dev);
	long blks_written;

	if (!ops->write)
		return -ENOSYS;

	blkcache_invalidate(desc->uclass_id, desc->devnum);

	if (IS_ENABLED(CONFIG_BOUNCE_BUFFER) && desc->bb) {
		struct blk_bounce_buffer bbstate = { .dev = dev };
		int ret;

		ret = bounce_buffer_start_extalign(&bbstate.state, (void *)buf,
						   blkcnt * desc->blksz,
						   GEN_BB_READ, desc->blksz,
						   blk_buffer_aligned);
		if (ret)
			return ret;

		blks_written = ops->write(dev, start, blkcnt,
					  bbstate.state.bounce_buffer);

		bounce_buffer_stop(&bbstate.state);
	} else {
		blks_written = ops->write(dev, start, blkcnt, buf);
	}

	return blks_written;
}

long blk_erase(struct udevice *dev, lbaint_t start, lbaint_t blkcnt)
{
	struct blk_desc *desc = dev_get_uclass_plat(dev);
	const struct blk_ops *ops = blk_get_ops(dev);

	if (!ops->erase)
		return -ENOSYS;

	blkcache_invalidate(desc->uclass_id, desc->devnum);

	return ops->erase(dev, start, blkcnt);
}

ulong blk_dread(struct blk_desc *desc, lbaint_t start, lbaint_t blkcnt,
		void *buffer)
{
	return blk_read(desc->bdev, start, blkcnt, buffer);
}

ulong blk_dwrite(struct blk_desc *desc, lbaint_t start, lbaint_t blkcnt,
		 const void *buffer)
{
	return blk_write(desc->bdev, start, blkcnt, buffer);
}

ulong blk_derase(struct blk_desc *desc, lbaint_t start, lbaint_t blkcnt)
{
	return blk_erase(desc->bdev, start, blkcnt);
}

int blk_find_from_parent(struct udevice *parent, struct udevice **devp)
{
	struct udevice *dev;

	if (device_find_first_child_by_uclass(parent, UCLASS_BLK, &dev)) {
		debug("%s: No block device found for parent '%s'\n", __func__,
		      parent->name);
		return -ENODEV;
	}
	*devp = dev;

	return 0;
}

int blk_get_from_parent(struct udevice *parent, struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	ret = blk_find_from_parent(parent, &dev);
	if (ret)
		return ret;
	ret = device_probe(dev);
	if (ret)
		return ret;
	*devp = dev;

	return 0;
}

const char *blk_get_devtype(struct udevice *dev)
{
	struct udevice *parent = dev_get_parent(dev);

	return uclass_get_name(device_get_uclass_id(parent));
};

int blk_find_max_devnum(enum uclass_id uclass_id)
{
	struct udevice *dev;
	int max_devnum = -ENODEV;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		struct blk_desc *desc = dev_get_uclass_plat(dev);

		if (desc->uclass_id == uclass_id && desc->devnum > max_devnum)
			max_devnum = desc->devnum;
	}

	return max_devnum;
}

int blk_next_free_devnum(enum uclass_id uclass_id)
{
	int ret;

	ret = blk_find_max_devnum(uclass_id);
	if (ret == -ENODEV)
		return 0;
	if (ret < 0)
		return ret;

	return ret + 1;
}

static int blk_flags_check(struct udevice *dev, enum blk_flag_t req_flags)
{
	const struct blk_desc *desc = dev_get_uclass_plat(dev);
	enum blk_flag_t flags;

	flags = desc->removable ? BLKF_REMOVABLE : BLKF_FIXED;

	return flags & req_flags ? 0 : 1;
}

int blk_find_first(enum blk_flag_t flags, struct udevice **devp)
{
	int ret;

	for (ret = uclass_find_first_device(UCLASS_BLK, devp);
	     *devp && !blk_flags_check(*devp, flags);
	     ret = uclass_find_next_device(devp))
		return 0;

	return -ENODEV;
}

int blk_find_next(enum blk_flag_t flags, struct udevice **devp)
{
	int ret;

	for (ret = uclass_find_next_device(devp);
	     *devp && !blk_flags_check(*devp, flags);
	     ret = uclass_find_next_device(devp))
		return 0;

	return -ENODEV;
}

int blk_first_device_err(enum blk_flag_t flags, struct udevice **devp)
{
	for (uclass_first_device(UCLASS_BLK, devp);
	     *devp;
	     uclass_next_device(devp)) {
		if (!blk_flags_check(*devp, flags))
			return 0;
	}

	return -ENODEV;
}

int blk_next_device_err(enum blk_flag_t flags, struct udevice **devp)
{
	for (uclass_next_device(devp);
	     *devp;
	     uclass_next_device(devp)) {
		if (!blk_flags_check(*devp, flags))
			return 0;
	}

	return -ENODEV;
}

int blk_count_devices(enum blk_flag_t flag)
{
	struct udevice *dev;
	int count = 0;

	blk_foreach_probe(flag, dev)
		count++;

	return count;
}

static int blk_claim_devnum(enum uclass_id uclass_id, int devnum)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		struct blk_desc *desc = dev_get_uclass_plat(dev);

		if (desc->uclass_id == uclass_id && desc->devnum == devnum) {
			int next = blk_next_free_devnum(uclass_id);

			if (next < 0)
				return next;
			desc->devnum = next;
			return 0;
		}
	}

	return -ENOENT;
}

int blk_create_device(struct udevice *parent, const char *drv_name,
		      const char *name, int uclass_id, int devnum, int blksz,
		      lbaint_t lba, struct udevice **devp)
{
	struct blk_desc *desc;
	struct udevice *dev;
	int ret;

	if (devnum == -1) {
		devnum = blk_next_free_devnum(uclass_id);
	} else {
		ret = blk_claim_devnum(uclass_id, devnum);
		if (ret < 0 && ret != -ENOENT)
			return ret;
	}
	if (devnum < 0)
		return devnum;
	ret = device_bind_driver(parent, drv_name, name, &dev);
	if (ret)
		return ret;
	desc = dev_get_uclass_plat(dev);
	desc->uclass_id = uclass_id;
	desc->blksz = blksz;
	desc->log2blksz = LOG2(desc->blksz);
	desc->lba = lba;
	desc->part_type = PART_TYPE_UNKNOWN;
	desc->bdev = dev;
	desc->devnum = devnum;
	*devp = dev;

	return 0;
}

int blk_create_devicef(struct udevice *parent, const char *drv_name,
		       const char *name, int uclass_id, int devnum, int blksz,
		       lbaint_t lba, struct udevice **devp)
{
	char dev_name[30], *str;
	int ret;

	snprintf(dev_name, sizeof(dev_name), "%s.%s", parent->name, name);
	str = strdup(dev_name);
	if (!str)
		return -ENOMEM;

	ret = blk_create_device(parent, drv_name, str, uclass_id, devnum,
				blksz, lba, devp);
	if (ret) {
		free(str);
		return ret;
	}
	device_set_name_alloced(*devp);

	return 0;
}

int blk_probe_or_unbind(struct udevice *dev)
{
	int ret;

	ret = device_probe(dev);
	if (ret) {
		log_debug("probing %s failed\n", dev->name);
		device_unbind(dev);
	}

	return ret;
}

int blk_unbind_all(int uclass_id)
{
	struct uclass *uc;
	struct udevice *dev, *next;
	int ret;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev_safe(dev, next, uc) {
		struct blk_desc *desc = dev_get_uclass_plat(dev);

		if (desc->uclass_id == uclass_id) {
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

static int part_create_block_devices(struct udevice *blk_dev)
{
	int part, count;
	struct blk_desc *desc = dev_get_uclass_plat(blk_dev);
	struct disk_partition info;
	struct disk_part *part_data;
	char devname[32];
	struct udevice *dev;
	int ret;

	if (!CONFIG_IS_ENABLED(PARTITIONS) || !blk_enabled())
		return 0;

	if (device_get_uclass_id(blk_dev) != UCLASS_BLK)
		return 0;

	/* Add devices for each partition */
	for (count = 0, part = 1; part <= MAX_SEARCH_PARTITIONS; part++) {
		if (part_get_info(desc, part, &info))
			continue;
		snprintf(devname, sizeof(devname), "%s:%d", blk_dev->name,
			 part);

		ret = device_bind_driver(blk_dev, "blk_partition",
					 strdup(devname), &dev);
		if (ret)
			return ret;

		part_data = dev_get_uclass_plat(dev);
		part_data->partnum = part;
		part_data->gpt_part_info = info;
		count++;

		ret = device_probe(dev);
		if (ret) {
			debug("Can't probe\n");
			count--;
			device_unbind(dev);

			continue;
		}
	}
	debug("%s: %d partitions found in %s\n", __func__, count,
	      blk_dev->name);

	return 0;
}

static int blk_post_probe(struct udevice *dev)
{
	if (CONFIG_IS_ENABLED(PARTITIONS) && blk_enabled()) {
		struct blk_desc *desc = dev_get_uclass_plat(dev);

		part_init(desc);

		if (desc->part_type != PART_TYPE_UNKNOWN &&
		    part_create_block_devices(dev))
			debug("*** creating partitions failed\n");
	}

	return 0;
}

UCLASS_DRIVER(blk) = {
	.id		= UCLASS_BLK,
	.name		= "blk",
	.post_probe	= blk_post_probe,
	.per_device_plat_auto	= sizeof(struct blk_desc),
};
