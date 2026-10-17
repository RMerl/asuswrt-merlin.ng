/* SPDX-License-Identifier: GPL 2.0+ OR BSD-3-Clause */
/*
 * Copyright (c) Thomas Gleixner <tglx@linutronix.de>
 */
#ifndef __UBOOT_UBISPL_H
#define __UBOOT_UBISPL_H

#define UBI_VOL_NAME_MAX	127

/*
 * The following CONFIG options are relevant for UBISPL
 *
 * #define CONFIG_SPL_UBI_MAX_VOL_LEBS		256
 *
 * Defines the maximum number of logical erase blocks per loadable
 * (static) volume to size the ubispl internal arrays.
 *
 * #define CONFIG_SPL_UBI_MAX_PEB_SIZE		(256*1024)
 *
 * Defines the maximum physical erase block size to size the fastmap
 * buffer for ubispl.
 *
 * #define CONFIG_SPL_UBI_MAX_PEBS		4096
 *
 * Define the maximum number of physical erase blocks to size the
 * ubispl internal arrays.
 *
 * #define CONFIG_SPL_UBI_VOL_IDS		8
 *
 * Defines the maximum number of volumes in which UBISPL is
 * interested. Limits the amount of memory for the scan data and
 * speeds up the scan process as we simply ignore stuff which we dont
 * want to load from the SPL anyway. So the volumes which can be
 * loaded in the above example are ids 0 - 7
 */

/*
 * The struct definition is in drivers/mtd/ubispl/ubispl.h. It does
 * not fit into the BSS due to the large buffer requirement of the
 * upstream fastmap code. So the caller of ubispl_load_volumes needs
 * to hand in a pointer to a free memory area where ubispl will place
 * its data. The area is not required to be initialized.
 */
struct ubi_scan_info;

typedef int (*ubispl_read_flash)(int pnum, int offset, int len, void *dst);

/**
 * struct ubispl_info - description structure for fast ubi scan
 * @ubi:		Pointer to memory space for ubi scan info structure
 * @peb_size:		Physical erase block size
 * @vid_offset:		Offset of the VID header
 * @leb_start:		Start of the logical erase block, i.e. offset of data
 * @peb_count:		Number of physical erase blocks in the UBI FLASH area
 *			aka MTD partition.
 * @peb_offset:		Offset of PEB0 in the UBI FLASH area (aka MTD partition)
 *			to the real start of the FLASH in erase blocks.
 * @fastmap:		Enable fastmap attachment
 * @read:		Read function to access the flash
 */
struct ubispl_info {
	struct ubi_scan_info	*ubi;
	u32			peb_size;
	u32			vid_offset;
	u32			leb_start;
	u32			peb_count;
	u32			peb_offset;
	int			fastmap;
	ubispl_read_flash	read;
};

/**
 * struct ubispl_load - structure to describe a volume to load
 * @vol_id:	Volume id
 * @load_addr:	Load address of the volume
 */
struct ubispl_load {
	int		vol_id;
#ifdef CONFIG_SPL_UBI_LOAD_BY_VOLNAME
	u32		name_len;
	char		name[UBI_VOL_NAME_MAX + 1];
#endif
	void		*load_addr;
};

/**
 * ubispl_load_volumes - Scan flash and load volumes
 * @info:	Pointer to the ubi scan info structure
 * @lovls:	Pointer to array of volumes to load
 * @nrvols:	Array size of @lovls
 */
int ubispl_load_volumes(struct ubispl_info *info,
			struct ubispl_load *lvols, int nrvols);

#endif
