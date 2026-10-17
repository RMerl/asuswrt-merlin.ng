// SPDX-License-Identifier: GPL-2.0+
/*
 * EFI device path from u-boot device-model mapping
 *
 * (C) Copyright 2017 Rob Clark
 */

#define LOG_CATEGORY LOGC_EFI

#include <blk.h>
#include <dm.h>
#include <dm/root.h>
#include <log.h>
#include <net.h>
#include <usb.h>
#include <mmc.h>
#include <nvme.h>
#include <efi_loader.h>
#include <part.h>
#include <uuid.h>
#include <asm-generic/unaligned.h>
#include <linux/compat.h> /* U16_MAX */

/* template END node: */
const struct efi_device_path END = {
	.type     = DEVICE_PATH_TYPE_END,
	.sub_type = DEVICE_PATH_SUB_TYPE_END,
	.length   = sizeof(END),
};

#if defined(CONFIG_MMC)
/*
 * Determine if an MMC device is an SD card.
 *
 * @desc	block device descriptor
 * Return:	true if the device is an SD card
 */
static bool is_sd(struct blk_desc *desc)
{
	struct mmc *mmc = find_mmc_device(desc->devnum);

	if (!mmc)
		return false;

	return IS_SD(mmc) != 0U;
}
#endif

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

/**
 * efi_dp_shorten() - shorten device-path
 *
 * When creating a short boot option we want to use a device-path that is
 * independent of the location where the block device is plugged in.
 *
 * UsbWwi() nodes contain a serial number, hard drive paths a partition
 * UUID. Both should be unique.
 *
 * See UEFI spec, section 3.1.2 for "short-form device path".
 *
 * @dp:		original device-path
 * @Return:	shortened device-path or NULL
 */
struct efi_device_path *efi_dp_shorten(struct efi_device_path *dp)
{
	while (dp) {
		if (EFI_DP_TYPE(dp, MESSAGING_DEVICE, MSG_USB_WWI) ||
		    EFI_DP_TYPE(dp, MEDIA_DEVICE, HARD_DRIVE_PATH) ||
		    EFI_DP_TYPE(dp, MEDIA_DEVICE, FILE_PATH))
			return dp;

		dp = efi_dp_next(dp);
	}

	return dp;
}

/**
 * find_handle() - find handle by device path and installed protocol
 *
 * If @rem is provided, the handle with the longest partial match is returned.
 *
 * @dp:		device path to search
 * @guid:	GUID of protocol that must be installed on path or NULL
 * @short_path:	use short form device path for matching
 * @rem:	pointer to receive remaining device path
 * Return:	matching handle
 */
static efi_handle_t find_handle(struct efi_device_path *dp,
				const efi_guid_t *guid, bool short_path,
				struct efi_device_path **rem)
{
	efi_handle_t handle, best_handle = NULL;
	efi_uintn_t len, best_len = 0;

	len = efi_dp_instance_size(dp);

	list_for_each_entry(handle, &efi_obj_list, link) {
		struct efi_handler *handler;
		struct efi_device_path *dp_current;
		efi_uintn_t len_current;
		efi_status_t ret;

		if (guid) {
			ret = efi_search_protocol(handle, guid, &handler);
			if (ret != EFI_SUCCESS)
				continue;
		}
		ret = efi_search_protocol(handle, &efi_guid_device_path,
					  &handler);
		if (ret != EFI_SUCCESS)
			continue;
		dp_current = handler->protocol_interface;
		if (short_path) {
			dp_current = efi_dp_shorten(dp_current);
			if (!dp_current)
				continue;
		}
		len_current = efi_dp_instance_size(dp_current);
		if (rem) {
			if (len_current > len)
				continue;
		} else {
			if (len_current != len)
				continue;
		}
		if (memcmp(dp_current, dp, len_current))
			continue;
		if (!rem)
			return handle;
		if (len_current > best_len) {
			best_len = len_current;
			best_handle = handle;
			*rem = (void*)((u8 *)dp + len_current);
		}
	}
	return best_handle;
}

/**
 * efi_dp_find_obj() - find handle by device path
 *
 * If @rem is provided, the handle with the longest partial match is returned.
 *
 * @dp:		device path to search
 * @guid:	GUID of protocol that must be installed on path or NULL
 * @rem:	pointer to receive remaining device path
 * Return:	matching handle
 */
efi_handle_t efi_dp_find_obj(struct efi_device_path *dp,
			     const efi_guid_t *guid,
			     struct efi_device_path **rem)
{
	efi_handle_t handle;

	handle = find_handle(dp, guid, false, rem);
	if (!handle)
		/* Match short form device path */
		handle = find_handle(dp, guid, true, rem);

	return handle;
}

/*
 * Determine the last device path node that is not the end node.
 *
 * @dp		device path
 * Return:	last node before the end node if it exists
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

	ndp = efi_alloc(sz);
	if (!ndp)
		return NULL;
	memcpy(ndp, dp, sz);

	return ndp;
}

/**
 * efi_dp_concat() - Concatenate two device paths and add and terminate them
 *                   with an end node.
 *
 * @dp1:	    First device path
 * @dp2:	    Second device path
 * @split_end_node: If true the two device paths will be concatenated and
 *                  separated by an end node (DEVICE_PATH_SUB_TYPE_END).
 *		    If false the second device path will be concatenated to the
 *		    first one as-is.
 *
 * Return:
 * concatenated device path or NULL. Caller must free the returned value
 */
struct
efi_device_path *efi_dp_concat(const struct efi_device_path *dp1,
			       const struct efi_device_path *dp2,
			       bool split_end_node)
{
	struct efi_device_path *ret;
	size_t end_size;

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
		void *p;

		if (split_end_node)
			end_size = 2 * sizeof(END);
		else
			end_size = sizeof(END);
		p = efi_alloc(sz1 + sz2 + end_size);
		if (!p)
			return NULL;
		ret = p;
		memcpy(p, dp1, sz1);
		p += sz1;

		if (split_end_node) {
			memcpy(p, &END, sizeof(END));
			p += sizeof(END);
		}

		/* the end node of the second device path has to be retained */
		memcpy(p, dp2, sz2);
		p += sz2;
		memcpy(p, &END, sizeof(END));
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
		void *p = efi_alloc(sz + sizeof(END));
		if (!p)
			return NULL;
		memcpy(p, node, sz);
		memcpy(p + sz, &END, sizeof(END));
		ret = p;
	} else {
		/* both dp and node are non-null */
		size_t sz = efi_dp_size(dp);
		void *p = efi_alloc(sz + node->length + sizeof(END));
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

	ret = efi_alloc(length);
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
	p = efi_alloc(sz + szi + 2 * sizeof(END));
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
	p = efi_alloc(sz + sizeof(END));
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

/* size of device-path not including END node for device and all parents
 * up to the root device.
 */
__maybe_unused static unsigned int dp_size(struct udevice *dev)
{
	if (!dev || !dev->driver)
		return sizeof(struct efi_device_path_udevice);

	switch (device_get_uclass_id(dev)) {
	case UCLASS_ROOT:
		/* stop traversing parents at this point: */
		return sizeof(struct efi_device_path_udevice);
	case UCLASS_ETH:
		return dp_size(dev->parent) +
			sizeof(struct efi_device_path_mac_addr);
	case UCLASS_BLK:
		switch (dev->parent->uclass->uc_drv->id) {
#ifdef CONFIG_IDE
		case UCLASS_IDE:
			return dp_size(dev->parent) +
				sizeof(struct efi_device_path_atapi);
#endif
#if defined(CONFIG_SCSI)
		case UCLASS_SCSI:
			return dp_size(dev->parent) +
				sizeof(struct efi_device_path_scsi);
#endif
#if defined(CONFIG_MMC)
		case UCLASS_MMC:
			return dp_size(dev->parent) +
				sizeof(struct efi_device_path_sd_mmc_path);
#endif
#if defined(CONFIG_AHCI) || defined(CONFIG_SATA)
		case UCLASS_AHCI:
			return dp_size(dev->parent) +
				sizeof(struct efi_device_path_sata);
#endif
#if defined(CONFIG_NVME)
		case UCLASS_NVME:
			return dp_size(dev->parent) +
				sizeof(struct efi_device_path_nvme);
#endif
#ifdef CONFIG_USB
		case UCLASS_MASS_STORAGE:
			return dp_size(dev->parent)
				+ sizeof(struct efi_device_path_controller);
#endif
		default:
			/* UCLASS_BLKMAP, UCLASS_HOST, UCLASS_VIRTIO */
			return dp_size(dev->parent) +
				sizeof(struct efi_device_path_udevice);
		}
#if defined(CONFIG_MMC)
	case UCLASS_MMC:
		return dp_size(dev->parent) +
			sizeof(struct efi_device_path_sd_mmc_path);
#endif
	case UCLASS_MASS_STORAGE:
	case UCLASS_USB_HUB:
		return dp_size(dev->parent) +
			sizeof(struct efi_device_path_usb);
	default:
		return dp_size(dev->parent) +
			sizeof(struct efi_device_path_udevice);
	}
}

/*
 * Recursively build a device path.
 *
 * @buf		pointer to the end of the device path
 * @dev		device
 * Return:	pointer to the end of the device path
 */
__maybe_unused static void *dp_fill(void *buf, struct udevice *dev)
{
	enum uclass_id uclass_id;

	if (!dev || !dev->driver)
		return buf;

	uclass_id = device_get_uclass_id(dev);
	if (uclass_id != UCLASS_ROOT)
		buf = dp_fill(buf, dev->parent);

	switch (uclass_id) {
#ifdef CONFIG_NETDEVICES
	case UCLASS_ETH: {
		struct efi_device_path_mac_addr *dp = buf;
		struct eth_pdata *pdata = dev_get_plat(dev);

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
	case UCLASS_BLK:
		switch (device_get_uclass_id(dev->parent)) {
#ifdef CONFIG_IDE
		case UCLASS_IDE: {
			struct efi_device_path_atapi *dp = buf;
			struct blk_desc *desc = dev_get_uclass_plat(dev);

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
#if defined(CONFIG_SCSI)
		case UCLASS_SCSI: {
			struct efi_device_path_scsi *dp = buf;
			struct blk_desc *desc = dev_get_uclass_plat(dev);

			dp->dp.type = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
			dp->dp.sub_type = DEVICE_PATH_SUB_TYPE_MSG_SCSI;
			dp->dp.length = sizeof(*dp);
			dp->logical_unit_number = desc->lun;
			dp->target_id = desc->target;
			return &dp[1];
			}
#endif
#if defined(CONFIG_MMC)
		case UCLASS_MMC: {
			struct efi_device_path_sd_mmc_path *sddp = buf;
			struct blk_desc *desc = dev_get_uclass_plat(dev);

			sddp->dp.type     = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
			sddp->dp.sub_type = is_sd(desc) ?
				DEVICE_PATH_SUB_TYPE_MSG_SD :
				DEVICE_PATH_SUB_TYPE_MSG_MMC;
			sddp->dp.length   = sizeof(*sddp);
			sddp->slot_number = dev_seq(dev);
			return &sddp[1];
			}
#endif
#if defined(CONFIG_AHCI) || defined(CONFIG_SATA)
		case UCLASS_AHCI: {
			struct efi_device_path_sata *dp = buf;
			struct blk_desc *desc = dev_get_uclass_plat(dev);

			dp->dp.type     = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
			dp->dp.sub_type = DEVICE_PATH_SUB_TYPE_MSG_SATA;
			dp->dp.length   = sizeof(*dp);
			dp->hba_port = desc->devnum;
			/* default 0xffff implies no port multiplier */
			dp->port_multiplier_port = 0xffff;
			dp->logical_unit_number = desc->lun;
			return &dp[1];
			}
#endif
#if defined(CONFIG_NVME)
		case UCLASS_NVME: {
			struct efi_device_path_nvme *dp = buf;
			u32 ns_id;

			dp->dp.type     = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
			dp->dp.sub_type = DEVICE_PATH_SUB_TYPE_MSG_NVME;
			dp->dp.length   = sizeof(*dp);
			nvme_get_namespace_id(dev, &ns_id, dp->eui64);
			memcpy(&dp->ns_id, &ns_id, sizeof(ns_id));
			return &dp[1];
			}
#endif
#if defined(CONFIG_USB)
		case UCLASS_MASS_STORAGE: {
			struct blk_desc *desc = dev_get_uclass_plat(dev);
			struct efi_device_path_controller *dp = buf;

			dp->dp.type	= DEVICE_PATH_TYPE_HARDWARE_DEVICE;
			dp->dp.sub_type = DEVICE_PATH_SUB_TYPE_CONTROLLER;
			dp->dp.length	= sizeof(*dp);
			dp->controller_number = desc->lun;
			return &dp[1];
		}
#endif
		default: {
			/* UCLASS_BLKMAP, UCLASS_HOST, UCLASS_VIRTIO */
			struct efi_device_path_udevice *dp = buf;
			struct blk_desc *desc = dev_get_uclass_plat(dev);

			dp->dp.type = DEVICE_PATH_TYPE_HARDWARE_DEVICE;
			dp->dp.sub_type = DEVICE_PATH_SUB_TYPE_VENDOR;
			dp->dp.length = sizeof(*dp);
			memcpy(&dp->guid, &efi_u_boot_guid,
			       sizeof(efi_guid_t));
			dp->uclass_id = (UCLASS_BLK & 0xffff) |
					(desc->uclass_id << 16);
			dp->dev_number = desc->devnum;

			return &dp[1];
		}
	}
#if defined(CONFIG_MMC)
	case UCLASS_MMC: {
		struct efi_device_path_sd_mmc_path *sddp = buf;
		struct mmc *mmc = mmc_get_mmc_dev(dev);
		struct blk_desc *desc = mmc_get_blk_desc(mmc);

		sddp->dp.type     = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
		sddp->dp.sub_type = is_sd(desc) ?
			DEVICE_PATH_SUB_TYPE_MSG_SD :
			DEVICE_PATH_SUB_TYPE_MSG_MMC;
		sddp->dp.length   = sizeof(*sddp);
		sddp->slot_number = dev_seq(dev);

		return &sddp[1];
	}
#endif
	case UCLASS_MASS_STORAGE:
	case UCLASS_USB_HUB: {
		struct efi_device_path_usb *udp = buf;

		switch (device_get_uclass_id(dev->parent)) {
		case UCLASS_USB_HUB: {
			struct usb_device *udev = dev_get_parent_priv(dev);

			udp->parent_port_number = udev->portnr;
			break;
		}
		default:
			udp->parent_port_number = 0;
		}
		udp->dp.type     = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
		udp->dp.sub_type = DEVICE_PATH_SUB_TYPE_MSG_USB;
		udp->dp.length   = sizeof(*udp);
		udp->usb_interface = 0;

		return &udp[1];
	}
	default: {
		struct efi_device_path_udevice *vdp = buf;

		vdp->dp.type = DEVICE_PATH_TYPE_HARDWARE_DEVICE;
		vdp->dp.sub_type = DEVICE_PATH_SUB_TYPE_VENDOR;
		vdp->dp.length = sizeof(*vdp);
		memcpy(&vdp->guid, &efi_u_boot_guid, sizeof(efi_guid_t));
		vdp->uclass_id = uclass_id;
		vdp->dev_number = dev->seq_;

		return &vdp[1];
	    }
	}
}

static unsigned dp_part_size(struct blk_desc *desc, int part)
{
	unsigned dpsize;
	struct udevice *dev = desc->bdev;

	dpsize = dp_size(dev);

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
 *
 * Return:	pointer to position after the node
 */
static void *dp_part_node(void *buf, struct blk_desc *desc, int part)
{
	struct disk_partition info;
	int ret;

	ret = part_get_info(desc, part, &info);
	if (ret < 0)
		return buf;

	if (desc->part_type == PART_TYPE_ISO) {
		struct efi_device_path_cdrom_path *cddp = buf;

		cddp->boot_entry = part;
		cddp->dp.type = DEVICE_PATH_TYPE_MEDIA_DEVICE;
		cddp->dp.sub_type = DEVICE_PATH_SUB_TYPE_CDROM_PATH;
		cddp->dp.length = sizeof(*cddp);
		cddp->partition_start = info.start;
		cddp->partition_size = info.size;

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
#if CONFIG_IS_ENABLED(PARTITION_UUIDS)
			/* info.uuid exists only with PARTITION_UUIDS */
			if (uuid_str_to_bin(info.uuid,
					    hddp->partition_signature,
					    UUID_STR_FORMAT_GUID)) {
				log_warning(
					"Partition %d: invalid GUID %s\n",
					part, info.uuid);
			}
#endif
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
	struct udevice *dev = desc->bdev;

	buf = dp_fill(buf, dev);

	if (part == 0) /* the actual disk, not a partition */
		return buf;

	return dp_part_node(buf, desc, part);
}

/* Construct a device-path from a partition on a block device: */
struct efi_device_path *efi_dp_from_part(struct blk_desc *desc, int part)
{
	void *buf, *start;

	start = buf = efi_alloc(dp_part_size(desc, part) + sizeof(END));
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
	buf = efi_alloc(dpsize);

	if (buf)
		dp_part_node(buf, desc, part);

	return buf;
}

/**
 * path_to_uefi() - convert UTF-8 path to an UEFI style path
 *
 * Convert UTF-8 path to a UEFI style path (i.e. with backslashes as path
 * separators and UTF-16).
 *
 * @src:	source buffer
 * @uefi:	target buffer, possibly unaligned
 */
static void path_to_uefi(void *uefi, const char *src)
{
	u16 *pos = uefi;

	/*
	 * efi_set_bootdev() calls this routine indirectly before the UEFI
	 * subsystem is initialized. So we cannot assume unaligned access to be
	 * enabled.
	 */
	allow_unaligned();

	while (*src) {
		s32 code = utf8_get(&src);

		if (code < 0)
			code = '?';
		else if (code == '/')
			code = '\\';
		utf16_put(code, &pos);
	}
	*pos = 0;
}

/**
 * efi_dp_from_file() - append file path node to device path.
 *
 * @dp:		device path or NULL
 * @path:	file path or NULL
 * Return:	device path or NULL in case of an error
 */
struct efi_device_path *efi_dp_from_file(const struct efi_device_path *dp,
					 const char *path)
{
	struct efi_device_path_file_path *fp;
	void *buf, *pos;
	size_t dpsize, fpsize;

	dpsize = efi_dp_size(dp);
	fpsize = sizeof(struct efi_device_path) +
		 2 * (utf8_utf16_strlen(path) + 1);
	if (fpsize > U16_MAX)
		return NULL;

	buf = efi_alloc(dpsize + fpsize + sizeof(END));
	if (!buf)
		return NULL;

	memcpy(buf, dp, dpsize);
	pos = buf + dpsize;

	/* add file-path: */
	if (*path) {
		fp = pos;
		fp->dp.type = DEVICE_PATH_TYPE_MEDIA_DEVICE;
		fp->dp.sub_type = DEVICE_PATH_SUB_TYPE_FILE_PATH;
		fp->dp.length = (u16)fpsize;
		path_to_uefi(fp->str, path);
		pos += fpsize;
	}

	memcpy(pos, &END, sizeof(END));

	return buf;
}

struct efi_device_path *efi_dp_from_uart(void)
{
	void *buf, *pos;
	struct efi_device_path_uart *uart;
	size_t dpsize = dp_size(dm_root()) + sizeof(*uart) + sizeof(END);

	buf = efi_alloc(dpsize);
	if (!buf)
		return NULL;
	pos = dp_fill(buf, dm_root());
	uart = pos;
	uart->dp.type = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
	uart->dp.sub_type = DEVICE_PATH_SUB_TYPE_MSG_UART;
	uart->dp.length = sizeof(*uart);
	pos += sizeof(*uart);
	memcpy(pos, &END, sizeof(END));

	return buf;
}

struct efi_device_path __maybe_unused *efi_dp_from_eth(void)
{
	void *buf, *start;
	unsigned dpsize = 0;

	assert(eth_get_dev());

	dpsize += dp_size(eth_get_dev());

	start = buf = efi_alloc(dpsize + sizeof(END));
	if (!buf)
		return NULL;

	buf = dp_fill(buf, eth_get_dev());

	*((struct efi_device_path *)buf) = END;

	return start;
}

/* Construct a device-path for memory-mapped image */
struct efi_device_path *efi_dp_from_mem(uint32_t memory_type,
					uint64_t start_address,
					uint64_t end_address)
{
	struct efi_device_path_memory *mdp;
	void *buf, *start;

	start = buf = efi_alloc(sizeof(*mdp) + sizeof(END));
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

/**
 * efi_dp_from_name() - convert U-Boot device and file path to device path
 *
 * @dev:	U-Boot device, e.g. 'mmc'
 * @devnr:	U-Boot device number, e.g. 1 for 'mmc:1'
 * @path:	file path relative to U-Boot device, may be NULL
 * @device:	pointer to receive device path of the device
 * @file:	pointer to receive device path for the file
 * Return:	status code
 */
efi_status_t efi_dp_from_name(const char *dev, const char *devnr,
			      const char *path,
			      struct efi_device_path **device,
			      struct efi_device_path **file)
{
	struct blk_desc *desc = NULL;
	struct efi_device_path *dp;
	struct disk_partition fs_partition;
	size_t image_size;
	void *image_addr;
	int part = 0;

	if (path && !file)
		return EFI_INVALID_PARAMETER;

	if (IS_ENABLED(CONFIG_EFI_BINARY_EXEC) &&
	    (!strcmp(dev, "Mem") || !strcmp(dev, "hostfs")))  {
		/* loadm command and semihosting */
		efi_get_image_parameters(&image_addr, &image_size);

		dp = efi_dp_from_mem(EFI_RESERVED_MEMORY_TYPE,
				     (uintptr_t)image_addr, image_size);
	} else if (IS_ENABLED(CONFIG_NETDEVICES) && !strcmp(dev, "Net")) {
		dp = efi_dp_from_eth();
	} else if (!strcmp(dev, "Uart")) {
		dp = efi_dp_from_uart();
	} else {
		part = blk_get_device_part_str(dev, devnr, &desc, &fs_partition,
					       1);
		if (part < 0 || !desc)
			return EFI_INVALID_PARAMETER;

		dp = efi_dp_from_part(desc, part);
	}
	if (device)
		*device = dp;

	if (!path)
		return EFI_SUCCESS;

	*file = efi_dp_from_file(dp, path);
	if (!*file)
		return EFI_OUT_OF_RESOURCES;

	return EFI_SUCCESS;
}

/**
 * efi_dp_check_length() - check length of a device path
 *
 * @dp:		pointer to device path
 * @maxlen:	maximum length of the device path
 * Return:
 * * length of the device path if it is less or equal @maxlen
 * * -1 if the device path is longer then @maxlen
 * * -1 if a device path node has a length of less than 4
 * * -EINVAL if maxlen exceeds SSIZE_MAX
 */
ssize_t efi_dp_check_length(const struct efi_device_path *dp,
			    const size_t maxlen)
{
	ssize_t ret = 0;
	u16 len;

	if (maxlen > SSIZE_MAX)
		return -EINVAL;
	for (;;) {
		len = dp->length;
		if (len < 4)
			return -1;
		ret += len;
		if (ret > maxlen)
			return -1;
		if (dp->type == DEVICE_PATH_TYPE_END &&
		    dp->sub_type == DEVICE_PATH_SUB_TYPE_END)
			return ret;
		dp = (const struct efi_device_path *)((const u8 *)dp + len);
	}
}

/**
 * efi_dp_from_lo() - Get the instance of a VenMedia node in a
 *                    multi-instance device path that matches
 *                    a specific GUID. This kind of device paths
 *                    is found in Boot#### options describing an
 *                    initrd location
 *
 * @lo:		EFI_LOAD_OPTION containing a valid device path
 * @guid:	guid to search for
 *
 * Return:
 * device path including the VenMedia node or NULL.
 * Caller must free the returned value.
 */
struct
efi_device_path *efi_dp_from_lo(struct efi_load_option *lo,
				const efi_guid_t *guid)
{
	struct efi_device_path *fp = lo->file_path;
	struct efi_device_path_vendor *vendor;
	int lo_len = lo->file_path_length;

	for (; lo_len >=  sizeof(struct efi_device_path);
	     lo_len -= fp->length, fp = (void *)fp + fp->length) {
		if (lo_len < 0 || efi_dp_check_length(fp, lo_len) < 0)
			break;
		if (fp->type != DEVICE_PATH_TYPE_MEDIA_DEVICE ||
		    fp->sub_type != DEVICE_PATH_SUB_TYPE_VENDOR_PATH)
			continue;

		vendor = (struct efi_device_path_vendor *)fp;
		if (!guidcmp(&vendor->guid, guid))
			return efi_dp_dup(efi_dp_next(fp));
	}
	log_debug("VenMedia(%pUl) not found in %ls\n", &guid, lo->label);

	return NULL;
}

/**
 * search_gpt_dp_node() - search gpt device path node
 *
 * @device_path:	device path
 *
 * Return:	pointer to the gpt device path node
 */
struct efi_device_path *search_gpt_dp_node(struct efi_device_path *device_path)
{
	struct efi_device_path *dp = device_path;

	while (dp) {
		if (dp->type == DEVICE_PATH_TYPE_MEDIA_DEVICE &&
		    dp->sub_type == DEVICE_PATH_SUB_TYPE_HARD_DRIVE_PATH) {
			struct efi_device_path_hard_drive_path *hd_dp =
				(struct efi_device_path_hard_drive_path *)dp;

			if (hd_dp->partmap_type == PART_FORMAT_GPT &&
			    hd_dp->signature_type == SIG_TYPE_GUID)
				return dp;
		}
		dp = efi_dp_next(dp);
	}

	return NULL;
}
