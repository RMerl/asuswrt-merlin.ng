// SPDX-License-Identifier: GPL-2.0+
/*
 * EFI device path from u-boot device-model mapping
 *
 * (C) Copyright 2017 Rob Clark
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <usb.h>
#include <mmc.h>
#include <efi_loader.h>
#include <part.h>

/* template END node: */
static const struct efi_device_path END = {
	.type     = DEVICE_PATH_TYPE_END,
	.sub_type = DEVICE_PATH_SUB_TYPE_END,
	.length   = sizeof(END),
};

/* template ROOT node: */
static const struct efi_device_path_vendor ROOT = {
	.dp = {
		.type     = DEVICE_PATH_TYPE_HARDWARE_DEVICE,
		.sub_type = DEVICE_PATH_SUB_TYPE_VENDOR,
		.length   = sizeof(ROOT),
	},
	.guid = U_BOOT_GUID,
};

#if defined(CONFIG_DM_MMC) && defined(CONFIG_MMC)
/*
 * Determine if an MMC device is an SD card.
 *
 * @desc	block device descriptor
 * @return	true if the device is an SD card
 */
static bool is_sd(struct blk_desc *desc)
{
	struct mmc *mmc = find_mmc_device(desc->devnum);

	if (!mmc)
		return false;

	return IS_SD(mmc) != 0U;
}
#endif

static void *dp_alloc(size_t sz)
{
	void *buf;

	if (efi_allocate_pool(EFI_ALLOCATE_ANY_PAGES, sz, &buf) !=
	    EFI_SUCCESS) {
		debug("EFI: ERROR: out of memory in %s\n", __func__);
		return NULL;
	}

	memset(buf, 0, sz);
	return buf;
}

/*
 * Iterate to next block in device-path, terminating (returning NULL)
 * at /End* node.
 */
struct efi_device_path *efi_dp_next(const struct efi_device_path *dp)
{
	if (dp == NULL)
		return NULL;
	if (dp->type == DEVICE_PATH_TYPE_END)
		return NULL;
	dp = ((void *)dp) + dp->length;
	if (dp->type == DEVICE_PATH_TYPE_END)
		return NULL;
	return (struct efi_device_path *)dp;
}

/*
 * Compare two device-paths, stopping when the shorter of the two hits
 * an End* node. This is useful to, for example, compare a device-path
 * representing a device with one representing a file on the device, or
 * a device with a parent device.
 */
int efi_dp_match(const struct efi_device_path *a,
		 const struct efi_device_path *b)
{
	while (1) {
		int ret;

		ret = memcmp(&a->length, &b->length, sizeof(a->length));
		if (ret)
			return ret;

		ret = memcmp(a, b, a->length);
		if (ret)
			return ret;

		a = efi_dp_next(a);
		b = efi_dp_next(b);

		if (!a || !b)
			return 0;
	}
}

/*
 * We can have device paths that start with a USB WWID or a USB Class node,
 * and a few other cases which don't encode the full device path with bus
 * hierarchy:
 *
 *   - MESSAGING:USB_WWID
 *   - MESSAGING:USB_CLASS
 *   - MEDIA:FILE_PATH
 *   - MEDIA:HARD_DRIVE
 *   - MESSAGING:URI
 *
 * See UEFI spec (section 3.1.2, about short-form device-paths)
 */
static struct efi_device_path *shorten_path(struct efi_device_path *dp)
{
	while (dp) {
		/*
		 * TODO: Add MESSAGING:USB_WWID and MESSAGING:URI..
		 * in practice fallback.efi just uses MEDIA:HARD_DRIVE
		 * so not sure when we would see these other cases.
		 */
		if (EFI_DP_TYPE(dp, MESSAGING_DEVICE, MSG_USB_CLASS) ||
		    EFI_DP_TYPE(dp, MEDIA_DEVICE, HARD_DRIVE_PATH) ||
		    EFI_DP_TYPE(dp, MEDIA_DEVICE, FILE_PATH))
			return dp;

		dp = efi_dp_next(dp);
	}

	return dp;
}

static struct efi_object *find_obj(struct efi_device_path *dp, bool short_path,
				   struct efi_device_path **rem)
{
	struct efi_object *efiobj;
	efi_uintn_t dp_size = efi_dp_instance_size(dp);

	list_for_each_entry(efiobj, &efi_obj_list, link) {
		struct efi_handler *handler;
		struct efi_device_path *obj_dp;
		efi_status_t ret;

		ret = efi_search_protocol(efiobj,
					  &efi_guid_device_path, &handler);
		if (ret != EFI_SUCCESS)
			continue;
		obj_dp = handler->protocol_interface;

		do {
			if (efi_dp_match(dp, obj_dp) == 0) {
				if (rem) {
					/*
					 * Allow partial matches, but inform
					 * the caller.
					 */
					*rem = ((void *)dp) +
						efi_dp_instance_size(obj_dp);
					return efiobj;
				} else {
					/* Only return on exact matches */
					if (efi_dp_instance_size(obj_dp) ==
					    dp_size)
						return efiobj;
				}
			}

			obj_dp = shorten_path(efi_dp_next(obj_dp));
		} while (short_path && obj_dp);
	}

	return NULL;
}

/*
 * Find an efiobj from device-path, if 'rem' is not NULL, returns the
 * remaining part of the device path after the matched object.
 */
struct efi_object *efi_dp_find_obj(struct efi_device_path *dp,
				   struct efi_device_path **rem)
{
	struct efi_object *efiobj;

	/* Search for an exact match first */
	efiobj = find_obj(dp, false, NULL);

	/* Then for a fuzzy match */
	if (!efiobj)
		efiobj = find_obj(dp, false, rem);

	/* And now for a fuzzy short match */
	if (!efiobj)
		efiobj = find_obj(dp, true, rem);

	return efiobj;
}

/*
 * Determine the last device path node that is not the end node.
 *
 * @dp		device path
 * @return	last node before the end node if it exists
 *		otherwise NULL
 */
const struct efi_device_path *efi_dp_last_node(const struct efi_device_path *dp)
{
	struct efi_device_path *ret;

	if (!dp || dp->type == DEVICE_PATH_TYPE_END)
		return NULL;
	while (dp) {
		ret = (struct efi_device_path *)dp;
		dp = efi_dp_next(dp);
	}
	return ret;
}

/* get size of the first device path instance excluding end node */
efi_uintn_t efi_dp_instance_size(const struct efi_device_path *dp)
{
	efi_uintn_t sz = 0;

	if (!dp || dp->type == DEVICE_PATH_TYPE_END)
		return 0;
	while (dp) {
		sz += dp->length;
		dp = efi_dp_next(dp);
	}

	return sz;
}

/* get size of multi-instance device path excluding end node */
efi_uintn_t efi_dp_size(const struct efi_device_path *dp)
{
	const struct efi_device_path *p = dp;

	if (!p)
		return 0;
	while (p->type != DEVICE_PATH_TYPE_END ||
	       p->sub_type != DEVICE_PATH_SUB_TYPE_END)
		p = (void *)p + p->length;

	return (void *)p - (void *)dp;
}

/* copy multi-instance device path */
struct efi_device_path *efi_dp_dup(const struct efi_device_path *dp)
{
	struct efi_device_path *ndp;
	size_t sz = efi_dp_size(dp) + sizeof(END);

	if (!dp)
		return NULL;

	ndp = dp_alloc(sz);
	if (!ndp)
		return NULL;
	memcpy(ndp, dp, sz);

	return ndp;
}

struct efi_device_path *efi_dp_append(const struct efi_device_path *dp1,
				      const struct efi_device_path *dp2)
{
	struct efi_device_path *ret;

	if (!dp1 && !dp2) {
		/* return an end node */
		ret = efi_dp_dup(&END);
	} else if (!dp1) {
		ret = efi_dp_dup(dp2);
	} else if (!dp2) {
		ret = efi_dp_dup(dp1);
	} else {
		/* both dp1 and dp2 are non-null */
		unsigned sz1 = efi_dp_size(dp1);
		unsigned sz2 = efi_dp_size(dp2);
		void *p = dp_alloc(sz1 + sz2 + sizeof(END));
		if (!p)
			return NULL;
		memcpy(p, dp1, sz1);
		/* the end node of the second device path has to be retained */
		memcpy(p + sz1, dp2, sz2 + sizeof(END));
		ret = p;
	}

	return ret;
}

struct efi_device_path *efi_dp_append_node(const struct efi_device_path *dp,
					   const struct efi_device_path *node)
{
	struct efi_device_path *ret;

	if (!node && !dp) {
		ret = efi_dp_dup(&END);
	} else if (!node) {
		ret = efi_dp_dup(dp);
	} else if (!dp) {
		size_t sz = node->length;
		void *p = dp_alloc(sz + sizeof(END));
		if (!p)
			return NULL;
		memcpy(p, node, sz);
		memcpy(p + sz, &END, sizeof(END));
		ret = p;
	} else {
		/* both dp and node are non-null */
		size_t sz = efi_dp_size(dp);
		void *p = dp_alloc(sz + node->length + sizeof(END));
		if (!p)
			return NULL;
		memcpy(p, dp, sz);
		memcpy(p + sz, node, node->length);
		memcpy(p + sz + node->length, &END, sizeof(END));
		ret = p;
	}

	return ret;
}

struct efi_device_path *efi_dp_create_device_node(const u8 type,
						  const u8 sub_type,
						  const u16 length)
{
	struct efi_device_path *ret;

	if (length < sizeof(struct efi_device_path))
		return NULL;

	ret = dp_alloc(length);
	if (!ret)
		return ret;
	ret->type = type;
	ret->sub_type = sub_type;
	ret->length = length;
	return ret;
}

struct efi_device_path *efi_dp_append_instance(
		const struct efi_device_path *dp,
		const struct efi_device_path *dpi)
{
	size_t sz, szi;
	struct efi_device_path *p, *ret;

	if (!dpi)
		return NULL;
	if (!dp)
		return efi_dp_dup(dpi);
	sz = efi_dp_size(dp);
	szi = efi_dp_instance_size(dpi);
	p = dp_alloc(sz + szi + 2 * sizeof(END));
	if (!p)
		return NULL;
	ret = p;
	memcpy(p, dp, sz + sizeof(END));
	p = (void *)p + sz;
	p->sub_type = DEVICE_PATH_SUB_TYPE_INSTANCE_END;
	p = (void *)p + sizeof(END);
	memcpy(p, dpi, szi);
	p = (void *)p + szi;
	memcpy(p, &END, sizeof(END));
	return ret;
}

struct efi_device_path *efi_dp_get_next_instance(struct efi_device_path **dp,
						 efi_uintn_t *size)
{
	size_t sz;
	struct efi_device_path *p;

	if (size)
		*size = 0;
	if (!dp || !*dp)
		return NULL;
	sz = efi_dp_instance_size(*dp);
	p = dp_alloc(sz + sizeof(END));
	if (!p)
		return NULL;
	memcpy(p, *dp, sz + sizeof(END));
	*dp = (void *)*dp + sz;
	if ((*dp)->sub_type == DEVICE_PATH_SUB_TYPE_INSTANCE_END)
		*dp = (void *)*dp + sizeof(END);
	else
		*dp = NULL;
	if (size)
		*size = sz + sizeof(END);
	return p;
}

bool efi_dp_is_multi_instance(const struct efi_device_path *dp)
{
	const struct efi_device_path *p = dp;

	if (!p)
		return false;
	while (p->type != DEVICE_PATH_TYPE_END)
		p = (void *)p + p->length;
	return p->sub_type == DEVICE_PATH_SUB_TYPE_INSTANCE_END;
}

#ifdef CONFIG_DM
/* size of device-path not including END node for device and all parents
 * up to the root device.
 */
static unsigned dp_size(struct udevice *dev)
{
	if (!dev || !dev->driver)
		return sizeof(ROOT);

	switch (dev->driver->id) {
	case UCLASS_ROOT:
	case UCLASS_SIMPLE_BUS:
		/* stop traversing parents at this point: */
		return sizeof(ROOT);
	case UCLASS_ETH:
		return dp_size(dev->parent) +
			sizeof(struct efi_device_path_mac_addr);
#ifdef CONFIG_BLK
	case UCLASS_BLK:
		switch (dev->parent->uclass->uc_drv->id) {
#ifdef CONFIG_IDE
		case UCLASS_IDE:
			return dp_size(dev->parent) +
				sizeof(struct efi_device_path_atapi);
#endif
#if defined(CONFIG_SCSI) && defined(CONFIG_DM_SCSI)
		case UCLASS_SCSI:
			return dp_size(dev->parent) +
				sizeof(struct efi_device_path_scsi);
#endif
#if defined(CONFIG_DM_MMC) && defined(CONFIG_MMC)
		case UCLASS_MMC:
			return dp_size(dev->parent) +
				sizeof(struct efi_device_path_sd_mmc_path);
#endif
		default:
			return dp_size(dev->parent);
		}
#endif
#if defined(CONFIG_DM_MMC) && defined(CONFIG_MMC)
	case UCLASS_MMC:
		return dp_size(dev->parent) +
			sizeof(struct efi_device_path_sd_mmc_path);
#endif
	case UCLASS_MASS_STORAGE:
	case UCLASS_USB_HUB:
		return dp_size(dev->parent) +
			sizeof(struct efi_device_path_usb_class);
	default:
		/* just skip over unknown classes: */
		return dp_size(dev->parent);
	}
}

/*
 * Recursively build a device path.
 *
 * @buf		pointer to the end of the device path
 * @dev		device
 * @return	pointer to the end of the device path
 */
static void *dp_fill(void *buf, struct udevice *dev)
{
	if (!dev || !dev->driver)
		return buf;

	switch (dev->driver->id) {
	case UCLASS_ROOT:
	case UCLASS_SIMPLE_BUS: {
		/* stop traversing parents at this point: */
		struct efi_device_path_vendor *vdp = buf;
		*vdp = ROOT;
		return &vdp[1];
	}
#ifdef CONFIG_DM_ETH
	case UCLASS_ETH: {
		struct efi_device_path_mac_addr *dp =
			dp_fill(buf, dev->parent);
		struct eth_pdata *pdata = dev->platdata;

		dp->dp.type = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
		dp->dp.sub_type = DEVICE_PATH_SUB_TYPE_MSG_MAC_ADDR;
		dp->dp.length = sizeof(*dp);
		memset(&dp->mac, 0, sizeof(dp->mac));
		/* We only support IPv4 */
		memcpy(&dp->mac, &pdata->enetaddr, ARP_HLEN);
		/* Ethernet */
		dp->if_type = 1;
		return &dp[1];
	}
#endif
#ifdef CONFIG_BLK
	case UCLASS_BLK:
		switch (dev->parent->uclass->uc_drv->id) {
#ifdef CONFIG_IDE
		case UCLASS_IDE: {
			struct efi_device_path_atapi *dp =
			dp_fill(buf, dev->parent);
			struct blk_desc *desc = dev_get_uclass_platdata(dev);

			dp->dp.type = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
			dp->dp.sub_type = DEVICE_PATH_SUB_TYPE_MSG_ATAPI;
			dp->dp.length = sizeof(*dp);
			dp->logical_unit_number = desc->devnum;
			dp->primary_secondary = IDE_BUS(desc->devnum);
			dp->slave_master = desc->devnum %
				(CONFIG_SYS_IDE_MAXDEVICE /
				 CONFIG_SYS_IDE_MAXBUS);
			return &dp[1];
			}
#endif
#if defined(CONFIG_SCSI) && defined(CONFIG_DM_SCSI)
		case UCLASS_SCSI: {
			struct efi_device_path_scsi *dp =
				dp_fill(buf, dev->parent);
			struct blk_desc *desc = dev_get_uclass_platdata(dev);

			dp->dp.type = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
			dp->dp.sub_type = DEVICE_PATH_SUB_TYPE_MSG_SCSI;
			dp->dp.length = sizeof(*dp);
			dp->logical_unit_number = desc->lun;
			dp->target_id = desc->target;
			return &dp[1];
			}
#endif
#if defined(CONFIG_DM_MMC) && defined(CONFIG_MMC)
		case UCLASS_MMC: {
			struct efi_device_path_sd_mmc_path *sddp =
				dp_fill(buf, dev->parent);
			struct blk_desc *desc = dev_get_uclass_platdata(dev);

			sddp->dp.type     = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
			sddp->dp.sub_type = is_sd(desc) ?
				DEVICE_PATH_SUB_TYPE_MSG_SD :
				DEVICE_PATH_SUB_TYPE_MSG_MMC;
			sddp->dp.length   = sizeof(*sddp);
			sddp->slot_number = dev->seq;
			return &sddp[1];
			}
#endif
		default:
			debug("%s(%u) %s: unhandled parent class: %s (%u)\n",
			      __FILE__, __LINE__, __func__,
			      dev->name, dev->parent->uclass->uc_drv->id);
			return dp_fill(buf, dev->parent);
		}
#endif
#if defined(CONFIG_DM_MMC) && defined(CONFIG_MMC)
	case UCLASS_MMC: {
		struct efi_device_path_sd_mmc_path *sddp =
			dp_fill(buf, dev->parent);
		struct mmc *mmc = mmc_get_mmc_dev(dev);
		struct blk_desc *desc = mmc_get_blk_desc(mmc);

		sddp->dp.type     = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
		sddp->dp.sub_type = is_sd(desc) ?
			DEVICE_PATH_SUB_TYPE_MSG_SD :
			DEVICE_PATH_SUB_TYPE_MSG_MMC;
		sddp->dp.length   = sizeof(*sddp);
		sddp->slot_number = dev->seq;

		return &sddp[1];
	}
#endif
	case UCLASS_MASS_STORAGE:
	case UCLASS_USB_HUB: {
		struct efi_device_path_usb_class *udp =
			dp_fill(buf, dev->parent);
		struct usb_device *udev = dev_get_parent_priv(dev);
		struct usb_device_descriptor *desc = &udev->descriptor;

		udp->dp.type     = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
		udp->dp.sub_type = DEVICE_PATH_SUB_TYPE_MSG_USB_CLASS;
		udp->dp.length   = sizeof(*udp);
		udp->vendor_id   = desc->idVendor;
		udp->product_id  = desc->idProduct;
		udp->device_class    = desc->bDeviceClass;
		udp->device_subclass = desc->bDeviceSubClass;
		udp->device_protocol = desc->bDeviceProtocol;

		return &udp[1];
	}
	default:
		debug("%s(%u) %s: unhandled device class: %s (%u)\n",
		      __FILE__, __LINE__, __func__,
		      dev->name, dev->driver->id);
		return dp_fill(buf, dev->parent);
	}
}

/* Construct a device-path from a device: */
struct efi_device_path *efi_dp_from_dev(struct udevice *dev)
{
	void *buf, *start;

	start = buf = dp_alloc(dp_size(dev) + sizeof(END));
	if (!buf)
		return NULL;
	buf = dp_fill(buf, dev);
	*((struct efi_device_path *)buf) = END;

	return start;
}
#endif

static unsigned dp_part_size(struct blk_desc *desc, int part)
{
	unsigned dpsize;

#ifdef CONFIG_BLK
	{
		struct udevice *dev;
		int ret = blk_find_device(desc->if_type, desc->devnum, &dev);

		if (ret)
			dev = desc->bdev->parent;
		dpsize = dp_size(dev);
	}
#else
	dpsize = sizeof(ROOT) + sizeof(struct efi_device_path_usb);
#endif

	if (part == 0) /* the actual disk, not a partition */
		return dpsize;

	if (desc->part_type == PART_TYPE_ISO)
		dpsize += sizeof(struct efi_device_path_cdrom_path);
	else
		dpsize += sizeof(struct efi_device_path_hard_drive_path);

	return dpsize;
}

/*
 * Create a device node for a block device partition.
 *
 * @buf		buffer to which the device path is written
 * @desc	block device descriptor
 * @part	partition number, 0 identifies a block device
 */
static void *dp_part_node(void *buf, struct blk_desc *desc, int part)
{
	disk_partition_t info;

	part_get_info(desc, part, &info);

	if (desc->part_type == PART_TYPE_ISO) {
		struct efi_device_path_cdrom_path *cddp = buf;

		cddp->boot_entry = part;
		cddp->dp.type = DEVICE_PATH_TYPE_MEDIA_DEVICE;
		cddp->dp.sub_type = DEVICE_PATH_SUB_TYPE_CDROM_PATH;
		cddp->dp.length = sizeof(*cddp);
		cddp->partition_start = info.start;
		cddp->partition_end = info.size;

		buf = &cddp[1];
	} else {
		struct efi_device_path_hard_drive_path *hddp = buf;

		hddp->dp.type = DEVICE_PATH_TYPE_MEDIA_DEVICE;
		hddp->dp.sub_type = DEVICE_PATH_SUB_TYPE_HARD_DRIVE_PATH;
		hddp->dp.length = sizeof(*hddp);
		hddp->partition_number = part;
		hddp->partition_start = info.start;
		hddp->partition_end = info.size;
		if (desc->part_type == PART_TYPE_EFI)
			hddp->partmap_type = 2;
		else
			hddp->partmap_type = 1;

		switch (desc->sig_type) {
		case SIG_TYPE_NONE:
		default:
			hddp->signature_type = 0;
			memset(hddp->partition_signature, 0,
			       sizeof(hddp->partition_signature));
			break;
		case SIG_TYPE_MBR:
			hddp->signature_type = 1;
			memset(hddp->partition_signature, 0,
			       sizeof(hddp->partition_signature));
			memcpy(hddp->partition_signature, &desc->mbr_sig,
			       sizeof(desc->mbr_sig));
			break;
		case SIG_TYPE_GUID:
			hddp->signature_type = 2;
			memcpy(hddp->partition_signature, &desc->guid_sig,
			       sizeof(hddp->partition_signature));
			break;
		}

		buf = &hddp[1];
	}

	return buf;
}

/*
 * Create a device path for a block device or one of its partitions.
 *
 * @buf		buffer to which the device path is written
 * @desc	block device descriptor
 * @part	partition number, 0 identifies a block device
 */
static void *dp_part_fill(void *buf, struct blk_desc *desc, int part)
{
#ifdef CONFIG_BLK
	{
		struct udevice *dev;
		int ret = blk_find_device(desc->if_type, desc->devnum, &dev);

		if (ret)
			dev = desc->bdev->parent;
		buf = dp_fill(buf, dev);
	}
#else
	/*
	 * We *could* make a more accurate path, by looking at if_type
	 * and handling all the different cases like we do for non-
	 * legacy (i.e. CONFIG_BLK=y) case. But most important thing
	 * is just to have a unique device-path for if_type+devnum.
	 * So map things to a fictitious USB device.
	 */
	struct efi_device_path_usb *udp;

	memcpy(buf, &ROOT, sizeof(ROOT));
	buf += sizeof(ROOT);

	udp = buf;
	udp->dp.type = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
	udp->dp.sub_type = DEVICE_PATH_SUB_TYPE_MSG_USB;
	udp->dp.length = sizeof(*udp);
	udp->parent_port_number = desc->if_type;
	udp->usb_interface = desc->devnum;
	buf = &udp[1];
#endif

	if (part == 0) /* the actual disk, not a partition */
		return buf;

	return dp_part_node(buf, desc, part);
}

/* Construct a device-path from a partition on a block device: */
struct efi_device_path *efi_dp_from_part(struct blk_desc *desc, int part)
{
	void *buf, *start;

	start = buf = dp_alloc(dp_part_size(desc, part) + sizeof(END));
	if (!buf)
		return NULL;

	buf = dp_part_fill(buf, desc, part);

	*((struct efi_device_path *)buf) = END;

	return start;
}

/*
 * Create a device node for a block device partition.
 *
 * @buf		buffer to which the device path is written
 * @desc	block device descriptor
 * @part	partition number, 0 identifies a block device
 */
struct efi_device_path *efi_dp_part_node(struct blk_desc *desc, int part)
{
	efi_uintn_t dpsize;
	void *buf;

	if (desc->part_type == PART_TYPE_ISO)
		dpsize = sizeof(struct efi_device_path_cdrom_path);
	else
		dpsize = sizeof(struct efi_device_path_hard_drive_path);
	buf = dp_alloc(dpsize);

	dp_part_node(buf, desc, part);

	return buf;
}

/* convert path to an UEFI style path (i.e. DOS style backslashes and UTF-16) */
static void path_to_uefi(u16 *uefi, const char *path)
{
	while (*path) {
		char c = *(path++);
		if (c == '/')
			c = '\\';
		*(uefi++) = c;
	}
	*uefi = '\0';
}

/*
 * If desc is NULL, this creates a path with only the file component,
 * otherwise it creates a full path with both device and file components
 */
struct efi_device_path *efi_dp_from_file(struct blk_desc *desc, int part,
		const char *path)
{
	struct efi_device_path_file_path *fp;
	void *buf, *start;
	unsigned dpsize = 0, fpsize;

	if (desc)
		dpsize = dp_part_size(desc, part);

	fpsize = sizeof(struct efi_device_path) + 2 * (strlen(path) + 1);
	dpsize += fpsize;

	start = buf = dp_alloc(dpsize + sizeof(END));
	if (!buf)
		return NULL;

	if (desc)
		buf = dp_part_fill(buf, desc, part);

	/* add file-path: */
	fp = buf;
	fp->dp.type = DEVICE_PATH_TYPE_MEDIA_DEVICE;
	fp->dp.sub_type = DEVICE_PATH_SUB_TYPE_FILE_PATH;
	fp->dp.length = fpsize;
	path_to_uefi(fp->str, path);
	buf += fpsize;

	*((struct efi_device_path *)buf) = END;

	return start;
}

#ifdef CONFIG_NET
struct efi_device_path *efi_dp_from_eth(void)
{
#ifndef CONFIG_DM_ETH
	struct efi_device_path_mac_addr *ndp;
#endif
	void *buf, *start;
	unsigned dpsize = 0;

	assert(eth_get_dev());

#ifdef CONFIG_DM_ETH
	dpsize += dp_size(eth_get_dev());
#else
	dpsize += sizeof(ROOT);
	dpsize += sizeof(*ndp);
#endif

	start = buf = dp_alloc(dpsize + sizeof(END));
	if (!buf)
		return NULL;

#ifdef CONFIG_DM_ETH
	buf = dp_fill(buf, eth_get_dev());
#else
	memcpy(buf, &ROOT, sizeof(ROOT));
	buf += sizeof(ROOT);

	ndp = buf;
	ndp->dp.type = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
	ndp->dp.sub_type = DEVICE_PATH_SUB_TYPE_MSG_MAC_ADDR;
	ndp->dp.length = sizeof(*ndp);
	ndp->if_type = 1; /* Ethernet */
	memcpy(ndp->mac.addr, eth_get_ethaddr(), ARP_HLEN);
	buf = &ndp[1];
#endif

	*((struct efi_device_path *)buf) = END;

	return start;
}
#endif

/* Construct a device-path for memory-mapped image */
struct efi_device_path *efi_dp_from_mem(uint32_t memory_type,
					uint64_t start_address,
					uint64_t end_address)
{
	struct efi_device_path_memory *mdp;
	void *buf, *start;

	start = buf = dp_alloc(sizeof(*mdp) + sizeof(END));
	if (!buf)
		return NULL;

	mdp = buf;
	mdp->dp.type = DEVICE_PATH_TYPE_HARDWARE_DEVICE;
	mdp->dp.sub_type = DEVICE_PATH_SUB_TYPE_MEMORY;
	mdp->dp.length = sizeof(*mdp);
	mdp->memory_type = memory_type;
	mdp->start_address = start_address;
	mdp->end_address = end_address;
	buf = &mdp[1];

	*((struct efi_device_path *)buf) = END;

	return start;
}

/**
 * efi_dp_split_file_path() - split of relative file path from device path
 *
 * Given a device path indicating a file on a device, separate the device
 * path in two: the device path of the actual device and the file path
 * relative to this device.
 *
 * @full_path:		device path including device and file path
 * @device_path:	path of the device
 * @file_path:		relative path of the file or NULL if there is none
 * Return:		status code
 */
efi_status_t efi_dp_split_file_path(struct efi_device_path *full_path,
				    struct efi_device_path **device_path,
				    struct efi_device_path **file_path)
{
	struct efi_device_path *p, *dp, *fp = NULL;

	*device_path = NULL;
	*file_path = NULL;
	dp = efi_dp_dup(full_path);
	if (!dp)
		return EFI_OUT_OF_RESOURCES;
	p = dp;
	while (!EFI_DP_TYPE(p, MEDIA_DEVICE, FILE_PATH)) {
		p = efi_dp_next(p);
		if (!p)
			goto out;
	}
	fp = efi_dp_dup(p);
	if (!fp)
		return EFI_OUT_OF_RESOURCES;
	p->type = DEVICE_PATH_TYPE_END;
	p->sub_type = DEVICE_PATH_SUB_TYPE_END;
	p->length = sizeof(*p);

out:
	*device_path = dp;
	*file_path = fp;
	return EFI_SUCCESS;
}

efi_status_t efi_dp_from_name(const char *dev, const char *devnr,
			      const char *path,
			      struct efi_device_path **device,
			      struct efi_device_path **file)
{
	int is_net;
	struct blk_desc *desc = NULL;
	disk_partition_t fs_partition;
	int part = 0;
	char filename[32] = { 0 }; /* dp->str is u16[32] long */
	char *s;

	if (path && !file)
		return EFI_INVALID_PARAMETER;

	is_net = !strcmp(dev, "Net");
	if (!is_net) {
		part = blk_get_device_part_str(dev, devnr, &desc, &fs_partition,
					       1);
		if (part < 0 || !desc)
			return EFI_INVALID_PARAMETER;

		if (device)
			*device = efi_dp_from_part(desc, part);
	} else {
#ifdef CONFIG_NET
		if (device)
			*device = efi_dp_from_eth();
#endif
	}

	if (!path)
		return EFI_SUCCESS;

	snprintf(filename, sizeof(filename), "%s", path);
	/* DOS style file path: */
	s = filename;
	while ((s = strchr(s, '/')))
		*s++ = '\\';
	*file = efi_dp_from_file(((!is_net && device) ? desc : NULL),
				 part, filename);

	return EFI_SUCCESS;
}
