// SPDX-License-Identifier: GPL-2.0+
/*
 * Image manipulator for Vybrid SoCs
 *
 * Derived from vybridimage.c
 *
 * (C) Copyright 2016  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 */

#include "imagetool.h"
#include <compiler.h>
#include <image.h>

/*
 * NAND page 0 boot header
 */

struct nand_page_0_boot_header {
	union {
		uint32_t fcb[128];
		uint8_t fcb_bytes[512];
	};				/* 0x00000000 - 0x000001ff */
	uint8_t  sw_ecc[512];		/* 0x00000200 - 0x000003ff */
	uint32_t padding[65280];	/* 0x00000400 - 0x0003ffff */
	uint8_t ivt_prefix[1024];	/* 0x00040000 - 0x000403ff */
};

/* signature byte for a readable block */

static struct nand_page_0_boot_header vybridimage_header;

static int vybridimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_VYBRIDIMAGE)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

static uint8_t vybridimage_sw_ecc(uint8_t byte)
{
	uint8_t bit0  = (byte & (1 << 0)) ? 1 : 0;
	uint8_t bit1  = (byte & (1 << 1)) ? 1 : 0;
	uint8_t bit2  = (byte & (1 << 2)) ? 1 : 0;
	uint8_t bit3  = (byte & (1 << 3)) ? 1 : 0;
	uint8_t bit4  = (byte & (1 << 4)) ? 1 : 0;
	uint8_t bit5  = (byte & (1 << 5)) ? 1 : 0;
	uint8_t bit6  = (byte & (1 << 6)) ? 1 : 0;
	uint8_t bit7  = (byte & (1 << 7)) ? 1 : 0;
	uint8_t res = 0;

	res |= ((bit6 ^ bit5 ^ bit3 ^ bit2) << 0);
	res |= ((bit7 ^ bit5 ^ bit4 ^ bit2 ^ bit1) << 1);
	res |= ((bit7 ^ bit6 ^ bit5 ^ bit1 ^ bit0) << 2);
	res |= ((bit7 ^ bit4 ^ bit3 ^ bit0) << 3);
	res |= ((bit6 ^ bit4 ^ bit3 ^ bit2 ^ bit1 ^ bit0) << 4);

	return res;
}

static int vybridimage_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	struct nand_page_0_boot_header *hdr =
		(struct nand_page_0_boot_header *)ptr;
	int idx;

	if (hdr->fcb[1] != 0x46434220)
		return -1;
	if (hdr->fcb[2] != 1)
		return -1;
	if (hdr->fcb[7] != 64)
		return -1;
	if (hdr->fcb[14] != 6)
		return -1;
	if (hdr->fcb[30] != 0x0001ff00)
		return -1;
	if (hdr->fcb[43] != 1)
		return -1;
	if (hdr->fcb[54] != 0)
		return -1;
	if (hdr->fcb[55] != 8)
		return -1;

	/* check software ECC */
	for (idx = 0; idx < sizeof(hdr->fcb_bytes); idx++) {
		uint8_t sw_ecc = vybridimage_sw_ecc(hdr->fcb_bytes[idx]);
		if (sw_ecc != hdr->sw_ecc[idx])
			return -1;
	}

	return 0;
}

static void vybridimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	struct nand_page_0_boot_header *hdr =
		(struct nand_page_0_boot_header *)ptr;
	int idx;

	/* fill header with 0x00 for first 56 entries then 0xff */
	memset(&hdr->fcb[0], 0x0, 56*sizeof(uint32_t));
	memset(&hdr->fcb[56], 0xff, 72*sizeof(uint32_t));
	/* fill SW ecc and padding with 0xff */
	memset(&hdr->sw_ecc[0], 0xff, sizeof(hdr->sw_ecc));
	memset(&hdr->padding[0], 0xff, sizeof(hdr->padding));
	/* fill IVT prefix with 0x00 */
	memset(&hdr->ivt_prefix[0], 0x00, sizeof(hdr->ivt_prefix));

	/* populate fcb */
	hdr->fcb[1] = 0x46434220; /* signature */
	hdr->fcb[2] = 0x00000001; /* version */
	hdr->fcb[5] = 2048; /* page size */
	hdr->fcb[6] = (2048+64); /* page + OOB size */
	hdr->fcb[7] = 64; /* pages per block */
	hdr->fcb[14] = 6; /* ECC mode 6 */
	hdr->fcb[26] = 128; /* fw address (0x40000) in 2K pages */
	hdr->fcb[27] = 128; /* fw address (0x40000) in 2K pages */
	hdr->fcb[30] = 0x0001ff00; /* DBBT search area start address */
	hdr->fcb[33] = 2048; /* BB marker physical offset */
	hdr->fcb[43] = 1; /* DISBBM */
	hdr->fcb[54] = 0; /* DISBB_Search */
	hdr->fcb[55] = 8; /* Bad block search limit */

	/* compute software ECC */
	for (idx = 0; idx < sizeof(hdr->fcb_bytes); idx++)
		hdr->sw_ecc[idx] = vybridimage_sw_ecc(hdr->fcb_bytes[idx]);
}

static void vybridimage_print_hdr_field(struct nand_page_0_boot_header *hdr,
	int idx)
{
	printf("header.fcb[%d] = %08x\n", idx, hdr->fcb[idx]);
}

static void vybridimage_print_header(const void *ptr)
{
	struct nand_page_0_boot_header *hdr =
		(struct nand_page_0_boot_header *)ptr;
	int idx;

	for (idx = 0; idx < 56; idx++)
		vybridimage_print_hdr_field(hdr, idx);
}

/*
 * vybridimage parameters
 */
U_BOOT_IMAGE_TYPE(
	vybridimage,
	"Vybrid Boot Image",
	sizeof(vybridimage_header),
	(void *)&vybridimage_header,
	NULL,
	vybridimage_verify_header,
	vybridimage_print_header,
	vybridimage_set_header,
	NULL,
	vybridimage_check_image_types,
	NULL,
	NULL
);
