// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <blk.h>
#include <command.h>
#include <env.h>
#include <errno.h>
#include <ide.h>
#include <log.h>
#include <malloc.h>
#include <part.h>
#include <ubifs_uboot.h>

#undef	PART_DEBUG

#ifdef	PART_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

/* Check all partition types */
#define PART_TYPE_ALL		-1

/**
 * part_driver_get_type() - Get a driver given its type
 *
 * @part_type: Partition type to find the driver for
 * Return: Driver for that type, or NULL if none
 */
static struct part_driver *part_driver_get_type(int part_type)
{
	struct part_driver *drv =
		ll_entry_start(struct part_driver, part_driver);
	const int n_ents = ll_entry_count(struct part_driver, part_driver);
	struct part_driver *entry;

	for (entry = drv; entry != drv + n_ents; entry++) {
		if (part_type == entry->part_type)
			return entry;
	}

	/* Not found */
	return NULL;
}

/**
 * part_driver_lookup_type() - Look up the partition driver for a blk device
 *
 * If @desc->part_type is PART_TYPE_UNKNOWN, this checks each parition driver
 * against the blk device to see if there is a valid partition table acceptable
 * to that driver.
 *
 * If @desc->part_type is already set, it just returns the driver for that
 * type, without testing if the driver can find a valid partition on the
 * descriptor.
 *
 * On success it updates @desc->part_type if set to PART_TYPE_UNKNOWN on entry
 *
 * @dev_desc: Device descriptor
 * Return: Driver found, or NULL if none
 */
static struct part_driver *part_driver_lookup_type(struct blk_desc *desc)
{
	struct part_driver *drv =
		ll_entry_start(struct part_driver, part_driver);
	const int n_ents = ll_entry_count(struct part_driver, part_driver);
	struct part_driver *entry;

	if (desc->part_type == PART_TYPE_UNKNOWN) {
		for (entry = drv; entry != drv + n_ents; entry++) {
			int ret;

			ret = entry->test(desc);
			if (!ret) {
				desc->part_type = entry->part_type;
				return entry;
			}
		}
	} else {
		return part_driver_get_type(desc->part_type);
	}

	/* Not found */
	return NULL;
}

int part_get_type_by_name(const char *name)
{
	struct part_driver *drv =
		ll_entry_start(struct part_driver, part_driver);
	const int n_ents = ll_entry_count(struct part_driver, part_driver);
	struct part_driver *entry;

	for (entry = drv; entry != drv + n_ents; entry++) {
		if (!strcasecmp(name, entry->name))
			return entry->part_type;
	}

	/* Not found */
	return PART_TYPE_UNKNOWN;
}

/**
 * get_dev_hwpart() - Get the descriptor for a device with hardware partitions
 *
 * @ifname:	Interface name (e.g. "ide", "scsi")
 * @dev:	Device number (0 for first device on that interface, 1 for
 *		second, etc.
 * @hwpart: Hardware partition, or 0 if none (used for MMC)
 * Return: pointer to the block device, or NULL if not available, or an
 *	   error occurred.
 */
static struct blk_desc *get_dev_hwpart(const char *ifname, int dev, int hwpart)
{
	struct blk_desc *desc;
	int ret;

	if (!blk_enabled())
		return NULL;
	desc = blk_get_devnum_by_uclass_idname(ifname, dev);
	if (!desc) {
		debug("%s: No device for iface '%s', dev %d\n", __func__,
		      ifname, dev);
		return NULL;
	}
	ret = blk_dselect_hwpart(desc, hwpart);
	if (ret) {
		debug("%s: Failed to select h/w partition: err-%d\n", __func__,
		      ret);
		return NULL;
	}

	return desc;
}

struct blk_desc *blk_get_dev(const char *ifname, int dev)
{
	if (!blk_enabled())
		return NULL;

	return get_dev_hwpart(ifname, dev, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * reports device info to the user
 */

#ifdef CONFIG_LBA48
typedef uint64_t lba512_t;
#else
typedef lbaint_t lba512_t;
#endif

/*
 * Overflowless variant of (block_count * mul_by / 2**right_shift)
 * when 2**right_shift > mul_by
 */
static lba512_t lba512_muldiv(lba512_t block_count, lba512_t mul_by,
			      int right_shift)
{
	lba512_t bc_quot, bc_rem;

	/* x * m / d == x / d * m + (x % d) * m / d */
	bc_quot = block_count >> right_shift;
	bc_rem  = block_count - (bc_quot << right_shift);
	return bc_quot * mul_by + ((bc_rem * mul_by) >> right_shift);
}

void dev_print(struct blk_desc *desc)
{
	lba512_t lba512; /* number of blocks if 512bytes block size */

	if (desc->type == DEV_TYPE_UNKNOWN) {
		puts ("not available\n");
		return;
	}

	switch (desc->uclass_id) {
	case UCLASS_SCSI:
		printf("(%d:%d) Vendor: %s Prod.: %s Rev: %s\n", desc->target,
		       desc->lun, desc->vendor, desc->product, desc->revision);
		break;
	case UCLASS_IDE:
	case UCLASS_AHCI:
		printf("Model: %s Firm: %s Ser#: %s\n", desc->vendor,
		       desc->revision, desc->product);
		break;
	case UCLASS_MMC:
	case UCLASS_USB:
	case UCLASS_NVME:
	case UCLASS_PVBLOCK:
	case UCLASS_HOST:
	case UCLASS_BLKMAP:
	case UCLASS_RKMTD:
		printf ("Vendor: %s Rev: %s Prod: %s\n",
			desc->vendor,
			desc->revision,
			desc->product);
		break;
	case UCLASS_VIRTIO:
		printf("%s VirtIO Block Device\n", desc->vendor);
		break;
	case UCLASS_EFI_MEDIA:
		printf("EFI media Block Device %d\n", desc->devnum);
		break;
	case UCLASS_INVALID:
		puts("device type unknown\n");
		return;
	default:
		printf("Unhandled device type: %i\n", desc->uclass_id);
		return;
	}
	puts ("            Type: ");
	if (desc->removable)
		puts ("Removable ");
	switch (desc->type & 0x1F) {
	case DEV_TYPE_HARDDISK:
		puts ("Hard Disk");
		break;
	case DEV_TYPE_CDROM:
		puts ("CD ROM");
		break;
	case DEV_TYPE_OPDISK:
		puts ("Optical Device");
		break;
	case DEV_TYPE_TAPE:
		puts ("Tape");
		break;
	default:
		printf("# %02X #", desc->type & 0x1F);
		break;
	}
	puts ("\n");
	if (desc->lba > 0L && desc->blksz > 0L) {
		ulong mb, mb_quot, mb_rem, gb, gb_quot, gb_rem;
		lbaint_t lba;

		lba = desc->lba;

		lba512 = lba * (desc->blksz / 512);
		/* round to 1 digit */
		/* 2048 = (1024 * 1024) / 512 MB */
		mb = lba512_muldiv(lba512, 10, 11);

		mb_quot	= mb / 10;
		mb_rem	= mb - (10 * mb_quot);

		gb = mb / 1024;
		gb_quot	= gb / 10;
		gb_rem	= gb - (10 * gb_quot);
#ifdef CONFIG_LBA48
		if (desc->lba48)
			printf ("            Supports 48-bit addressing\n");
#endif
#if defined(CONFIG_SYS_64BIT_LBA)
		printf ("            Capacity: %lu.%lu MB = %lu.%lu GB (%llu x %lu)\n",
			mb_quot, mb_rem,
			gb_quot, gb_rem,
			lba,
			desc->blksz);
#else
		printf ("            Capacity: %lu.%lu MB = %lu.%lu GB (%lu x %lu)\n",
			mb_quot, mb_rem,
			gb_quot, gb_rem,
			(ulong)lba,
			desc->blksz);
#endif
	} else {
		puts ("            Capacity: not available\n");
	}
}

void part_init(struct blk_desc *desc)
{
	struct part_driver *drv =
		ll_entry_start(struct part_driver, part_driver);
	const int n_ents = ll_entry_count(struct part_driver, part_driver);
	struct part_driver *entry;

	blkcache_invalidate(desc->uclass_id, desc->devnum);

	desc->part_type = PART_TYPE_UNKNOWN;
	for (entry = drv; entry != drv + n_ents; entry++) {
		int ret;

		ret = entry->test(desc);
		debug("%s: try '%s': ret=%d\n", __func__, entry->name, ret);
		if (!ret) {
			desc->part_type = entry->part_type;
			break;
		}
	}
}

static void print_part_header(const char *type, struct blk_desc *desc)
{
#if CONFIG_IS_ENABLED(MAC_PARTITION) || \
	CONFIG_IS_ENABLED(DOS_PARTITION) || \
	CONFIG_IS_ENABLED(ISO_PARTITION) || \
	CONFIG_IS_ENABLED(AMIGA_PARTITION) || \
	CONFIG_IS_ENABLED(EFI_PARTITION)
	puts ("\nPartition Map for ");
	switch (desc->uclass_id) {
	case UCLASS_IDE:
		puts ("IDE");
		break;
	case UCLASS_AHCI:
		puts ("SATA");
		break;
	case UCLASS_SCSI:
		puts ("SCSI");
		break;
	case UCLASS_USB:
		puts ("USB");
		break;
	case UCLASS_MMC:
		puts ("MMC");
		break;
	case UCLASS_HOST:
		puts ("HOST");
		break;
	case UCLASS_NVME:
		puts ("NVMe");
		break;
	case UCLASS_PVBLOCK:
		puts("PV BLOCK");
		break;
	case UCLASS_RKMTD:
		puts("RKMTD");
		break;
	case UCLASS_VIRTIO:
		puts("VirtIO");
		break;
	case UCLASS_EFI_MEDIA:
		puts("EFI");
		break;
	case UCLASS_BLKMAP:
		puts("BLKMAP");
		break;
	default:
		printf("UNKNOWN(%d)", desc->uclass_id);
		break;
	}
	printf (" device %d  --   Partition Type: %s\n\n",
			desc->devnum, type);
#endif /* any CONFIG_..._PARTITION */
}

void part_print(struct blk_desc *desc)
{
	struct part_driver *drv;

	drv = part_driver_lookup_type(desc);
	if (!drv) {
		printf("## Unknown partition table type %x\n",
		       desc->part_type);
		return;
	}

	PRINTF("## Testing for valid %s partition ##\n", drv->name);
	print_part_header(drv->name, desc);
	if (drv->print)
		drv->print(desc);
}

int part_get_info_by_type(struct blk_desc *desc, int part, int part_type,
			  struct disk_partition *info)
{
	struct part_driver *drv;

	if (blk_enabled()) {
		/* The common case is no UUID support */
		disk_partition_clr_uuid(info);
		disk_partition_clr_type_guid(info);

		if (part_type == PART_TYPE_UNKNOWN) {
			drv = part_driver_lookup_type(desc);
		} else {
			drv = part_driver_get_type(part_type);
		}

		if (!drv) {
			debug("## Unknown partition table type %x\n",
			      desc->part_type);
			return -EPROTONOSUPPORT;
		}
		if (!drv->get_info) {
			PRINTF("## Driver %s does not have the get_info() method\n",
			       drv->name);
			return -ENOSYS;
		}
		if (drv->get_info(desc, part, info) == 0) {
			PRINTF("## Valid %s partition found ##\n", drv->name);
			return 0;
		}
	}

	return -ENOENT;
}

int part_get_info(struct blk_desc *desc, int part,
		  struct disk_partition *info)
{
	return part_get_info_by_type(desc, part, PART_TYPE_UNKNOWN, info);
}

int part_get_info_whole_disk(struct blk_desc *desc,
			     struct disk_partition *info)
{
	info->start = 0;
	info->size = desc->lba;
	info->blksz = desc->blksz;
	info->bootable = 0;
	strcpy((char *)info->type, BOOT_PART_TYPE);
	strcpy((char *)info->name, "Whole Disk");
	disk_partition_clr_uuid(info);
	disk_partition_clr_type_guid(info);

	return 0;
}

int blk_get_device_by_str(const char *ifname, const char *dev_hwpart_str,
			  struct blk_desc **desc)
{
	char *ep;
	char *dup_str = NULL;
	const char *dev_str, *hwpart_str;
	int dev, hwpart;

	hwpart_str = strchr(dev_hwpart_str, '.');
	if (hwpart_str) {
		dup_str = strdup(dev_hwpart_str);
		dup_str[hwpart_str - dev_hwpart_str] = 0;
		dev_str = dup_str;
		hwpart_str++;
	} else {
		dev_str = dev_hwpart_str;
		hwpart = 0;
	}

	dev = hextoul(dev_str, &ep);
	if (*ep) {
		printf("** Bad device specification %s %s **\n",
		       ifname, dev_str);
		dev = -EINVAL;
		goto cleanup;
	}

	if (hwpart_str) {
		hwpart = hextoul(hwpart_str, &ep);
		if (*ep) {
			printf("** Bad HW partition specification %s %s **\n",
			    ifname, hwpart_str);
			dev = -EINVAL;
			goto cleanup;
		}
	}

	*desc = get_dev_hwpart(ifname, dev, hwpart);
	if (!(*desc) || ((*desc)->type == DEV_TYPE_UNKNOWN)) {
		debug("** Bad device %s %s **\n", ifname, dev_hwpart_str);
		dev = -ENODEV;
		goto cleanup;
	}

	if (blk_enabled()) {
		/*
		 * Updates the partition table for the specified hw partition.
		 * Always should be done, otherwise hw partition 0 will return
		 * stale data after displaying a non-zero hw partition.
		 */
		if ((*desc)->uclass_id == UCLASS_MMC)
			part_init(*desc);
	}

cleanup:
	free(dup_str);
	return dev;
}

#define PART_UNSPECIFIED -2
#define PART_AUTO -1
int blk_get_device_part_str(const char *ifname, const char *dev_part_str,
			     struct blk_desc **desc,
			     struct disk_partition *info, int allow_whole_dev)
{
	int ret;
	const char *part_str;
	char *dup_str = NULL;
	const char *dev_str;
	int dev;
	char *ep;
	int p;
	int part;
	struct disk_partition tmpinfo;

	*desc = NULL;
	memset(info, 0, sizeof(*info));

#if IS_ENABLED(CONFIG_SANDBOX) || IS_ENABLED(CONFIG_SEMIHOSTING)
	/*
	 * Special-case a pseudo block device "hostfs", to allow access to the
	 * host's own filesystem.
	 */
	if (!strcmp(ifname, "hostfs")) {
		strcpy((char *)info->type, BOOT_PART_TYPE);
		strcpy((char *)info->name, "Host filesystem");

		return 0;
	}
#endif

#if IS_ENABLED(CONFIG_CMD_UBIFS) && !IS_ENABLED(CONFIG_SPL_BUILD)
	/*
	 * Special-case ubi, ubi goes through a mtd, rather than through
	 * a regular block device.
	 */
	if (!strcmp(ifname, "ubi")) {
		if (!ubifs_is_mounted()) {
			printf("UBIFS not mounted, use ubifsmount to mount volume first!\n");
			return -EINVAL;
		}

		strcpy((char *)info->type, BOOT_PART_TYPE);
		strcpy((char *)info->name, "UBI");
		return 0;
	}
#endif

	/* If no dev_part_str, use bootdevice environment variable */
	if (CONFIG_IS_ENABLED(ENV_SUPPORT)) {
		if (!dev_part_str || !strlen(dev_part_str) ||
		    !strcmp(dev_part_str, "-"))
			dev_part_str = env_get("bootdevice");
	}

	/* If still no dev_part_str, it's an error */
	if (!dev_part_str) {
		printf("** No device specified **\n");
		ret = -ENODEV;
		goto cleanup;
	}

	/* Separate device and partition ID specification */
	part_str = strchr(dev_part_str, ':');
	if (part_str) {
		dup_str = strdup(dev_part_str);
		dup_str[part_str - dev_part_str] = 0;
		dev_str = dup_str;
		part_str++;
	} else {
		dev_str = dev_part_str;
	}

	/* Look up the device */
	dev = blk_get_device_by_str(ifname, dev_str, desc);
	if (dev < 0) {
		printf("** Bad device specification %s %s **\n",
		       ifname, dev_str);
		ret = dev;
		goto cleanup;
	}

	/* Convert partition ID string to number */
	if (!part_str || !*part_str) {
		part = PART_UNSPECIFIED;
	} else if (!strcmp(part_str, "auto")) {
		part = PART_AUTO;
	} else {
		/* Something specified -> use exactly that */
		part = (int)hextoul(part_str, &ep);
		/*
		 * Less than whole string converted,
		 * or request for whole device, but caller requires partition.
		 */
		if (*ep || (part == 0 && !allow_whole_dev)) {
			printf("** Bad partition specification %s %s **\n",
			    ifname, dev_part_str);
			ret = -ENOENT;
			goto cleanup;
		}
	}

	/*
	 * No partition table on device,
	 * or user requested partition 0 (entire device).
	 */
	if (((*desc)->part_type == PART_TYPE_UNKNOWN) || !part) {
		if (!(*desc)->lba) {
			printf("** Bad device size - %s %s **\n", ifname,
			       dev_str);
			ret = -EINVAL;
			goto cleanup;
		}

		/*
		 * If user specified a partition ID other than 0,
		 * or the calling command only accepts partitions,
		 * it's an error.
		 */
		if ((part > 0) || (!allow_whole_dev)) {
			printf("** No partition table - %s %s **\n", ifname,
			       dev_str);
			ret = -EPROTONOSUPPORT;
			goto cleanup;
		}

		(*desc)->log2blksz = LOG2((*desc)->blksz);

		part_get_info_whole_disk(*desc, info);

		ret = 0;
		goto cleanup;
	}

	/*
	 * Now there's known to be a partition table,
	 * not specifying a partition means to pick partition 1.
	 */
	if (part == PART_UNSPECIFIED)
		part = 1;

	/*
	 * If user didn't specify a partition number, or did specify something
	 * other than "auto", use that partition number directly.
	 */
	if (part != PART_AUTO) {
		ret = part_get_info(*desc, part, info);
		if (ret) {
			printf("** Invalid partition %d **\n", part);
			goto cleanup;
		}
	} else {
		/*
		 * Find the first bootable partition.
		 * If none are bootable, fall back to the first valid partition.
		 */
		part = 0;
		for (p = 1; p <= MAX_SEARCH_PARTITIONS; p++) {
			ret = part_get_info(*desc, p, info);
			if (ret)
				continue;

			/*
			 * First valid partition, or new better partition?
			 * If so, save partition ID.
			 */
			if (!part || info->bootable)
				part = p;

			/* Best possible partition? Stop searching. */
			if (info->bootable)
				break;

			/*
			 * We now need to search further for best possible.
			 * If we what we just queried was the best so far,
			 * save the info since we over-write it next loop.
			 */
			if (part == p)
				tmpinfo = *info;
		}
		/* If we found any acceptable partition */
		if (part) {
			/*
			 * If we searched all possible partition IDs,
			 * return the first valid partition we found.
			 */
			if (p == MAX_SEARCH_PARTITIONS + 1)
				*info = tmpinfo;
		} else {
			printf("** No valid partitions found **\n");
			goto cleanup;
		}
	}
	if (strncmp((char *)info->type, BOOT_PART_TYPE, sizeof(info->type)) != 0) {
		printf("** Invalid partition type \"%.32s\""
			" (expect \"" BOOT_PART_TYPE "\")\n",
			info->type);
		ret  = -EINVAL;
		goto cleanup;
	}

	(*desc)->log2blksz = LOG2((*desc)->blksz);

	ret = part;
	goto cleanup;

cleanup:
	free(dup_str);
	return ret;
}

int part_get_info_by_name(struct blk_desc *desc, const char *name,
			  struct disk_partition *info)
{
	struct part_driver *part_drv;
	int ret;
	int i;

	part_drv = part_driver_lookup_type(desc);
	if (!part_drv)
		return -1;

	if (!part_drv->get_info) {
		log_debug("## Driver %s does not have the get_info() method\n",
			  part_drv->name);
		return -ENOSYS;
	}

	for (i = 1; i < part_drv->max_entries; i++) {
		ret = part_drv->get_info(desc, i, info);
		if (ret != 0) {
			/* no more entries in table */
			break;
		}
		if (strcmp(name, (const char *)info->name) == 0) {
			/* matched */
			return i;
		}
	}

	return -ENOENT;
}

/**
 * Get partition info from device number and partition name.
 *
 * Parse a device number and partition name string in the form of
 * "devicenum.hwpartnum#partition_name", for example "0.1#misc". devicenum and
 * hwpartnum are both optional, defaulting to 0. If the partition is found,
 * sets desc and part_info accordingly with the information of the
 * partition with the given partition_name.
 *
 * @param[in] dev_iface Device interface
 * @param[in] dev_part_str Input string argument, like "0.1#misc"
 * @param[out] desc Place to store the device description pointer
 * @param[out] part_info Place to store the partition information
 * Return: 0 on success, or a negative on error
 */
static int part_get_info_by_dev_and_name(const char *dev_iface,
					 const char *dev_part_str,
					 struct blk_desc **desc,
					 struct disk_partition *part_info)
{
	char *dup_str = NULL;
	const char *dev_str, *part_str;
	int ret;

	/* Separate device and partition name specification */
	if (dev_part_str)
		part_str = strchr(dev_part_str, '#');
	else
		part_str = NULL;

	if (part_str) {
		dup_str = strdup(dev_part_str);
		dup_str[part_str - dev_part_str] = 0;
		dev_str = dup_str;
		part_str++;
	} else {
		return -EINVAL;
	}

	ret = blk_get_device_by_str(dev_iface, dev_str, desc);
	if (ret < 0)
		goto cleanup;

	ret = part_get_info_by_name(*desc, part_str, part_info);
	if (ret < 0)
		printf("Could not find \"%s\" partition\n", part_str);

cleanup:
	free(dup_str);
	return ret;
}

int part_get_info_by_dev_and_name_or_num(const char *dev_iface,
					 const char *dev_part_str,
					 struct blk_desc **desc,
					 struct disk_partition *part_info,
					 int allow_whole_dev)
{
	int ret;

	/* Split the part_name if passed as "$dev_num#part_name". */
	ret = part_get_info_by_dev_and_name(dev_iface, dev_part_str, desc,
					    part_info);
	if (ret >= 0)
		return ret;
	/*
	 * Couldn't lookup by name, try looking up the partition description
	 * directly.
	 */
	ret = blk_get_device_part_str(dev_iface, dev_part_str, desc, part_info,
				      allow_whole_dev);
	if (ret < 0)
		printf("Couldn't find partition %s %s\n",
		       dev_iface, dev_part_str);
	return ret;
}

void part_set_generic_name(const struct blk_desc *desc, int part_num,
			   char *name)
{
	char *devtype;

	switch (desc->uclass_id) {
	case UCLASS_IDE:
	case UCLASS_AHCI:
		devtype = "hd";
		break;
	case UCLASS_SCSI:
		devtype = "sd";
		break;
	case UCLASS_USB:
		devtype = "usbd";
		break;
	case UCLASS_MMC:
		devtype = "mmcsd";
		break;
	default:
		devtype = "xx";
		break;
	}

	sprintf(name, "%s%c%d", devtype, 'a' + desc->devnum, part_num);
}

int part_get_bootable(struct blk_desc *desc)
{
	struct disk_partition info;
	int p;

	for (p = 1; p <= MAX_SEARCH_PARTITIONS; p++) {
		int ret;

		ret = part_get_info(desc, p, &info);
		if (!ret && info.bootable)
			return p;
	}

	return 0;
}
