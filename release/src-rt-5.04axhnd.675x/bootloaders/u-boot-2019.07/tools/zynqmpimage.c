// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Michal Simek <michals@xilinx.com>
 * Copyright (C) 2015 Nathan Rossi <nathan@nathanrossi.com>
 *
 * The following Boot Header format/structures and values are defined in the
 * following documents:
 *   * ug1085 ZynqMP TRM doc v1.4 (Chapter 11, Table 11-4)
 *   * ug1137 ZynqMP Software Developer Guide v6.0 (Chapter 16)
 *
 * Expected Header Size = 0x9C0
 * Forced as 'little' endian, 32-bit words
 *
 *  0x  0 - Interrupt table (8 words)
 *  ...     (Default value = 0xeafffffe)
 *  0x 1f
 *  0x 20 - Width detection
 *         * DEFAULT_WIDTHDETECTION    0xaa995566
 *  0x 24 - Image identifier
 *         * DEFAULT_IMAGEIDENTIFIER   0x584c4e58
 *  0x 28 - Encryption
 *         * 0x00000000 - None
 *         * 0xa5c3c5a3 - eFuse
 *         * 0xa5c3c5a7 - obfuscated key in eFUSE
 *         * 0x3a5c3c5a - bbRam
 *         * 0xa35c7ca5 - obfuscated key in boot header
 *  0x 2C - Image load
 *  0x 30 - Image offset
 *  0x 34 - PFW image length
 *  0x 38 - Total PFW image length
 *  0x 3C - Image length
 *  0x 40 - Total image length
 *  0x 44 - Image attributes
 *  0x 48 - Header checksum
 *  0x 4c - Obfuscated key
 *  ...
 *  0x 68
 *  0x 6c - Reserved
 *  0x 70 - User defined
 *  ...
 *  0x 9c
 *  0x a0 - Secure header initialization vector
 *  ...
 *  0x a8
 *  0x ac - Obfuscated key initialization vector
 *  ...
 *  0x b4
 *  0x b8 - Register Initialization, 511 Address and Data word pairs
 *         * List is terminated with an address of 0xffffffff or
 *  ...    * at the max number of entries
 *  0x8b4
 *  0x8b8 - Reserved
 *  ...
 *  0x9bf
 *  0x9c0 - Data/Image starts here or above
 */

#include "imagetool.h"
#include "mkimage.h"
#include "zynqmpimage.h"
#include <image.h>

static struct zynqmp_header zynqmpimage_header;
static void *dynamic_header;
static FILE *fpmu;

static uint32_t zynqmpimage_checksum(struct zynqmp_header *ptr)
{
	uint32_t checksum = 0;

	if (ptr == NULL)
		return 0;

	checksum += le32_to_cpu(ptr->width_detection);
	checksum += le32_to_cpu(ptr->image_identifier);
	checksum += le32_to_cpu(ptr->encryption);
	checksum += le32_to_cpu(ptr->image_load);
	checksum += le32_to_cpu(ptr->image_offset);
	checksum += le32_to_cpu(ptr->pfw_image_length);
	checksum += le32_to_cpu(ptr->total_pfw_image_length);
	checksum += le32_to_cpu(ptr->image_size);
	checksum += le32_to_cpu(ptr->image_stored_size);
	checksum += le32_to_cpu(ptr->image_attributes);
	checksum = ~checksum;

	return cpu_to_le32(checksum);
}

void zynqmpimage_default_header(struct zynqmp_header *ptr)
{
	int i;

	if (ptr == NULL)
		return;

	ptr->width_detection = HEADER_WIDTHDETECTION;
	ptr->image_attributes = HEADER_CPU_SELECT_A53_64BIT;
	ptr->image_identifier = HEADER_IMAGEIDENTIFIER;
	ptr->encryption = cpu_to_le32(ENCRYPTION_NONE);

	/* Setup not-supported/constant/reserved fields */
	for (i = 0; i < HEADER_INTERRUPT_VECTORS; i++)
		ptr->interrupt_vectors[i] = HEADER_INTERRUPT_DEFAULT;

	for (i = 0; i < HEADER_REGINITS; i++) {
		ptr->register_init[i].address = HEADER_REGINIT_NULL;
		ptr->register_init[i].data = 0;
	}

	/*
	 * Certain reserved fields are required to be set to 0, ensure they are
	 * set as such.
	 */
	ptr->pfw_image_length = 0x0;
	ptr->total_pfw_image_length = 0x0;
}

/* mkimage glue functions */
static int zynqmpimage_verify_header(unsigned char *ptr, int image_size,
		struct image_tool_params *params)
{
	struct zynqmp_header *zynqhdr = (struct zynqmp_header *)ptr;

	if (image_size < sizeof(struct zynqmp_header))
		return -1;

	if (zynqhdr->width_detection != HEADER_WIDTHDETECTION)
		return -1;
	if (zynqhdr->image_identifier != HEADER_IMAGEIDENTIFIER)
		return -1;

	if (zynqmpimage_checksum(zynqhdr) != zynqhdr->checksum)
		return -1;

	return 0;
}

static void print_partition(const void *ptr, const struct partition_header *ph)
{
	uint32_t attr = le32_to_cpu(ph->attributes);
	unsigned long len = le32_to_cpu(ph->len) * 4;
	const char *part_owner;
	const char *dest_devs[0x8] = {
		"none", "PS", "PL", "PMU", "unknown", "unknown", "unknown",
		"unknown"
	};

	switch (attr & PART_ATTR_PART_OWNER_MASK) {
	case PART_ATTR_PART_OWNER_FSBL:
		part_owner = "FSBL";
		break;
	case PART_ATTR_PART_OWNER_UBOOT:
		part_owner = "U-Boot";
		break;
	default:
		part_owner = "Unknown";
		break;
	}

	printf("%s payload on CPU %s (%s):\n", part_owner,
	       dest_cpus[(attr & PART_ATTR_DEST_CPU_MASK) >> 8],
	       dest_devs[(attr & PART_ATTR_DEST_DEVICE_MASK) >> 4]);

	printf("    Offset     : 0x%08x\n", le32_to_cpu(ph->offset) * 4);
	printf("    Size       : %lu (0x%lx) bytes\n", len, len);
	printf("    Load       : 0x%08llx",
	       (unsigned long long)le64_to_cpu(ph->load_address));
	if (ph->load_address != ph->entry_point)
		printf(" (entry=0x%08llx)\n",
		       (unsigned long long)le64_to_cpu(ph->entry_point));
	else
		printf("\n");
	printf("    Attributes : ");

	if (attr & PART_ATTR_VEC_LOCATION)
		printf("vec ");

	if (attr & PART_ATTR_ENCRYPTED)
		printf("encrypted ");

	switch (attr & PART_ATTR_CHECKSUM_MASK) {
	case PART_ATTR_CHECKSUM_MD5:
		printf("md5 ");
		break;
	case PART_ATTR_CHECKSUM_SHA2:
		printf("sha2 ");
		break;
	case PART_ATTR_CHECKSUM_SHA3:
		printf("sha3 ");
		break;
	}

	if (attr & PART_ATTR_BIG_ENDIAN)
		printf("BigEndian ");

	if (attr & PART_ATTR_RSA_SIG)
		printf("RSA ");

	if (attr & PART_ATTR_A53_EXEC_AARCH32)
		printf("AArch32 ");

	if (attr & PART_ATTR_TARGET_EL_MASK)
		printf("EL%d ", (attr & PART_ATTR_TARGET_EL_MASK) >> 1);

	if (attr & PART_ATTR_TZ_SECURE)
		printf("secure ");
	printf("\n");

	printf("    Checksum   : 0x%08x\n", le32_to_cpu(ph->checksum));
}

void zynqmpimage_print_header(const void *ptr)
{
	struct zynqmp_header *zynqhdr = (struct zynqmp_header *)ptr;
	int i;

	printf("Image Type   : Xilinx ZynqMP Boot Image support\n");
	printf("Image Offset : 0x%08x\n", le32_to_cpu(zynqhdr->image_offset));
	printf("Image Size   : %lu bytes (%lu bytes packed)\n",
	       (unsigned long)le32_to_cpu(zynqhdr->image_size),
	       (unsigned long)le32_to_cpu(zynqhdr->image_stored_size));

	if (zynqhdr->pfw_image_length)
		printf("PMUFW Size   : %lu bytes (%lu bytes packed)\n",
		       (unsigned long)le32_to_cpu(zynqhdr->pfw_image_length),
		       (unsigned long)le32_to_cpu(
				zynqhdr->total_pfw_image_length));

	printf("Image Load   : 0x%08x\n", le32_to_cpu(zynqhdr->image_load));
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

	if (zynqhdr->image_header_table_offset) {
		struct image_header_table *iht = (void *)ptr +
			zynqhdr->image_header_table_offset;
		struct partition_header *ph;
		uint32_t ph_offset;
		uint32_t next;
		int i;

		ph_offset = le32_to_cpu(iht->partition_header_offset) * 4;
		ph = (void *)ptr + ph_offset;
		for (i = 0; i < le32_to_cpu(iht->nr_parts); i++) {
			next = le32_to_cpu(ph->next_partition_offset) * 4;

			/* Partition 0 is the base image itself */
			if (i)
				print_partition(ptr, ph);

			ph = (void *)ptr + next;
		}
	}

	free(dynamic_header);
}

static int zynqmpimage_check_params(struct image_tool_params *params)
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

static int zynqmpimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_ZYNQMPIMAGE)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

static uint32_t fsize(FILE *fp)
{
	int size, ret, origin;

	origin = ftell(fp);
	if (origin < 0) {
		fprintf(stderr, "Incorrect file size\n");
		fclose(fp);
		exit(2);
	}

	ret = fseek(fp, 0L, SEEK_END);
	if (ret) {
		fprintf(stderr, "Incorrect file SEEK_END\n");
		fclose(fp);
		exit(3);
	}

	size = ftell(fp);
	if (size < 0) {
		fprintf(stderr, "Incorrect file size\n");
		fclose(fp);
		exit(4);
	}

	/* going back */
	ret = fseek(fp, origin, SEEK_SET);
	if (ret) {
		fprintf(stderr, "Incorrect file SEEK_SET to %d\n", origin);
		fclose(fp);
		exit(3);
	}

	return size;
}

static void zynqmpimage_pmufw(struct zynqmp_header *zynqhdr,
			      const char *filename)
{
	uint32_t size;

	/* Setup PMU fw size */
	zynqhdr->pfw_image_length = fsize(fpmu);
	zynqhdr->total_pfw_image_length = zynqhdr->pfw_image_length;

	zynqhdr->image_size -= zynqhdr->pfw_image_length;
	zynqhdr->image_stored_size -= zynqhdr->total_pfw_image_length;

	/* Read the whole PMUFW to the header */
	size = fread(&zynqhdr->__reserved4[66], 1,
		     zynqhdr->pfw_image_length, fpmu);
	if (size != zynqhdr->pfw_image_length) {
		fprintf(stderr, "Cannot read PMUFW file: %s\n", filename);
		fclose(fpmu);
		exit(1);
	}

	fclose(fpmu);
}

static void zynqmpimage_parse_initparams(struct zynqmp_header *zynqhdr,
	const char *filename)
{
	FILE *fp;
	struct zynqmp_reginit reginit;
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

static void zynqmpimage_set_header(void *ptr, struct stat *sbuf, int ifd,
		struct image_tool_params *params)
{
	struct zynqmp_header *zynqhdr = (struct zynqmp_header *)ptr;
	zynqmpimage_default_header(zynqhdr);

	/* place image directly after header */
	zynqhdr->image_offset =
		cpu_to_le32((uint32_t)sizeof(struct zynqmp_header));
	zynqhdr->image_size = cpu_to_le32(params->file_size -
					  sizeof(struct zynqmp_header));
	zynqhdr->image_stored_size = zynqhdr->image_size;
	zynqhdr->image_load = 0xfffc0000;
	if (params->eflag)
		zynqhdr->image_load = cpu_to_le32((uint32_t)params->ep);

	/* PMUFW */
	if (fpmu)
		zynqmpimage_pmufw(zynqhdr, params->imagename);

	/* User can pass in text file with init list */
	if (strlen(params->imagename2))
		zynqmpimage_parse_initparams(zynqhdr, params->imagename2);

	zynqhdr->checksum = zynqmpimage_checksum(zynqhdr);
}

static int zynqmpimage_vrec_header(struct image_tool_params *params,
				   struct image_type_params *tparams)
{
	struct stat path_stat;
	char *filename = params->imagename;
	int err;

	/* Handle static case without PMUFW */
	tparams->header_size = sizeof(struct zynqmp_header);
	tparams->hdr = (void *)&zynqmpimage_header;

	/* PMUFW name is passed via params->imagename */
	if (strlen(filename) == 0)
		return EXIT_SUCCESS;

	fpmu = fopen(filename, "r");
	if (!fpmu) {
		fprintf(stderr, "Cannot open PMUFW file: %s\n", filename);
		return EXIT_FAILURE;
	}

	err = fstat(fileno(fpmu), &path_stat);
	if (err) {
		fclose(fpmu);
		fpmu = NULL;
		return EXIT_FAILURE;
	}

	if (!S_ISREG(path_stat.st_mode)) {
		fclose(fpmu);
		fpmu = NULL;
		return EXIT_FAILURE;
	}

	/* Increase header size by PMUFW file size */
	tparams->header_size += fsize(fpmu);

	/* Allocate buffer with space for PMUFW */
	dynamic_header = calloc(1, tparams->header_size);
	tparams->hdr = dynamic_header;

	return EXIT_SUCCESS;
}

U_BOOT_IMAGE_TYPE(
	zynqmpimage,
	"Xilinx ZynqMP Boot Image support",
	sizeof(struct zynqmp_header),
	(void *)&zynqmpimage_header,
	zynqmpimage_check_params,
	zynqmpimage_verify_header,
	zynqmpimage_print_header,
	zynqmpimage_set_header,
	NULL,
	zynqmpimage_check_image_types,
	NULL,
	zynqmpimage_vrec_header
);
