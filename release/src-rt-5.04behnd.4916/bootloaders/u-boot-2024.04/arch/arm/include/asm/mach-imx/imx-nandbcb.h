/*
 * Copyright (C) 2017 Jagan Teki <jagan@amarulasolutions.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _IMX_NAND_BCB_H_
#define _IMX_NAND_BCB_H_

#define FCB_FINGERPRINT		0x20424346      /* 'FCB' */
#define FCB_VERSION_1		0x01000000
#define FCB_FINGERPRINT_OFF	0x4		/* FCB fingerprint offset*/

#define DBBT_FINGERPRINT	0x54424244	/* 'DBBT' */
#define DBBT_VERSION_1		0x01000000
#define DBBT_FINGERPRINT_OFF	0x4		/* DBBT fingerprint offset*/

struct dbbt_block {
	u32 checksum;	/* reserved on i.MX6 */
	u32 fingerprint;
	u32 version;
	u32 numberbb;	/* reserved on i.MX6 */
	u32 dbbtpages;
};

struct fcb_block {
	u32 checksum;		/* First fingerprint in first byte */
	u32 fingerprint;	/* 2nd fingerprint at byte 4 */
	u32 version;		/* 3rd fingerprint at byte 8 */
	u8 datasetup;
	u8 datahold;
	u8 addr_setup;
	u8 dsample_time;

	/* These are for application use only and not for ROM. */
	u8 nandtiming;
	u8 rea;
	u8 rloh;
	u8 rhoh;
	u32 pagesize;		/* 2048 for 2K pages, 4096 for 4K pages */
	u32 oob_pagesize;	/* 2112 for 2K pages, 4314 for 4K pages */
	u32 sectors;		/* Number of 2K sections per block */
	u32 nr_nand;		/* Total Number of NANDs - not used by ROM */
	u32 nr_die;		/* Number of separate chips in this NAND */
	u32 celltype;		/* MLC or SLC */
	u32 ecc_type;		/* Type of ECC, can be one of BCH-0-20 */
	u32 ecc_nr;		/* Number of bytes for Block0 - BCH */

	/* Block size in bytes for all blocks other than Block0 - BCH */
	u32 ecc_size;
	u32 ecc_level;		/* Ecc level for Block 0 - BCH */
	u32 meta_size;		/* Metadata size - BCH */
	/* Number of blocks per page for ROM use - BCH */
	u32 nr_blocks;
	u32 ecc_type_sdk;	/* Type of ECC, can be one of BCH-0-20 */
	u32 ecc_nr_sdk;		/* Number of bytes for Block0 - BCH */
	/* Block size in bytes for all blocks other than Block0 - BCH */
	u32 ecc_size_sdk;
	u32 ecc_level_sdk;	/* Ecc level for Block 0 - BCH */
	/* Number of blocks per page for SDK use - BCH */
	u32 nr_blocks_sdk;
	u32 meta_size_sdk;	/* Metadata size - BCH */
	u32 erase_th;		/* To set into BCH_MODE register */

	/*
	 * 0: normal boot
	 * 1: to load patch starting next to FCB
	 */
	u32 bootpatch;
	u32 patch_size;	/* Size of patch in sectors */
	u32 fw1_start;	/* Firmware image starts on this sector */
	u32 fw2_start;	/* Secondary FW Image starting Sector */
	u32 fw1_pages;	/* Number of sectors in firmware image */
	u32 fw2_pages;	/* Number of sector in secondary FW image */
	u32 dbbt_start; /* Page address where dbbt search area begins */

	/*
	 * Byte in page data that have manufacturer marked bad block marker,
	 * this will be swapped with metadata[0] to complete page data.
	 */
	u32 bb_byte;

	/*
	 * For BCH ECC sizes other than 8 and 16 the bad block marker does not
	 * start at 0th bit of bb_byte. This field is used to get to
	 * the start bit of bad block marker byte with in bb_byte
	 */
	u32 bb_start_bit;

	/*
	 * FCB value that gives byte offset for
	 * bad block marker on physical NAND page
	 */
	u32 phy_offset;
	u32 bchtype;

	u32 readlatency;
	u32 predelay;
	u32 cedelay;
	u32 postdelay;
	u32 cmdaddpause;
	u32 datapause;
	u32 tmspeed;
	u32 busytimeout;

	/* the flag to enable (1)/disable(0) bi swap */
	u32 disbbm;

	/* The swap position of main area in spare area */
	u32 spare_offset;

	/* Actual for iMX7 only */
	u32 onfi_sync_enable;
	u32 onfi_sync_speed;
	u32 onfi_sync_nand_data;
	u32 reserved2[6];
	u32 disbbm_search;
	u32 disbbm_search_limit;
	u32 reserved3[15];
	u32 read_retry_enable;
	u32 reserved4[1];
	u32 fill_to_1024[183];
};

#endif	/* _IMX_NAND_BCB_H_ */
