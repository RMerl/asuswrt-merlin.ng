/* SPDX-License-Identifier: GPL 2.0+ OR BSD-3-Clause */
/*
 * The parts taken from the kernel implementation are:
 *
 * Copyright (c) International Business Machines Corp., 2006
 *
 * UBISPL specific defines:
 *
 * Copyright (c) Thomas Gleixner <tglx@linutronix.de>
 */

/*
 * Contains various defines copy&pasted from ubi.h and ubi-user.h to make
 * the upstream fastboot code happy.
 */
#ifndef __UBOOT_UBI_WRAPPER_H
#define __UBOOT_UBI_WRAPPER_H

/*
 * Error codes returned by the I/O sub-system.
 *
 * UBI_IO_FF: the read region of flash contains only 0xFFs
 * UBI_IO_FF_BITFLIPS: the same as %UBI_IO_FF, but also also there was a data
 *                     integrity error reported by the MTD driver
 *                     (uncorrectable ECC error in case of NAND)
 * UBI_IO_BAD_HDR: the EC or VID header is corrupted (bad magic or CRC)
 * UBI_IO_BAD_HDR_EBADMSG: the same as %UBI_IO_BAD_HDR, but also there was a
 *                         data integrity error reported by the MTD driver
 *                         (uncorrectable ECC error in case of NAND)
 * UBI_IO_BITFLIPS: bit-flips were detected and corrected
 *
 * UBI_FASTMAP_ANCHOR:  u-boot SPL add on to tell the caller that the fastmap
 *			anchor block has been found
 *
 * Note, it is probably better to have bit-flip and ebadmsg as flags which can
 * be or'ed with other error code. But this is a big change because there are
 * may callers, so it does not worth the risk of introducing a bug
 */
enum {
	UBI_IO_FF = 1,
	UBI_IO_FF_BITFLIPS,
	UBI_IO_BAD_HDR,
	UBI_IO_BAD_HDR_EBADMSG,
	UBI_IO_BITFLIPS,
	UBI_FASTMAP_ANCHOR,
};

/*
 * UBI volume type constants.
 *
 * @UBI_DYNAMIC_VOLUME: dynamic volume
 * @UBI_STATIC_VOLUME:  static volume
 */
enum {
	UBI_DYNAMIC_VOLUME = 3,
	UBI_STATIC_VOLUME  = 4,
};

/*
 * Return codes of the fastmap sub-system
 *
 * UBI_NO_FASTMAP: No fastmap super block was found
 * UBI_BAD_FASTMAP: A fastmap was found but it's unusable
 */
enum {
	UBI_NO_FASTMAP = 1,
	UBI_BAD_FASTMAP,
};

/**
 * struct ubi_fastmap_layout - in-memory fastmap data structure.
 * @e: PEBs used by the current fastmap
 * @to_be_tortured: if non-zero tortured this PEB
 * @used_blocks: number of used PEBs
 * @max_pool_size: maximal size of the user pool
 * @max_wl_pool_size: maximal size of the pool used by the WL sub-system
 */
struct ubi_fastmap_layout {
	struct ubi_wl_entry *e[UBI_FM_MAX_BLOCKS];
	int to_be_tortured[UBI_FM_MAX_BLOCKS];
	int used_blocks;
	int max_pool_size;
	int max_wl_pool_size;
};

/**
 * struct ubi_fm_pool - in-memory fastmap pool
 * @pebs: PEBs in this pool
 * @used: number of used PEBs
 * @size: total number of PEBs in this pool
 * @max_size: maximal size of the pool
 *
 * A pool gets filled with up to max_size.
 * If all PEBs within the pool are used a new fastmap will be written
 * to the flash and the pool gets refilled with empty PEBs.
 *
 */
struct ubi_fm_pool {
	int pebs[UBI_FM_MAX_POOL_SIZE];
	int used;
	int size;
	int max_size;
};

#endif
