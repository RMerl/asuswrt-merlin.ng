// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Xilinx, Inc.
 */

#include <common.h>
#include <log.h>
#include <part.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <u-boot/md5.h>
#include <zynq_bootimg.h>

DECLARE_GLOBAL_DATA_PTR;

#define ZYNQ_IMAGE_PHDR_OFFSET		0x09C
#define ZYNQ_IMAGE_FSBL_LEN_OFFSET	0x040
#define ZYNQ_PART_HDR_CHKSUM_WORD_COUNT	0x0F
#define ZYNQ_PART_HDR_WORD_COUNT	0x10
#define ZYNQ_MAXIMUM_IMAGE_WORD_LEN	0x40000000
#define MD5_CHECKSUM_SIZE	16

struct headerarray {
	u32 fields[16];
};

/*
 * Check whether the given partition is last partition or not
 */
static int zynq_islastpartition(struct headerarray *head)
{
	int index;

	debug("%s\n", __func__);
	if (head->fields[ZYNQ_PART_HDR_CHKSUM_WORD_COUNT] != 0xFFFFFFFF)
		return -1;

	for (index = 0; index < ZYNQ_PART_HDR_WORD_COUNT - 1; index++) {
		if (head->fields[index] != 0x0)
			return -1;
	}

	return 0;
}

/*
 * Get the partition count from the partition header
 */
int zynq_get_part_count(struct partition_hdr *part_hdr_info)
{
	u32 count;
	struct headerarray *hap;

	debug("%s\n", __func__);

	for (count = 0; count < ZYNQ_MAX_PARTITION_NUMBER; count++) {
		hap = (struct headerarray *)&part_hdr_info[count];
		if (zynq_islastpartition(hap) != -1)
			break;
	}

	return count;
}

/*
 * Get the partition info of all the partitions available.
 */
int zynq_get_partition_info(u32 image_base_addr, u32 *fsbl_len,
			    struct partition_hdr *part_hdr)
{
	u32 parthdroffset;

	*fsbl_len = *((u32 *)(image_base_addr + ZYNQ_IMAGE_FSBL_LEN_OFFSET));

	parthdroffset = *((u32 *)(image_base_addr + ZYNQ_IMAGE_PHDR_OFFSET));

	parthdroffset += image_base_addr;

	memcpy(part_hdr, (u32 *)parthdroffset,
	       (sizeof(struct partition_hdr) * ZYNQ_MAX_PARTITION_NUMBER));

	return 0;
}

/*
 * Check whether the partition header is valid or not
 */
int zynq_validate_hdr(struct partition_hdr *header)
{
	struct headerarray *hap;
	u32 index;
	u32 checksum;

	debug("%s\n", __func__);

	hap = (struct headerarray *)header;

	for (index = 0; index < ZYNQ_PART_HDR_WORD_COUNT; index++) {
		if (hap->fields[index])
			break;
	}
	if (index == ZYNQ_PART_HDR_WORD_COUNT)
		return -1;

	checksum = 0;
	for (index = 0; index < ZYNQ_PART_HDR_CHKSUM_WORD_COUNT; index++)
		checksum += hap->fields[index];

	checksum ^= 0xFFFFFFFF;

	if (hap->fields[ZYNQ_PART_HDR_CHKSUM_WORD_COUNT] != checksum) {
		printf("Error: Checksum 0x%8.8x != 0x%8.8x\n",
		       checksum, hap->fields[ZYNQ_PART_HDR_CHKSUM_WORD_COUNT]);
		return -1;
	}

	if (header->imagewordlen > ZYNQ_MAXIMUM_IMAGE_WORD_LEN) {
		printf("INVALID_PARTITION_LENGTH\n");
		return -1;
	}

	return 0;
}

/*
 * Validate the partition by calculationg the md5 checksum for the
 * partition and compare with checksum present in checksum offset of
 * partition
 */
int zynq_validate_partition(u32 start_addr, u32 len, u32 chksum_off)
{
	u8 checksum[MD5_CHECKSUM_SIZE];
	u8 calchecksum[MD5_CHECKSUM_SIZE];

	memcpy(&checksum[0], (u32 *)chksum_off, MD5_CHECKSUM_SIZE);

	md5_wd((u8 *)start_addr, len, &calchecksum[0], 0x10000);

	if (!memcmp(checksum, calchecksum, MD5_CHECKSUM_SIZE))
		return 0;

	printf("Error: Partition DataChecksum\n");
	return -1;
}
