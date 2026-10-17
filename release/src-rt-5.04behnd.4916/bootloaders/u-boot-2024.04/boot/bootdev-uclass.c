// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <dm.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <fs.h>
#include <log.h>
#include <malloc.h>
#include <part.h>
#include <sort.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>

enum {
	/*
	 * Set some sort of limit on the number of partitions a bootdev can
	 * have. Note that for disks this limits the partitions numbers that
	 * are scanned to 1..MAX_BOOTFLOWS_PER_BOOTDEV
	 */
	MAX_PART_PER_BOOTDEV	= 30,

	/* Maximum supported length of the "boot_targets" env string */
	BOOT_TARGETS_MAX_LEN	= 100,
};

int bootdev_add_bootflow(struct bootflow *bflow)
{
	struct bootstd_priv *std;
	struct bootflow *new;
	int ret;

	assert(bflow->dev);
	ret = bootstd_get_priv(&std);
	if (ret)
		return ret;

	new = malloc(sizeof(*bflow));
	if (!new)
		return log_msg_ret("bflow", -ENOMEM);
	memcpy(new, bflow, sizeof(*bflow));

	list_add_tail(&new->glob_node, &std->glob_head);
	if (bflow->dev) {
		struct bootdev_uc_plat *ucp = dev_get_uclass_plat(bflow->dev);

		list_add_tail(&new->bm_node, &ucp->bootflow_head);
	}

	return 0;
}

int bootdev_first_bootflow(struct udevice *dev, struct bootflow **bflowp)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	if (list_empty(&ucp->bootflow_head))
		return -ENOENT;

	*bflowp = list_first_entry(&ucp->bootflow_head, struct bootflow,
				   bm_node);

	return 0;
}

int bootdev_next_bootflow(struct bootflow **bflowp)
{
	struct bootflow *bflow = *bflowp;
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(bflow->dev);

	*bflowp = NULL;

	if (list_is_last(&bflow->bm_node, &ucp->bootflow_head))
		return -ENOENT;

	*bflowp = list_entry(bflow->bm_node.next, struct bootflow, bm_node);

	return 0;
}

int bootdev_bind(struct udevice *parent, const char *drv_name, const char *name,
		 struct udevice **devp)
{
	struct udevice *dev;
	char dev_name[30];
	char *str;
	int ret;

	snprintf(dev_name, sizeof(dev_name), "%s.%s", parent->name, name);
	str = strdup(dev_name);
	if (!str)
		return -ENOMEM;
	ret = device_bind_driver(parent, drv_name, str, &dev);
	if (ret)
		return ret;
	device_set_name_alloced(dev);
	*devp = dev;

	return 0;
}

int bootdev_find_in_blk(struct udevice *dev, struct udevice *blk,
			struct bootflow_iter *iter, struct bootflow *bflow)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(bflow->method);
	bool allow_any_part = plat->flags & BOOTMETHF_ANY_PART;
	struct blk_desc *desc = dev_get_uclass_plat(blk);
	struct disk_partition info;
	char partstr[20];
	char name[60];
	int ret;

	/* Sanity check */
	if (iter->part >= MAX_PART_PER_BOOTDEV)
		return log_msg_ret("max", -ESHUTDOWN);

	bflow->blk = blk;
	if (iter->part)
		snprintf(partstr, sizeof(partstr), "part_%x", iter->part);
	else
		strcpy(partstr, "whole");
	snprintf(name, sizeof(name), "%s.%s", dev->name, partstr);
	bflow->name = strdup(name);
	if (!bflow->name)
		return log_msg_ret("name", -ENOMEM);

	bflow->part = iter->part;

	ret = bootmeth_check(bflow->method, iter);
	if (ret)
		return log_msg_ret("check", ret);

	/*
	 * partition numbers start at 0 so this cannot succeed, but it can tell
	 * us whether there is valid media there
	 */
	ret = part_get_info(desc, iter->part, &info);
	log_debug("part_get_info() returned %d\n", ret);
	if (!iter->part && ret == -ENOENT)
		ret = 0;

	/*
	 * This error indicates the media is not present. Otherwise we just
	 * blindly scan the next partition. We could be more intelligent here
	 * and check which partition numbers actually exist.
	 */
	if (ret == -EOPNOTSUPP)
		ret = -ESHUTDOWN;
	else
		bflow->state = BOOTFLOWST_MEDIA;
	if (ret && !allow_any_part) {
		/* allow partition 1 to be missing */
		if (iter->part == 1) {
			iter->max_part = 3;
			ret = -ENOENT;
		}

		return log_msg_ret("part", ret);
	}

	/*
	 * Currently we don't get the number of partitions, so just
	 * assume a large number
	 */
	iter->max_part = MAX_PART_PER_BOOTDEV;

	/* If this is the whole disk, check if we have bootable partitions */
	if (!iter->part) {
		iter->first_bootable = part_get_bootable(desc);
		log_debug("checking bootable=%d\n", iter->first_bootable);
	} else if (allow_any_part) {
		/*
		 * allow any partition to be scanned, by skipping any checks
		 * for filesystems or partition contents on this disk
		 */

	/* if there are bootable partitions, scan only those */
	} else if (iter->first_bootable >= 0 &&
		   (iter->first_bootable ? !info.bootable : iter->part != 1)) {
		return log_msg_ret("boot", -EINVAL);
	} else {
		ret = fs_set_blk_dev_with_part(desc, bflow->part);
		bflow->state = BOOTFLOWST_PART;
		if (ret)
			return log_msg_ret("fs", ret);

		log_debug("%s: Found partition %x type %x fstype %d\n",
			  blk->name, bflow->part,
			  IS_ENABLED(CONFIG_DOS_PARTITION) ?
			  disk_partition_sys_ind(&info) : 0,
			  ret ? -1 : fs_get_type());
		bflow->blk = blk;
		bflow->state = BOOTFLOWST_FS;
	}

	log_debug("method %s\n", bflow->method->name);
	ret = bootmeth_read_bootflow(bflow->method, bflow);
	if (ret)
		return log_msg_ret("method", ret);

	return 0;
}

void bootdev_list(bool probe)
{
	struct udevice *dev;
	int ret;
	int i;

	printf("Seq  Probed  Status  Uclass    Name\n");
	printf("---  ------  ------  --------  ------------------\n");
	if (probe)
		ret = uclass_first_device_check(UCLASS_BOOTDEV, &dev);
	else
		ret = uclass_find_first_device(UCLASS_BOOTDEV, &dev);
	for (i = 0; dev; i++) {
		printf("%3x   [ %c ]  %6s  %-9.9s %s\n", dev_seq(dev),
		       device_active(dev) ? '+' : ' ',
		       ret ? simple_itoa(-ret) : "OK",
		       dev_get_uclass_name(dev_get_parent(dev)), dev->name);
		if (probe)
			ret = uclass_next_device_check(&dev);
		else
			ret = uclass_find_next_device(&dev);
	}
	printf("---  ------  ------  --------  ------------------\n");
	printf("(%d bootdev%s)\n", i, i != 1 ? "s" : "");
}

int bootdev_setup_for_dev(struct udevice *parent, const char *drv_name)
{
	struct udevice *bdev;
	int ret;

	ret = device_find_first_child_by_uclass(parent, UCLASS_BOOTDEV,
						&bdev);
	if (ret) {
		if (ret != -ENODEV) {
			log_debug("Cannot access bootdev device\n");
			return ret;
		}

		ret = bootdev_bind(parent, drv_name, "bootdev", &bdev);
		if (ret) {
			log_debug("Cannot create bootdev device\n");
			return ret;
		}
	}

	return 0;
}

static int bootdev_get_suffix_start(struct udevice *dev, const char *suffix)
{
	int len, slen;

	len = strlen(dev->name);
	slen = strlen(suffix);
	if (len > slen && !strcmp(suffix, dev->name + len - slen))
		return len - slen;

	return len;
}

int bootdev_setup_for_sibling_blk(struct udevice *blk, const char *drv_name)
{
	struct udevice *parent, *dev;
	char dev_name[50];
	int ret, len;

	len = bootdev_get_suffix_start(blk, ".blk");
	snprintf(dev_name, sizeof(dev_name), "%.*s.%s", len, blk->name,
		 "bootdev");

	parent = dev_get_parent(blk);
	ret = device_find_child_by_name(parent, dev_name, &dev);
	if (ret) {
		char *str;

		if (ret != -ENODEV) {
			log_debug("Cannot access bootdev device\n");
			return ret;
		}
		str = strdup(dev_name);
		if (!str)
			return -ENOMEM;

		ret = device_bind_driver(parent, drv_name, str, &dev);
		if (ret) {
			log_debug("Cannot create bootdev device\n");
			return ret;
		}
		device_set_name_alloced(dev);
	}

	return 0;
}

int bootdev_get_sibling_blk(struct udevice *dev, struct udevice **blkp)
{
	struct udevice *parent = dev_get_parent(dev);
	struct udevice *blk;
	int ret, len;

	if (device_get_uclass_id(dev) != UCLASS_BOOTDEV)
		return -EINVAL;

	/*
	 * This should always work if bootdev_setup_for_sibling_blk() was used
	 */
	len = bootdev_get_suffix_start(dev, ".bootdev");
	ret = device_find_child_by_namelen(parent, dev->name, len, &blk);
	if (ret) {
		char dev_name[50];

		snprintf(dev_name, sizeof(dev_name), "%.*s.blk", len,
			 dev->name);
		ret = device_find_child_by_name(parent, dev_name, &blk);
		if (ret)
			return log_msg_ret("find", ret);
	}
	ret = device_probe(blk);
	if (ret)
		return log_msg_ret("act", ret);
	*blkp = blk;

	return 0;
}

static int bootdev_get_from_blk(struct udevice *blk, struct udevice **bootdevp)
{
	struct udevice *parent = dev_get_parent(blk);
	struct udevice *bootdev;
	char dev_name[50];
	int ret, len;

	if (device_get_uclass_id(blk) != UCLASS_BLK)
		return -EINVAL;

	/* This should always work if bootdev_setup_for_sibling_blk() was used */
	len = bootdev_get_suffix_start(blk, ".blk");
	snprintf(dev_name, sizeof(dev_name), "%.*s.%s", len, blk->name,
		 "bootdev");
	ret = device_find_child_by_name(parent, dev_name, &bootdev);
	if (ret)
		return log_msg_ret("find", ret);
	*bootdevp = bootdev;

	return 0;
}

int bootdev_unbind_dev(struct udevice *parent)
{
	struct udevice *dev;
	int ret;

	ret = device_find_first_child_by_uclass(parent, UCLASS_BOOTDEV, &dev);
	if (!ret) {
		ret = device_remove(dev, DM_REMOVE_NORMAL);
		if (ret)
			return log_msg_ret("rem", ret);
		ret = device_unbind(dev);
		if (ret)
			return log_msg_ret("unb", ret);
	}

	return 0;
}

/**
 * label_to_uclass() - Convert a label to a uclass and sequence number
 *
 * @label: Label to look up (e.g. "mmc1" or "mmc0")
 * @seqp: Returns the sequence number, or -1 if none
 * @method_flagsp: If non-NULL, returns any flags implied by the label
 * (enum bootflow_meth_flags_t), 0 if none
 * Returns: sequence number on success, -EPFNOSUPPORT is the uclass is not
 * known, other -ve error code on other error
 */
static int label_to_uclass(const char *label, int *seqp, int *method_flagsp)
{
	int seq, len, method_flags;
	enum uclass_id id;
	const char *end;

	method_flags = 0;
	seq = trailing_strtoln_end(label, NULL, &end);
	len = end - label;
	if (!len)
		return -EINVAL;
	id = uclass_get_by_namelen(label, len);
	log_debug("find %s: seq=%d, id=%d/%s\n", label, seq, id,
		  uclass_get_name(id));
	if (id == UCLASS_INVALID) {
		/* try some special cases */
		if (IS_ENABLED(CONFIG_BOOTDEV_SPI_FLASH) &&
		    !strncmp("spi", label, len)) {
			id = UCLASS_SPI_FLASH;
		} else if (IS_ENABLED(CONFIG_BOOTDEV_ETH) &&
		    !strncmp("pxe", label, len)) {
			id = UCLASS_ETH;
			method_flags |= BOOTFLOW_METHF_PXE_ONLY;
		} else if (IS_ENABLED(CONFIG_BOOTDEV_ETH) &&
		    !strncmp("dhcp", label, len)) {
			id = UCLASS_ETH;
			method_flags |= BOOTFLOW_METHF_DHCP_ONLY;
		} else {
			return -EPFNOSUPPORT;
		}
	}
	if (id == UCLASS_USB)
		id = UCLASS_MASS_STORAGE;
	*seqp = seq;
	if (method_flagsp)
		*method_flagsp = method_flags;

	return id;
}

int bootdev_find_by_label(const char *label, struct udevice **devp,
			  int *method_flagsp)
{
	int seq, ret, method_flags = 0;
	struct udevice *media;
	struct uclass *uc;
	enum uclass_id id;

	ret = label_to_uclass(label, &seq, &method_flags);
	if (ret < 0)
		return log_msg_ret("uc", ret);
	id = ret;

	/* Iterate through devices in the media uclass (e.g. UCLASS_MMC) */
	uclass_id_foreach_dev(id, media, uc) {
		struct udevice *bdev, *blk;
		int ret;

		/* if there is no seq, match anything */
		if (seq != -1 && dev_seq(media) != seq) {
			log_debug("- skip, media seq=%d\n", dev_seq(media));
			continue;
		}

		ret = device_find_first_child_by_uclass(media, UCLASS_BOOTDEV,
							&bdev);
		if (ret) {
			log_debug("- looking via blk, seq=%d, id=%d\n", seq,
				  id);
			ret = blk_find_device(id, seq, &blk);
			if (!ret) {
				log_debug("- get from blk %s\n", blk->name);
				ret = bootdev_get_from_blk(blk, &bdev);
			}
		}
		if (!ret) {
			log_debug("- found %s\n", bdev->name);
			*devp = bdev;

			/*
			 * if no sequence number was provided, we must scan all
			 * bootdevs for this media uclass
			 */
			if (seq == -1)
				method_flags |= BOOTFLOW_METHF_SINGLE_UCLASS;
			if (method_flagsp)
				*method_flagsp = method_flags;
			log_debug("method flags %x\n", method_flags);
			return 0;
		}
		log_debug("- no device in %s\n", media->name);
	}

	return -ENOENT;
}

int bootdev_find_by_any(const char *name, struct udevice **devp,
			int *method_flagsp)
{
	struct udevice *dev;
	int method_flags = 0;
	int ret = -ENODEV, seq;
	char *endp;

	seq = simple_strtol(name, &endp, 16);

	/* Select by name, label or number */
	if (*endp) {
		ret = uclass_get_device_by_name(UCLASS_BOOTDEV, name, &dev);
		if (ret == -ENODEV) {
			ret = bootdev_find_by_label(name, &dev, &method_flags);
			if (ret) {
				printf("Cannot find bootdev '%s' (err=%d)\n",
				       name, ret);
				return log_msg_ret("lab", ret);
			}
			ret = device_probe(dev);
		}
		if (ret) {
			printf("Cannot probe bootdev '%s' (err=%d)\n", name,
			       ret);
			return log_msg_ret("pro", ret);
		}
	} else if (IS_ENABLED(CONFIG_BOOTSTD_FULL)) {
		ret = uclass_get_device_by_seq(UCLASS_BOOTDEV, seq, &dev);
		method_flags |= BOOTFLOW_METHF_SINGLE_DEV;
	}
	if (ret) {
		printf("Cannot find '%s' (err=%d)\n", name, ret);
		return ret;
	}

	*devp = dev;
	if (method_flagsp)
		*method_flagsp = method_flags;

	return 0;
}

int bootdev_hunt_and_find_by_label(const char *label, struct udevice **devp,
				   int *method_flagsp)
{
	int ret;

	ret = bootdev_hunt(label, false);
	if (ret)
		return log_msg_ret("scn", ret);
	ret = bootdev_find_by_label(label, devp, method_flagsp);
	if (ret)
		return log_msg_ret("fnd", ret);

	return 0;
}

static int default_get_bootflow(struct udevice *dev, struct bootflow_iter *iter,
				struct bootflow *bflow)
{
	struct udevice *blk;
	int ret;

	ret = bootdev_get_sibling_blk(dev, &blk);
	log_debug("sibling_blk ret=%d, blk=%s\n", ret,
		  ret ? "(none)" : blk->name);
	/*
	 * If there is no media, indicate that no more partitions should be
	 * checked
	 */
	if (ret == -EOPNOTSUPP)
		ret = -ESHUTDOWN;
	if (ret)
		return log_msg_ret("blk", ret);
	assert(blk);
	ret = bootdev_find_in_blk(dev, blk, iter, bflow);
	if (ret)
		return log_msg_ret("find", ret);

	return 0;
}

int bootdev_get_bootflow(struct udevice *dev, struct bootflow_iter *iter,
			 struct bootflow *bflow)
{
	const struct bootdev_ops *ops = bootdev_get_ops(dev);

	log_debug("->get_bootflow %s,%x=%p\n", dev->name, iter->part,
		  ops->get_bootflow);
	bootflow_init(bflow, dev, iter->method);
	if (!ops->get_bootflow)
		return default_get_bootflow(dev, iter, bflow);

	return ops->get_bootflow(dev, iter, bflow);
}

void bootdev_clear_bootflows(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	while (!list_empty(&ucp->bootflow_head)) {
		struct bootflow *bflow;

		bflow = list_first_entry(&ucp->bootflow_head, struct bootflow,
					 bm_node);
		bootflow_remove(bflow);
	}
}

int bootdev_next_label(struct bootflow_iter *iter, struct udevice **devp,
		       int *method_flagsp)
{
	struct udevice *dev;

	log_debug("next\n");
	for (dev = NULL; !dev && iter->labels[++iter->cur_label];) {
		const char *label = iter->labels[iter->cur_label];
		int ret;

		log_debug("Scanning: %s\n", label);
		ret = bootdev_hunt_and_find_by_label(label, &dev,
						     method_flagsp);
		if (iter->flags & BOOTFLOWIF_SHOW) {
			if (ret == -EPFNOSUPPORT) {
				log_warning("Unknown uclass '%s' in label\n",
					    label);
			} else if (ret == -ENOENT) {
				/*
				 * looking for, e.g. 'scsi0' should find
				 * something if SCSI is present
				 */
				if (!trailing_strtol(label)) {
					log_warning("No bootdevs for '%s'\n",
						    label);
				}
			}
		}

	}

	if (!dev)
		return log_msg_ret("fin", -ENODEV);
	*devp = dev;

	return 0;
}

int bootdev_next_prio(struct bootflow_iter *iter, struct udevice **devp)
{
	struct udevice *dev = *devp, *last_dev = NULL;
	bool found;
	int ret;

	/* find the next device with this priority */
	*devp = NULL;
	log_debug("next prio %d: dev=%p/%s\n", iter->cur_prio, dev,
		  dev ? dev->name : "none");
	do {
		/*
		 * Don't probe devices here since they may not be of the
		 * required priority
		 */
		if (!dev)
			uclass_find_first_device(UCLASS_BOOTDEV, &dev);
		else
			uclass_find_next_device(&dev);
		found = false;

		/* scan for the next device with the correct priority */
		while (dev) {
			struct bootdev_uc_plat *plat;

			plat = dev_get_uclass_plat(dev);
			log_debug("- %s: %d, want %d\n", dev->name, plat->prio,
				  iter->cur_prio);
			if (plat->prio == iter->cur_prio)
				break;
			uclass_find_next_device(&dev);
		}

		/* none found for this priority, so move to the next */
		if (!dev) {
			log_debug("None found at prio %d, moving to %d\n",
				  iter->cur_prio, iter->cur_prio + 1);
			if (++iter->cur_prio == BOOTDEVP_COUNT)
				return log_msg_ret("fin", -ENODEV);

			if (iter->flags & BOOTFLOWIF_HUNT) {
				/* hunt to find new bootdevs */
				ret = bootdev_hunt_prio(iter->cur_prio,
							iter->flags &
							BOOTFLOWIF_SHOW);
				log_debug("- bootdev_hunt_prio() ret %d\n",
					  ret);
				if (ret)
					return log_msg_ret("hun", ret);
			}
		} else {
			ret = device_probe(dev);
			if (!ret)
				last_dev = dev;
			if (ret) {
				log_warning("Device '%s' failed to probe\n",
					  dev->name);
				if (last_dev == dev) {
					/*
					 * We have already tried this device
					 * and it failed to probe. Give up.
					 */
					return log_msg_ret("probe", ret);
				}
				last_dev = dev;
				dev = NULL;
			}
		}
	} while (!dev);

	*devp = dev;

	return 0;
}

int bootdev_setup_iter(struct bootflow_iter *iter, const char *label,
		       struct udevice **devp, int *method_flagsp)
{
	struct udevice *bootstd, *dev = NULL;
	bool show = iter->flags & BOOTFLOWIF_SHOW;
	int method_flags;
	int ret;

	ret = uclass_first_device_err(UCLASS_BOOTSTD, &bootstd);
	if (ret) {
		log_err("Missing bootstd device\n");
		return log_msg_ret("std", ret);
	}

	/* hunt for any pre-scan devices */
	if (iter->flags & BOOTFLOWIF_HUNT) {
		ret = bootdev_hunt_prio(BOOTDEVP_1_PRE_SCAN, show);
		log_debug("- bootdev_hunt_prio() ret %d\n", ret);
		if (ret)
			return log_msg_ret("pre", ret);
	}

	/* Handle scanning a single device */
	if (IS_ENABLED(CONFIG_BOOTSTD_FULL) && label) {
		if (iter->flags & BOOTFLOWIF_HUNT) {
			ret = bootdev_hunt(label, show);
			if (ret)
				return log_msg_ret("hun", ret);
		}
		ret = bootdev_find_by_any(label, &dev, &method_flags);
		if (ret)
			return log_msg_ret("lab", ret);

		log_debug("method_flags: %x\n", method_flags);
		if (method_flags & BOOTFLOW_METHF_SINGLE_UCLASS)
			iter->flags |= BOOTFLOWIF_SINGLE_UCLASS;
		else if (method_flags & BOOTFLOW_METHF_SINGLE_DEV)
			iter->flags |= BOOTFLOWIF_SINGLE_DEV;
		else
			iter->flags |= BOOTFLOWIF_SINGLE_MEDIA;
		log_debug("Selected label: %s, flags %x\n", label, iter->flags);
	} else {
		bool ok;

		/* This either returns a non-empty list or NULL */
		iter->labels = bootstd_get_bootdev_order(bootstd, &ok);
		if (!ok)
			return log_msg_ret("ord", -ENOMEM);
		log_debug("setup labels %p\n", iter->labels);
		if (iter->labels) {
			iter->cur_label = -1;
			ret = bootdev_next_label(iter, &dev, &method_flags);
		} else {
			ret = bootdev_next_prio(iter, &dev);
			method_flags = 0;
		}
		if (!dev)
			return log_msg_ret("fin", -ENOENT);
		log_debug("Selected bootdev: %s\n", dev->name);
	}

	ret = device_probe(dev);
	if (ret)
		return log_msg_ret("probe", ret);
	if (method_flagsp)
		*method_flagsp = method_flags;
	*devp = dev;

	return 0;
}

static int bootdev_hunt_drv(struct bootdev_hunter *info, uint seq, bool show)
{
	const char *name = uclass_get_name(info->uclass);
	struct bootstd_priv *std;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return log_msg_ret("std", ret);

	if (!(std->hunters_used & BIT(seq))) {
		if (show)
			printf("Hunting with: %s\n",
			       uclass_get_name(info->uclass));
		log_debug("Hunting with: %s\n", name);
		if (info->hunt) {
			ret = info->hunt(info, show);
			log_debug("  - hunt result %d\n", ret);
			if (ret && ret != -ENOENT)
				return ret;
		}
		std->hunters_used |= BIT(seq);
	}

	return 0;
}

int bootdev_hunt(const char *spec, bool show)
{
	struct bootdev_hunter *start;
	const char *end;
	int n_ent, i;
	int result;
	size_t len;

	start = ll_entry_start(struct bootdev_hunter, bootdev_hunter);
	n_ent = ll_entry_count(struct bootdev_hunter, bootdev_hunter);
	result = 0;

	len = SIZE_MAX;
	if (spec) {
		trailing_strtoln_end(spec, NULL, &end);
		len = end - spec;
	}

	for (i = 0; i < n_ent; i++) {
		struct bootdev_hunter *info = start + i;
		const char *name = uclass_get_name(info->uclass);
		int ret;

		log_debug("looking at %.*s for %s\n",
			  (int)max(strlen(name), len), spec, name);
		if (spec && strncmp(spec, name, max(strlen(name), len))) {
			if (info->uclass != UCLASS_ETH ||
			    (strcmp("dhcp", spec) && strcmp("pxe", spec)))
				continue;
		}
		ret = bootdev_hunt_drv(info, i, show);
		if (ret)
			result = ret;
	}

	return result;
}

int bootdev_unhunt(enum uclass_id id)
{
	struct bootdev_hunter *start;
	int n_ent, i;

	start = ll_entry_start(struct bootdev_hunter, bootdev_hunter);
	n_ent = ll_entry_count(struct bootdev_hunter, bootdev_hunter);
	for (i = 0; i < n_ent; i++) {
		struct bootdev_hunter *info = start + i;

		if (info->uclass == id) {
			struct bootstd_priv *std;
			int ret;

			ret = bootstd_get_priv(&std);
			if (ret)
				return log_msg_ret("std", ret);
			if (!(std->hunters_used & BIT(i)))
				return -EALREADY;
			std->hunters_used &= ~BIT(i);
			return 0;
		}
	}

	return -ENOENT;
}

int bootdev_hunt_prio(enum bootdev_prio_t prio, bool show)
{
	struct bootdev_hunter *start;
	int n_ent, i;
	int result;

	start = ll_entry_start(struct bootdev_hunter, bootdev_hunter);
	n_ent = ll_entry_count(struct bootdev_hunter, bootdev_hunter);
	result = 0;

	log_debug("Hunting for priority %d\n", prio);
	for (i = 0; i < n_ent; i++) {
		struct bootdev_hunter *info = start + i;
		int ret;

		if (prio != info->prio)
			continue;
		ret = bootdev_hunt_drv(info, i, show);
		log_debug("bootdev_hunt_drv() return %d\n", ret);
		if (ret && ret != -ENOENT)
			result = ret;
	}
	log_debug("exit %d\n", result);

	return result;
}

void bootdev_list_hunters(struct bootstd_priv *std)
{
	struct bootdev_hunter *orig, *start;
	int n_ent, i;

	orig = ll_entry_start(struct bootdev_hunter, bootdev_hunter);
	n_ent = ll_entry_count(struct bootdev_hunter, bootdev_hunter);

	/*
	 * workaround for strange bug in clang-12 which sees all the below data
	 * as zeroes. Any access of start seems to fix it, such as
	 *
	 *    printf("%p", start);
	 *
	 * Use memcpy() to force the correct behaviour.
	 */
	memcpy(&start, &orig, sizeof(orig));
	printf("%4s  %4s  %-15s  %s\n", "Prio", "Used", "Uclass", "Hunter");
	printf("%4s  %4s  %-15s  %s\n", "----", "----", "---------------", "---------------");
	for (i = 0; i < n_ent; i++) {
		struct bootdev_hunter *info = start + i;

		printf("%4d  %4s  %-15s  %s\n", info->prio,
		       std->hunters_used & BIT(i) ? "*" : "",
		       uclass_get_name(info->uclass),
		       info->drv ? info->drv->name : "(none)");
	}

	printf("(total hunters: %d)\n", n_ent);
}

static int bootdev_post_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	INIT_LIST_HEAD(&ucp->bootflow_head);

	return 0;
}

static int bootdev_pre_unbind(struct udevice *dev)
{
	bootdev_clear_bootflows(dev);

	return 0;
}

UCLASS_DRIVER(bootdev) = {
	.id		= UCLASS_BOOTDEV,
	.name		= "bootdev",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.per_device_plat_auto	= sizeof(struct bootdev_uc_plat),
	.post_bind	= bootdev_post_bind,
	.pre_unbind	= bootdev_pre_unbind,
};
