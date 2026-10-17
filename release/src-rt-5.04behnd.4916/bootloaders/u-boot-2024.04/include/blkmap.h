/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023 Addiva Elektronik
 * Author: Tobias Waldekranz <tobias@waldekranz.com>
 */

#ifndef _BLKMAP_H
#define _BLKMAP_H

#include <dm/lists.h>

/**
 * struct blkmap - Block map
 *
 * Data associated with a blkmap.
 *
 * @label: Human readable name of this blkmap
 * @blk: Underlying block device
 * @slices: List of slices associated with this blkmap
 */
struct blkmap {
	char *label;
	struct udevice *blk;
	struct list_head slices;
};

/**
 * blkmap_map_linear() - Map region of other block device
 *
 * @dev: Blkmap to create the mapping on
 * @blknr: Start block number of the mapping
 * @blkcnt: Number of blocks to map
 * @lblk: The target block device of the mapping
 * @lblknr: The start block number of the target device
 * Returns: 0 on success, negative error code on failure
 */
int blkmap_map_linear(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
		      struct udevice *lblk, lbaint_t lblknr);

/**
 * blkmap_map_mem() - Map region of memory
 *
 * @dev: Blkmap to create the mapping on
 * @blknr: Start block number of the mapping
 * @blkcnt: Number of blocks to map
 * @addr: The target memory address of the mapping
 * Returns: 0 on success, negative error code on failure
 */
int blkmap_map_mem(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
		   void *addr);

/**
 * blkmap_map_pmem() - Map region of physical memory
 *
 * Ensures that a valid physical to virtual memory mapping for the
 * requested region is valid for the lifetime of the mapping, on
 * architectures that require it (sandbox).
 *
 * @dev: Blkmap to create the mapping on
 * @blknr: Start block number of the mapping
 * @blkcnt: Number of blocks to map
 * @paddr: The target physical memory address of the mapping
 * Returns: 0 on success, negative error code on failure
 */
int blkmap_map_pmem(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
		    phys_addr_t paddr);


/**
 * blkmap_from_label() - Find blkmap from label
 *
 * @label: Label of the requested blkmap
 * Returns: A pointer to the blkmap on success, NULL on failure
 */
struct udevice *blkmap_from_label(const char *label);

/**
 * blkmap_create() - Create new blkmap
 *
 * @label: Label of the new blkmap
 * @devp: If not NULL, updated with the address of the resulting device
 * Returns: 0 on success, negative error code on failure
 */
int blkmap_create(const char *label, struct udevice **devp);

/**
 * blkmap_destroy() - Destroy blkmap
 *
 * @dev: The blkmap to be destroyed
 * Returns: 0 on success, negative error code on failure
 */
int blkmap_destroy(struct udevice *dev);

/**
 * blkmap_create_ramdisk() - Create new ramdisk with blkmap
 *
 * @label: Label of the new blkmap
 * @image_addr: Target memory start address of this mapping
 * @image_size: Target memory size of this mapping
 * @devp: Updated with the address of the created blkmap device
 * Returns: 0 on success, negative error code on failure
 */
int blkmap_create_ramdisk(const char *label, ulong image_addr, ulong image_size,
			  struct udevice **devp);

#endif	/* _BLKMAP_H */
