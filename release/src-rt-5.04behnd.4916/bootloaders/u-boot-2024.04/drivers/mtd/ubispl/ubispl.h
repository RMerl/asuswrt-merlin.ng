/* SPDX-License-Identifier: GPL 2.0+ OR BSD-3-Clause */
/*
 * Copyright (c) Thomas Gleixner <tglx@linutronix.de>
 */

#ifndef _UBOOT_MTD_UBISPL_H
#define _UBOOT_MTD_UBISPL_H

#include "../ubi/ubi-media.h"
#include "ubi-wrapper.h"

/*
 * The maximum number of volume ids we scan. So you can load volume id
 * 0 to (CONFIG_SPL_UBI_VOL_ID_MAX - 1)
 */
#define UBI_SPL_VOL_IDS		CONFIG_SPL_UBI_VOL_IDS
/*
 * The size of the read buffer for the fastmap blocks. In theory up to
 * UBI_FM_MAX_BLOCKS * CONFIG_SPL_MAX_PEB_SIZE. In practice today
 * one or two blocks.
 */
#define UBI_FM_BUF_SIZE		(UBI_FM_MAX_BLOCKS*CONFIG_SPL_UBI_MAX_PEB_SIZE)
/*
 * The size of the bitmaps for the attach/ scan
 */
#define UBI_FM_BM_SIZE		((CONFIG_SPL_UBI_MAX_PEBS / BITS_PER_LONG) + 1)
/*
 * The maximum number of logical erase blocks per loadable volume
 */
#define UBI_MAX_VOL_LEBS	CONFIG_SPL_UBI_MAX_VOL_LEBS
/*
 * The bitmap size for the above to denote the found blocks inside the volume
 */
#define UBI_VOL_BM_SIZE		((UBI_MAX_VOL_LEBS / BITS_PER_LONG) + 1)

/**
 * struct ubi_vol_info - UBISPL internal volume represenation
 * @last_block:		The last block (highest LEB) found for this volume
 * @found:		Bitmap to mark found LEBS
 * @lebs_to_pebs:	LEB to PEB translation table
 */
struct ubi_vol_info {
	u32				last_block;
	unsigned long			found[UBI_VOL_BM_SIZE];
	u32				lebs_to_pebs[UBI_MAX_VOL_LEBS];
};

/**
 * struct ubi_scan_info - UBISPL internal data for FM attach and full scan
 *
 * @read:		Read function to access the flash provided by the caller
 * @peb_count:		Number of physical erase blocks in the UBI FLASH area
 *			aka MTD partition.
 * @peb_offset:		Offset of PEB0 in the UBI FLASH area (aka MTD partition)
 *			to the real start of the FLASH in erase blocks.
 * @fsize_mb:		Size of the scanned FLASH area in MB (stats only)
 * @vid_offset:		Offset from the start of a PEB to the VID header
 * @leb_start:		Offset from the start of a PEB to the data area
 * @leb_size:		Size of the data area
 *
 * @fastmap_pebs:	Counter of PEBs "attached" by fastmap
 * @fastmap_anchor:	The anchor PEB of the fastmap
 * @fm_sb:		The fastmap super block data
 * @fm_vh:		The fastmap VID header
 * @fm:			Pointer to the fastmap layout
 * @fm_layout:		The fastmap layout itself
 * @fm_pool:		The pool of PEBs to scan at fastmap attach time
 * @fm_wl_pool:		The pool of PEBs scheduled for wearleveling
 *
 * @fm_enabled:		Indicator whether fastmap attachment is enabled.
 * @fm_used:		Bitmap to indicate the PEBS covered by fastmap
 * @scanned:		Bitmap to indicate the PEBS of which the VID header
 *			hase been physically scanned.
 * @corrupt:		Bitmap to indicate corrupt blocks
 * @toload:		Bitmap to indicate the volumes which should be loaded
 *
 * @blockinfo:		The vid headers of the scanned blocks
 * @volinfo:		The volume information of the interesting (toload)
 *			volumes
 * @vtbl_corrupted:	Flag to indicate status of volume table
 * @vtbl:		Volume table
 *
 * @fm_buf:		The large fastmap attach buffer
 */
struct ubi_scan_info {
	ubispl_read_flash		read;
	unsigned int			fsize_mb;
	unsigned int			peb_count;
	unsigned int			peb_offset;

	unsigned long			vid_offset;
	unsigned long			leb_start;
	unsigned long			leb_size;

	/* Fastmap: The upstream required fields */
	int				fastmap_pebs;
	int				fastmap_anchor;
	size_t				fm_size;
	struct ubi_fm_sb		fm_sb;
	struct ubi_vid_hdr		fm_vh;
	struct ubi_fastmap_layout	*fm;
	struct ubi_fastmap_layout	fm_layout;
	struct ubi_fm_pool		fm_pool;
	struct ubi_fm_pool		fm_wl_pool;

	/* Fastmap: UBISPL specific data */
	int				fm_enabled;
	unsigned long			fm_used[UBI_FM_BM_SIZE];
	unsigned long			scanned[UBI_FM_BM_SIZE];
	unsigned long			corrupt[UBI_FM_BM_SIZE];
	unsigned long			toload[UBI_FM_BM_SIZE];

	/* Data for storing the VID and volume information */
	struct ubi_vol_info		volinfo[UBI_SPL_VOL_IDS];
	struct ubi_vid_hdr		blockinfo[CONFIG_SPL_UBI_MAX_PEBS];

#ifdef CONFIG_SPL_UBI_LOAD_BY_VOLNAME
	/* Volume table */
	int                             vtbl_valid;
	struct ubi_vtbl_record          vtbl[UBI_SPL_VOL_IDS];
#endif
	/* The large buffer for the fastmap */
	uint8_t				fm_buf[UBI_FM_BUF_SIZE];
};

#ifdef CFG_DEBUG
#define ubi_dbg(fmt, ...) printf("UBI: debug:" fmt "\n", ##__VA_ARGS__)
#else
#define ubi_dbg(fmt, ...)
#endif

#ifdef CONFIG_UBI_SPL_SILENCE_MSG
#define ubi_msg(fmt, ...)
#else
#define ubi_msg(fmt, ...) printf("UBI: " fmt "\n", ##__VA_ARGS__)
#endif
/* UBI warning messages */
#define ubi_warn(fmt, ...) printf("UBI warning: " fmt "\n", ##__VA_ARGS__)
/* UBI error messages */
#define ubi_err(fmt, ...) printf("UBI error: " fmt "\n", ##__VA_ARGS__)

#endif
