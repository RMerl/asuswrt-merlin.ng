/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Xilinx, Inc.
 */

#ifndef _ZYNQ_BOOTIMG_H_
#define _ZYNQ_BOOTIMG_H_

#define ZYNQ_MAX_PARTITION_NUMBER	0xE

struct partition_hdr {
	u32 imagewordlen;	/* 0x0 */
	u32 datawordlen;	/* 0x4 */
	u32 partitionwordlen;	/* 0x8 */
	u32 loadaddr;		/* 0xC */
	u32 execaddr;		/* 0x10 */
	u32 partitionstart;	/* 0x14 */
	u32 partitionattr;	/* 0x18 */
	u32 sectioncount;	/* 0x1C */
	u32 checksumoffset;	/* 0x20 */
	u32 pads1[1];
	u32 acoffset;	/* 0x28 */
	u32 pads2[4];
	u32 checksum;		/* 0x3C */
};

int zynq_get_part_count(struct partition_hdr *part_hdr_info);
int zynq_get_partition_info(u32 image_base_addr, u32 *fsbl_len,
			    struct partition_hdr *part_hdr);
int zynq_validate_hdr(struct partition_hdr *header);
int zynq_validate_partition(u32 start_addr, u32 len, u32 chksum_off);

#endif /* _ZYNQ_BOOTIMG_H_ */
