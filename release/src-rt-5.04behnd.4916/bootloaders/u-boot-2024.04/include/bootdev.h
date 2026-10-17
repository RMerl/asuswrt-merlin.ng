/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __bootdev_h
#define __bootdev_h

#include <dm/uclass-id.h>
#include <linux/list.h>

struct bootflow;
struct bootflow_iter;
struct bootstd_priv;
struct udevice;

/**
 * enum bootdev_prio_t - priority of each bootdev
 *
 * These values are associated with each bootdev and set up by the driver.
 *
 * Smallest value is the highest priority. By default, bootdevs are scanned from
 * highest to lowest priority
 *
 * BOOTDEVP_0_NONE: Invalid value, do not use
 * @BOOTDEVP_6_PRE_SCAN: Scan bootdevs with this priority always, before
 * starting any bootflow scan
 * @BOOTDEVP_2_INTERNAL_FAST: Internal devices which don't need scanning and
 * generally very quick to access, e.g. less than 100ms
 * @BOOTDEVP_3_INTERNAL_SLOW: Internal devices which don't need scanning but
 * take a significant fraction of a second to access
 * @BOOTDEVP_4_SCAN_FAST: Extenal devices which need scanning or bus
 * enumeration to find, but this enumeration happens quickly, typically under
 * 100ms
 * @BOOTDEVP_5_SCAN_SLOW: Extenal devices which need scanning or bus
 * enumeration to find. The enumeration takes significant fraction of a second
 * to complete
 * @BOOTDEVP_6_NET_BASE: Basic network devices which are quickly and easily
 * available. Typically used for an internal Ethernet device
 * @BOOTDEVP_7_NET_FALLBACK: Secondary network devices which require extra time
 * to start up, or are less desirable. Typically used for secondary Ethernet
 * devices. Note that USB ethernet devices are found during USB enumeration,
 * so do not use this priority
 */
enum bootdev_prio_t {
	BOOTDEVP_0_NONE,
	BOOTDEVP_1_PRE_SCAN,
	BOOTDEVP_2_INTERNAL_FAST,
	BOOTDEVP_3_INTERNAL_SLOW,
	BOOTDEVP_4_SCAN_FAST,
	BOOTDEVP_5_SCAN_SLOW,
	BOOTDEVP_6_NET_BASE,
	BOOTDEVP_7_NET_FALLBACK,

	BOOTDEVP_COUNT,
};

struct bootdev_hunter;

/**
 * bootdev_hunter_func - function to probe for bootdevs of a given type
 *
 * This should hunt around for bootdevs of the given type, binding them as it
 * finds them. This may involve bus enumeration, etc.
 *
 * @info: Info structure describing this hunter
 * @show: true to show information from the hunter
 * Returns: 0 if OK, -ENOENT on device not found, otherwise -ve on error
 */
typedef int (*bootdev_hunter_func)(struct bootdev_hunter *info, bool show);

/**
 * struct bootdev_hunter - information about how to hunt for bootdevs
 *
 * @prio: Scanning priority of this hunter
 * @uclass: Uclass ID for the media associated with this bootdev
 * @drv: bootdev driver for the things found by this hunter
 * @hunt: Function to call to hunt for bootdevs of this type (NULL if none)
 *
 * Some bootdevs are not visible until other devices are enumerated. For
 * example, USB bootdevs only appear when the USB bus is enumerated.
 *
 * On the other hand, we don't always want to enumerate all the buses just to
 * find the first valid bootdev. Ideally we want to work through them in
 * priority order, so that the fastest bootdevs are discovered first.
 *
 * This struct holds information about the bootdev so we can determine the probe
 * order and how to hunt for bootdevs of this type
 */
struct bootdev_hunter {
	enum bootdev_prio_t prio;
	enum uclass_id uclass;
	struct driver *drv;
	bootdev_hunter_func hunt;
};

/* declare a new bootdev hunter */
#define BOOTDEV_HUNTER(__name)						\
	ll_entry_declare(struct bootdev_hunter, __name, bootdev_hunter)

/* access a bootdev hunter by name */
#define BOOTDEV_HUNTER_GET(__name)						\
	ll_entry_get(struct bootdev_hunter, __name, bootdev_hunter)

/**
 * struct bootdev_uc_plat - uclass information about a bootdev
 *
 * This is attached to each device in the bootdev uclass and accessible via
 * dev_get_uclass_plat(dev)
 *
 * @bootflows: List of available bootflows for this bootdev
 * @piro: Priority of this bootdev
 */
struct bootdev_uc_plat {
	struct list_head bootflow_head;
	enum bootdev_prio_t prio;
};

/** struct bootdev_ops - Operations for the bootdev uclass */
struct bootdev_ops {
	/**
	 * get_bootflow() - get a bootflow (optional)
	 *
	 * If this is NULL then the default implementaton is used, which is
	 * default_get_bootflow()
	 *
	 * @dev:	Bootflow device to check
	 * @iter:	Provides current dev, part, method to get. Should update
	 *	max_part if there is a partition table. Should update state,
	 *	subdir, fname, buf, size according to progress
	 * @bflow:	Updated bootflow if found
	 * Return: 0 if OK, -ESHUTDOWN if there are no more bootflows on this
	 *	device, -ENOSYS if this device doesn't support bootflows,
	 *	other -ve value on other error
	 */
	int (*get_bootflow)(struct udevice *dev, struct bootflow_iter *iter,
			    struct bootflow *bflow);
};

#define bootdev_get_ops(dev)  ((struct bootdev_ops *)(dev)->driver->ops)

/**
 * bootdev_get_bootflow() - get a bootflow
 *
 * @dev:	Bootflow device to check
 * @iter:	Provides current  part, method to get
 * @bflow:	Returns bootflow if found
 * Return: 0 if OK, -ESHUTDOWN if there are no more bootflows on this device,
 *	-ENOSYS if this device doesn't support bootflows, other -ve value on
 *	other error
 */
int bootdev_get_bootflow(struct udevice *dev, struct bootflow_iter *iter,
			 struct bootflow *bflow);

/**
 * bootdev_bind() - Bind a new named bootdev device
 *
 * @parent:	Parent of the new device
 * @drv_name:	Driver name to use for the bootdev device
 * @name:	Name for the device (parent name is prepended)
 * @devp:	the new device (which has not been probed)
 */
int bootdev_bind(struct udevice *parent, const char *drv_name, const char *name,
		 struct udevice **devp);

/**
 * bootdev_find_in_blk() - Find a bootdev in a block device
 *
 * @dev: Bootflow device associated with this block device
 * @blk: Block device to search
 * @iter:	Provides current dev, part, method to get. Should update
 *	max_part if there is a partition table
 * @bflow: On entry, provides information about the partition and device to
 *	check. On exit, returns bootflow if found
 * Return: 0 if found, -ESHUTDOWN if no more bootflows, other -ve on error
 */
int bootdev_find_in_blk(struct udevice *dev, struct udevice *blk,
			struct bootflow_iter *iter, struct bootflow *bflow);

/**
 * bootdev_list() - List all available bootdevs
 *
 * @probe: true to probe devices, false to leave them as is
 */
void bootdev_list(bool probe);

/**
 * bootdev_clear_bootflows() - Clear bootflows from a bootdev
 *
 * Each bootdev maintains a list of discovered bootflows. This provides a
 * way to clear it. These bootflows are removed from the global list too.
 *
 * @dev: bootdev device to update
 */
void bootdev_clear_bootflows(struct udevice *dev);

/**
 * bootdev_add_bootflow() - Add a bootflow to the bootdev's list
 *
 * All fields in @bflow must be set up. Note that @bflow->dev is used to add the
 * bootflow to that device.
 *
 * @dev: Bootdev device to add to
 * @bflow: Bootflow to add. Note that fields within bflow must be allocated
 *	since this function takes over ownership of these. This functions makes
 *	a copy of @bflow itself (without allocating its fields again), so the
 *	caller must dispose of the memory used by the @bflow pointer itself
 * Return: 0 if OK, -ENOMEM if out of memory
 */
int bootdev_add_bootflow(struct bootflow *bflow);

/**
 * bootdev_first_bootflow() - Get the first bootflow from a bootdev
 *
 * Returns the first bootflow attached to a bootdev
 *
 * @dev: bootdev device
 * @bflowp: Returns a pointer to the bootflow
 * Return: 0 if found, -ENOENT if there are no bootflows
 */
int bootdev_first_bootflow(struct udevice *dev, struct bootflow **bflowp);

/**
 * bootdev_next_bootflow() - Get the next bootflow from a bootdev
 *
 * Returns the next bootflow attached to a bootdev
 *
 * @bflowp: On entry, the last bootflow returned , e.g. from
 *	bootdev_first_bootflow()
 * Return: 0 if found, -ENOENT if there are no more bootflows
 */
int bootdev_next_bootflow(struct bootflow **bflowp);

/**
 * bootdev_find_by_label() - Look up a bootdev by label
 *
 * Each bootdev has a label which contains the media-uclass name and a number,
 * e.g. 'mmc2'. This looks up the label and returns the associated bootdev
 *
 * The lookup is performed based on the media device's sequence number. So for
 * 'mmc2' this looks for a device in UCLASS_MMC with a dev_seq() of 2.
 *
 * @label: Label to look up (e.g. "mmc1" or "mmc0")
 * @devp: Returns the bootdev device found, or NULL if none (note it does not
 *	return the media device, but its bootdev child)
 * @method_flagsp: If non-NULL, returns any flags implied by the label
 * (enum bootflow_meth_flags_t), 0 if none. Unset if function fails
 * Return: 0 if OK, -EINVAL if the uclass is not supported by this board,
 * -ENOENT if there is no device with that number
 */
int bootdev_find_by_label(const char *label, struct udevice **devp,
			  int *method_flagsp);

/**
 * bootdev_find_by_any() - Find a bootdev by name, label or sequence
 *
 * @name: name (e.g. "mmc2.bootdev"), label ("mmc2"), or sequence ("2") to find
 * @devp: returns the device found, on success
 * @method_flagsp: If non-NULL, returns any flags implied by the label
 * (enum bootflow_meth_flags_t), 0 if none. Unset if function fails
 * Return: 0 if OK, -EPFNOSUPPORT if the uclass is not supported by this board,
 * -ENOENT if there is no device with that number
 */
int bootdev_find_by_any(const char *name, struct udevice **devp,
			int *method_flagsp);

/**
 * bootdev_setup_iter() - Set up iteration through bootdevs
 *
 * This sets up the an interation, based on the provided device or label. If
 * neither is provided, the iteration is based on the priority of each bootdev,
 * the * bootdev-order property in the bootstd node (or the boot_targets env
 * var).
 *
 * @iter: Iterator to update with the order
 * @label: label to scan, or NULL to scan all
 * @devp: On entry, *devp is NULL to scan all, otherwise this is the (single)
 *	device to scan. Returns the first device to use, which is the passed-in
 *	@devp if it was non-NULL
 * @method_flagsp: If non-NULL, returns any flags implied by the label
 * (enum bootflow_meth_flags_t), 0 if none
 * Return: 0 if OK, -ENOENT if no bootdevs, -ENOMEM if out of memory, other -ve
 *	on other error
 */
int bootdev_setup_iter(struct bootflow_iter *iter, const char *label,
		       struct udevice **devp, int *method_flagsp);

/**
 * bootdev_list_hunters() - List the available bootdev hunters
 *
 * These provide a way to find new bootdevs by enumerating buses, etc. This
 * function lists the available hunters
 *
 * @std: Pointer to bootstd private info
 */
void bootdev_list_hunters(struct bootstd_priv *std);

/**
 * bootdev_hunt() - Hunt for bootdevs matching a particular spec
 *
 * This runs the selected hunter (or all if @spec is NULL) to try to find new
 * bootdevs.
 *
 * @spec: Spec to match, e.g. "mmc0", or NULL for any. If provided, this must
 * match a uclass name so that the hunter can be determined. Any trailing number
 * is ignored
 * @show: true to show each hunter before using it
 * Returns: 0 if OK, -ve on error
 */
int bootdev_hunt(const char *spec, bool show);

/**
 * bootdev_hunt_prio() - Hunt for bootdevs of a particular priority
 *
 * This runs all hunters which can find bootdevs of the given priority.
 *
 * @prio: Priority to use
 * @show: true to show each hunter as it is used
 * Returns: 0 if OK, -ve on error
 */
int bootdev_hunt_prio(enum bootdev_prio_t prio, bool show);

/**
 * bootdev_unhunt() - Mark a device as needing to be hunted again
 *
 * @id: uclass ID to update
 * Return: 0 if done, -EALREADY if already in this state, -ENOENT if no hunter
 * found for that uclass
 */
int bootdev_unhunt(enum uclass_id id);

/**
 * bootdev_hunt_and_find_by_label() - Hunt for bootdevs by label
 *
 * Runs the hunter for the label, then tries to find the bootdev, possible
 * created by the hunter
 *
 * @label: Label to look up (e.g. "mmc1" or "mmc0")
 * @devp: Returns the bootdev device found, or NULL if none (note it does not
 *	return the media device, but its bootdev child)
 * @method_flagsp: If non-NULL, returns any flags implied by the label
 * (enum bootflow_meth_flags_t), 0 if none. Unset if function fails
 * Return: 0 if OK, -EINVAL if the uclass is not supported by this board,
 * -ENOENT if there is no device with that number
 */
int bootdev_hunt_and_find_by_label(const char *label, struct udevice **devp,
				   int *method_flagsp);

/**
 * bootdev_next_label() - Move to the next bootdev in the label sequence
 *
 * Looks through the remaining labels until it finds one that matches a bootdev.
 * Bootdev scanners are used as needed. For example a label "mmc1" results in
 * running the "mmc" bootdrv.
 *
 * @iter: Interation info, containing iter->cur_label
 * @devp: New bootdev found, if any was found
 * @method_flagsp: If non-NULL, returns any flags implied by the label
 * (enum bootflow_meth_flags_t), 0 if none
 * Returns 0 if OK, -ENODEV if no bootdev was found
 */
int bootdev_next_label(struct bootflow_iter *iter, struct udevice **devp,
		       int *method_flagsp);

/**
 * bootdev_next_prio() - Find the next bootdev in priority order
 *
 * This moves @devp to the next bootdev with the current priority. If there is
 * none, then it moves to the next priority and scans for new bootdevs there.
 *
 * @iter: Interation info, containing iter->cur_prio
 * @devp: On entry this is the previous bootdev that was considered. On exit
 *	this is the new bootdev, if any was found
 * Returns 0 on success (*devp is updated), -ENODEV if there are no more
 * bootdevs at any priority
 */
int bootdev_next_prio(struct bootflow_iter *iter, struct udevice **devp);

#if CONFIG_IS_ENABLED(BOOTSTD)
/**
 * bootdev_setup_for_dev() - Bind a new bootdev device (deprecated)
 *
 * Please use bootdev_setup_for_sibling_blk() instead since it supports multiple
 * (child) block devices for each media device.
 *
 * Creates a bootdev device as a child of @parent. This should be called from
 * the driver's bind() method or its uclass' post_bind() method.
 *
 * If a child bootdev already exists, this function does nothing
 *
 * @parent: Parent device (e.g. MMC or Ethernet)
 * @drv_name: Name of bootdev driver to bind
 * Return: 0 if OK, -ve on error
 */
int bootdev_setup_for_dev(struct udevice *parent, const char *drv_name);

/**
 * bootdev_setup_for_sibling_blk() - Bind a new bootdev device for a blk device
 *
 * Creates a bootdev device as a sibling of @blk. This should be called from
 * the driver's bind() method or its uclass' post_bind() method, at the same
 * time as the bould device is bound
 *
 * If a device of the same name already exists, this function does nothing
 *
 * @parent: Parent device (e.g. MMC or Ethernet)
 * @drv_name: Name of bootdev driver to bind
 * Return: 0 if OK, -ve on error
 */
int bootdev_setup_for_sibling_blk(struct udevice *blk, const char *drv_name);

/**
 * bootdev_get_sibling_blk() - Locate the block device for a bootdev
 *
 * @dev: bootdev to check
 * @blkp: returns associated block device
 * Return: 0 if OK, -EINVAL if @dev is not a bootdev device, other -ve on other
 *	error
 */
int bootdev_get_sibling_blk(struct udevice *dev, struct udevice **blkp);

/**
 * bootdev_unbind_dev() - Unbind a bootdev device
 *
 * Remove and unbind a bootdev device which is a child of @parent. This should
 * be called from the driver's unbind() method or its uclass' post_bind()
 * method.
 *
 * @parent: Parent device (e.g. MMC or Ethernet)
 * Return: 0 if OK, -ve on error
 */
int bootdev_unbind_dev(struct udevice *parent);
#else
static inline int bootdev_setup_for_dev(struct udevice *parent,
					const char *drv_name)
{
	return 0;
}

static inline int bootdev_setup_for_sibling_blk(struct udevice *blk,
						const char *drv_name)
{
	return 0;
}

static inline int bootdev_unbind_dev(struct udevice *parent)
{
	return 0;
}
#endif

#endif
