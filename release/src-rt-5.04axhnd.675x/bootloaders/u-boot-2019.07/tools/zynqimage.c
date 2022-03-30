// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Nathan Rossi <nathan@nathanrossi.com>
 *
 * The following Boot Header format/structures and values are defined in the
 * following documents:
 *   * Xilinx Zynq-7000 Technical Reference Manual (Section 6.3)
 *   * Xilinx Zynq-7000 Software Developers Guide (Appendix A.7 and A.8)
 *
 * Expected Header Size = 0x8C0
 * Forced as 'little' endian, 32-bit words
 *
 *  0x  0 - Interrupt Table (8 words)
 *  ...     (Default value = 0xeafffffe)
 *  0x 1f
 *  0x 20 - Width Detection
 *         * DEFAULT_WIDTHDETECTION    0xaa995566
 *  0x 24 - Image Identifier
 *         * DEFAULT_IMAGEIDENTIFIER   0x584c4e58
 *  0x 28 - Encryption
 *         * 0x00000000 - None
 *         * 0xa5c3c5a3 - eFuse
 *         * 0x3a5c3c5a - bbRam
 *  0x 2C - User Field
 *  0x 30 - Image Offset
 *  0x 34 - Image Size
 *  0x 38 - Reserved (0x00000000) (according to spec)
 *          * FSBL defines this field for Image Destination Address.
 *  0x 3C - Image Load
 *  0x 40 - Image Stored Size
 *  0x 44 - Reserved (0x00000000) (according to spec)
 *          * FSBL defines this field for QSPI configuration Data.
 *  0x 48 - Checksum
 *  0x 4c - Unused (21 words)
 *  ...
 *  0x 9c
 *  0x a0 - Register Initialization, 256 Address and Data word pairs
 *         * List is terminated with an address of 0xffffffff or
 *  ...    * at the max number of entries
 *  0x89c
 *  0x8a0 - Unused (8 words)
 *  ...
 *  0x8bf
 *  0x8c0 - Data/Image starts here or above
 */

#include "imagetool.h"
#include "mkimage.h"
#include <image.h>

#define HEADER_INTERRUPT_DEFAULT (cpu_to_le32(0xeafffffe))
#define HEADER_REGINIT_NULL (cpu_to_le32(0xffffffff))
#define HEADER_WIDTHDETECTION (cpu_to_le32(0xaa995566))
#define HEADER_IMAGEIDENTIFIER (cpu_to_le32(0x584c4e58))

enum {
	ENCRYPTION_EFUSE = 0xa5c3c5a3,
	ENCRYPTION_BBRAM = 0x3a5c3c5a,
	ENCRYPTION_NONE = 0x0,
};

struct zynq_reginit {
	uint32_t address;
	uint32_t data;
};

#define HEADER_INTERRUPT_VECTORS 8
#define HEADER_REGINITS 256

struct zynq_header {
	uint32_t interrupt_vectors[HEADER_INTERRUPT_VECTORS]; /* 0x0 */
	uint32_t width_detection; /* 0x20 */
	uint32_t image_identifier; /* 0x24 */
	uint32_t encryption; /* 0x28 */
	uint32_t user_field; /* 0x2c */
	uint32_t image_offset; /* 0x30 */
	uint32_t image_size; /* 0x34 */
	uint32_t __reserved1; /* 0x38 */
	uint32_t image_load; /* 0x3c */
	uint32_t image_stored_size; /* 0x40 */
	uint32_t __reserved2; /* 0x44 */
	uint32_t checksum; /* 0x48 */
	uint32_t __reserved3[21]; /* 0x4c */
	struct zynq_reginit register_init[HEADER_REGINITS]; /* 0xa0 */
	uint32_t __reserved4[8]; /* 0x8a0 */
};

static struct zynq_header zynqimage_header;

static uint32_t zynqimage_checksum(struct zynq_header *ptr)
{
	uint32_t checksum = 0;

	if (ptr == NULL)
		return 0;

	checksum += le32_to_cpu(ptr->width_detection);
	checksum += le32_to_cpu(ptr->image_identifier);
	checksum += le32_to_cpu(ptr->encryption);
	checksum += le32_to_cpu(ptr->user_field);
	checksum += le32_to_cpu(ptr->image_offset);
	checksum += le32_to_cpu(ptr->image_size);
	checksum += le32_to_cpu(ptr->__reserved1);
	checksum += le32_to_cpu(ptr->image_load);
	checksum += le32_to_cpu(ptr->image_stored_size);
	checksum += le32_to_cpu(ptr->__reserved2);
	checksum = ~checksum;

	return cpu_to_le32(checksum);
}

static void zynqimage_default_header(struct zynq_header *ptr)
{
	int i;

	if (ptr == NULL)
		return;

	ptr->width_detection = HEADER_WIDTHDETECTION;
	ptr->image_identifier = HEADER_IMAGEIDENTIFIER;
	ptr->encryption = cpu_to_le32(ENCRYPTION_NONE);

	/* Setup not-supported/constant/reserved fields */
	for (i = 0; i < HEADER_INTERRUPT_VECTORS; i++)
		ptr->interrupt_vectors[i] = HEADER_INTERRUPT_DEFAULT;

	for (i = 0; i < HEADER_REGINITS; i++) {
		ptr->register_init[i].address = HEADER_REGINIT_NULL;
		ptr->register_init[i].data = HEADER_REGINIT_NULL;
	}

	/*
	 * Certain reserved fields are required to be set to 0, ensure they are
	 * set as such.
	 */
	ptr->__reserved1 = 0x0;
	ptr->__reserved2 = 0x0;
}

/* mkimage glue functions */
static int zynqimage_verify_header(unsigned char *ptr, int image_size,
		struct image_tool_params *params)
{
	struct zynq_header *zynqhdr = (struct zynq_header *)ptr;

	if (image_size < sizeof(struct zynq_header))
		return -1;

	if (zynqhdr->__reserved1 != 0)
		return -1;

	if (zynqhdr->__reserved2 != 0)
		return -1;

	if (zynqhdr->width_detection != HEADER_WIDTHDETECTION)
		return -1;
	if (zynqhdr->image_identifier != HEADER_IMAGEIDENTIFIER)
		return -1;

	if (zynqimage_checksum(zynqhdr) != zynqhdr->checksum)
		return -1;

	return 0;
}

static void zynqimage_print_header(const void *ptr)
{
	struct zynq_header *zynqhdr = (struct zynq_header *)ptr;
	int i;

	printf("Image Type   : Xilinx Zynq Boot Image support\n");
	printf("Image Offset : 0x%08x\n", le32_to_cpu(zynqhdr->image_offset));
	printf("Image Size   : %lu bytes (%lu bytes packed)\n",
	       (unsigned long)le32_to_cpu(zynqhdr->image_size),
	       (unsigned long)le32_to_cpu(zynqhdr->image_stored_size));
	printf("Image Load   : 0x%08x\n", le32_to_cpu(zynqhdr->image_load));
	printf("User Field   : 0x%08x\n", le32_to_cpu(zynqhdr->user_field));
	printf("Checksum     : 0x%08x\n", le32_to_cpu(zynqhdr->checksum));

	for (i = 0; i < HEADER_INTERRUPT_VECTORS; i++) {
		if (zynqhdr->interrupt_vectors[i] == HEADER_INTERRUPT_DEFAULT)
			continue;

		printf("Modified Interrupt Vector Address [%d]: 0x%08x\n", i,
		       le32_to_cpu(zynqhdr->interrupt_vectors[i]));
	}

	for (i = 0; i < HEADER_REGINITS; i++) {
		if (zynqhdr->register_init[i].address == HEADER_REGINIT_NULL)
			break;

		if (i == 0)
			printf("Custom Register Initialization:\n");

		printf("    @ 0x%08x -> 0x%08x\n",
		       le32_to_cpu(zynqhdr->register_init[i].address),
		       le32_to_cpu(zynqhdr->register_init[i].data));
	}
}

static int zynqimage_check_params(struct image_tool_params *params)
{
	if (!params)
		return 0;

	if (params->addr != 0x0) {
		fprintf(stderr, "Error: Load Address cannot be specified.\n");
		return -1;
	}

	/*
	 * If the entry point is specified ensure it is 64 byte aligned.
	 */
	if (params->eflag && (params->ep % 64 != 0)) {
		fprintf(stderr,
			"Error: Entry Point must be aligned to a 64-byte boundary.\n");
		return -1;
	}

	return !(params->lflag || params->dflag);
}

static int zynqimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_ZYNQIMAGE)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

static void zynqimage_parse_initparams(struct zynq_header *zynqhdr,
	const char *filename)
{
	FILE *fp;
	struct zynq_reginit reginit;
	unsigned int reg_count = 0;
	int r, err;
	struct stat path_stat;

	/* Expect a table of register-value pairs, e.g. "0x12345678 0x4321" */
	fp = fopen(filename, "r");
	if (!fp) {
		fprintf(stderr, "Cannot open initparams file: %s\n", filename);
		exit(1);
	}

	err = fstat(fileno(fp), &path_stat);
	if (err) {
		fclose(fp);
		return;
	}

	if (!S_ISREG(path_stat.st_mode)) {
		fclose(fp);
		return;
	}

	do {
		r = fscanf(fp, "%x %x", &reginit.address, &reginit.data);
		if (r == 2) {
			zynqhdr->register_init[reg_count] = reginit;
			++reg_count;
		}
		r = fscanf(fp, "%*[^\n]\n"); /* Skip to next line */
	} while ((r != EOF) && (reg_count < HEADER_REGINITS));
	fclose(fp);
}

static void zynqimage_set_header(void *ptr, struct stat *sbuf, int ifd,
		struct image_tool_params *params)
{
	struct zynq_header *zynqhdr = (struct zynq_header *)ptr;
	zynqimage_default_header(zynqhdr);

	/* place image directly after header */
	zynqhdr->image_offset =
		cpu_to_le32((uint32_t)sizeof(struct zynq_header));
	zynqhdr->image_size = cpu_to_le32((uint32_t)sbuf->st_size);
	zynqhdr->image_stored_size = zynqhdr->image_size;
	zynqhdr->image_load = 0x0;
	if (params->eflag)
		zynqhdr->image_load = cpu_to_le32((uint32_t)params->ep);

	/* User can pass in text file with init list */
	if (strlen(params->imagename2))
		zynqimage_parse_initparams(zynqhdr, params->imagename2);

	zynqhdr->checksum = zynqimage_checksum(zynqhdr);
}

U_BOOT_IMAGE_TYPE(
	zynqimage,
	"Xilinx Zynq Boot Image support",
	sizeof(struct zynq_header),
	(void *)&zynqimage_header,
	zynqimage_check_params,
	zynqimage_verify_header,
	zynqimage_print_header,
	zynqimage_set_header,
	NULL,
	zynqimage_check_image_types,
	NULL,
	NULL
);
