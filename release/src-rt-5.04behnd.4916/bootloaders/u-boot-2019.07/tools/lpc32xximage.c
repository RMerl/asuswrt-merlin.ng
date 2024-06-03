// SPDX-License-Identifier: GPL-2.0+
/*
 * Image manipulator for LPC32XX SoCs
 *
 * (C) Copyright 2015  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 *
 * Derived from omapimage.c:
 *
 * (C) Copyright 2010
 * Linaro LTD, www.linaro.org
 * Author: John Rigby <john.rigby@linaro.org>
 * Based on TI's signGP.c
 *
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * (C) Copyright 2008
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include "imagetool.h"
#include <compiler.h>
#include <image.h>

/*
 * NAND page 0 boot header
 */

struct nand_page_0_boot_header {
	uint32_t data[129];
	uint32_t pad[383];
};

/*
 * Default ICC (interface configuration data [sic]) if none specified
 * in board config
 */

#ifndef LPC32XX_BOOT_ICR
#define LPC32XX_BOOT_ICR 0x00000096
#endif

/*
 * Default boot NAND page size if none specified in board config
 */

#ifndef LPC32XX_BOOT_NAND_PAGESIZE
#define LPC32XX_BOOT_NAND_PAGESIZE 2048
#endif

/*
 * Default boot NAND pages per sector if none specified in board config
 */

#ifndef LPC32XX_BOOT_NAND_PAGES_PER_SECTOR
#define LPC32XX_BOOT_NAND_PAGES_PER_SECTOR 64
#endif

/*
 * Maximum size for boot code is 56K unless defined in board config
 */

#ifndef LPC32XX_BOOT_CODESIZE
#define LPC32XX_BOOT_CODESIZE (56*1024)
#endif

/* signature byte for a readable block */

#define LPC32XX_BOOT_BLOCK_OK 0xaa

static struct nand_page_0_boot_header lpc32xximage_header;

static int lpc32xximage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_LPC32XXIMAGE)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

static int lpc32xximage_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	struct nand_page_0_boot_header *hdr =
		(struct nand_page_0_boot_header *)ptr;

	/* turn image size from bytes to NAND pages, page 0 included */
	int image_size_in_pages = ((image_size - 1)
				  / LPC32XX_BOOT_NAND_PAGESIZE);

	if (hdr->data[0] != (0xff & LPC32XX_BOOT_ICR))
		return -1;
	if (hdr->data[1] != (0xff & ~LPC32XX_BOOT_ICR))
		return -1;
	if (hdr->data[2] != (0xff & LPC32XX_BOOT_ICR))
		return -1;
	if (hdr->data[3] != (0xff & ~LPC32XX_BOOT_ICR))
		return -1;
	if (hdr->data[4] != (0xff & image_size_in_pages))
		return -1;
	if (hdr->data[5] != (0xff & ~image_size_in_pages))
		return -1;
	if (hdr->data[6] != (0xff & image_size_in_pages))
		return -1;
	if (hdr->data[7] != (0xff & ~image_size_in_pages))
		return -1;
	if (hdr->data[8] != (0xff & image_size_in_pages))
		return -1;
	if (hdr->data[9] != (0xff & ~image_size_in_pages))
		return -1;
	if (hdr->data[10] != (0xff & image_size_in_pages))
		return -1;
	if (hdr->data[11] != (0xff & ~image_size_in_pages))
		return -1;
	if (hdr->data[12] != LPC32XX_BOOT_BLOCK_OK)
		return -1;
	if (hdr->data[128] != LPC32XX_BOOT_BLOCK_OK)
		return -1;
	return 0;
}

static void print_hdr_byte(struct nand_page_0_boot_header *hdr, int ofs)
{
	printf("header[%d] = %02x\n", ofs, hdr->data[ofs]);
}

static void lpc32xximage_print_header(const void *ptr)
{
	struct nand_page_0_boot_header *hdr =
		(struct nand_page_0_boot_header *)ptr;
	int ofs;

	for (ofs = 0; ofs <= 12; ofs++)
		print_hdr_byte(hdr, ofs);
	print_hdr_byte(hdr, 128);
}

static void lpc32xximage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	struct nand_page_0_boot_header *hdr =
		(struct nand_page_0_boot_header *)ptr;

	/* turn image size from bytes to NAND pages, page 0 included */
	int image_size_in_pages = ((sbuf->st_size
				  + LPC32XX_BOOT_NAND_PAGESIZE - 1)
				  / LPC32XX_BOOT_NAND_PAGESIZE);

	/* fill header -- default byte value is 0x00, not 0xFF */
	memset((void *)hdr, 0, sizeof(*hdr));
	hdr->data[0] = (hdr->data[2] = 0xff & LPC32XX_BOOT_ICR);
	hdr->data[1] = (hdr->data[3] = 0xff & ~LPC32XX_BOOT_ICR);
	hdr->data[4] = (hdr->data[6] = (hdr->data[8]
		       = (hdr->data[10] = 0xff & image_size_in_pages)));
	hdr->data[5] = (hdr->data[7] = (hdr->data[9]
		       = (hdr->data[11] = 0xff & ~image_size_in_pages)));
	hdr->data[12] = (hdr->data[128] = LPC32XX_BOOT_BLOCK_OK);
}

/*
 * lpc32xximage parameters
 */
U_BOOT_IMAGE_TYPE(
	lpc32xximage,
	"LPC32XX Boot Image",
	sizeof(lpc32xximage_header),
	(void *)&lpc32xximage_header,
	NULL,
	lpc32xximage_verify_header,
	lpc32xximage_print_header,
	lpc32xximage_set_header,
	NULL,
	lpc32xximage_check_image_types,
	NULL,
	NULL
);
