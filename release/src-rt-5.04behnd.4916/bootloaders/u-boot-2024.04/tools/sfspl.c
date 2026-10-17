// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright Heinrich Schuchardt <heinrich.schuchardt@canonical.com>
 *
 * The StarFive JH7110 requires to prepend a header to u-boot-spl.bin describing
 * the payload length and CRC32.
 *
 * This module implements support in mkimage and dumpimage for this file format.
 *
 * StarFive's spl_tool available under GPL-2.0-and-later at
 * https://github.com/starfive-tech/Tools implements writing the same file
 * format and served as a reference.
 */

#include <compiler.h>
#include <fcntl.h>
#include <u-boot/crc.h>
#include <unistd.h>
#include "imagetool.h"

#define DEFAULT_VERSION 0x01010101
#define DEFAULT_BACKUP 0x200000U
#define DEFAULT_OFFSET 0x240

/**
 * struct spl_hdr - header for SPL on JH7110
 *
 * All fields are low-endian.
 */
struct spl_hdr {
	/** @offset:	offset to SPL header (0x240) */
	unsigned int offset;
	/** @bkp_offs:	address of backup SPL, defaults to DEFAULT_BACKUP */
	unsigned int bkp_offs;
	/** @zero1:	set to zero */
	unsigned int zero1[159];
	/** @version:	header version, defaults to DEFAULT_VERSION */
	unsigned int version;
	/** @file_size:	file size */
	unsigned int file_size;
	/** @hdr_size:	size of the file header (0x400) */
	unsigned int hdr_size;
	/** @crc32:	CRC32 */
	unsigned int crc32;
	/** @zero2:	set to zero */
	unsigned int zero2[91];
};

static int sfspl_check_params(struct image_tool_params *params)
{
	/* Only the RISC-V architecture is supported */
	if (params->Aflag && params->arch != IH_ARCH_RISCV)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

static int sfspl_verify_header(unsigned char *buf, int size,
			       struct image_tool_params *params)
{
	struct spl_hdr *hdr = (void *)buf;
	unsigned int hdr_size = le32_to_cpu(hdr->hdr_size);
	unsigned int file_size = le32_to_cpu(hdr->file_size);
	unsigned int crc = le32_to_cpu(hdr->crc32);
	unsigned int crc_check;

	if (size < 0 ||
	    (size_t)size < sizeof(struct spl_hdr) ||
	    (size_t)size < hdr_size + file_size) {
		printf("Truncated file\n");
		return EXIT_FAILURE;
	}
	if (hdr->version != DEFAULT_VERSION) {
		printf("Unknown file format version\n");
		return EXIT_FAILURE;
	}
	crc_check = crc32(0, &buf[hdr_size], size - hdr_size);
	if (crc_check != crc) {
		printf("Incorrect CRC32\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static void sfspl_print_header(const void *buf,
			       struct image_tool_params *params)
{
	struct spl_hdr *hdr = (void *)buf;
	unsigned int hdr_size = le32_to_cpu(hdr->hdr_size);
	unsigned int file_size = le32_to_cpu(hdr->file_size);

	printf("Header size: %u\n", hdr_size);
	printf("Payload size: %u\n", file_size);
}

static int sfspl_image_extract_subimage(void *ptr,
					struct image_tool_params *params)
{
	struct spl_hdr *hdr = (void *)ptr;
	unsigned char *buf = ptr;
	int fd, ret = EXIT_SUCCESS;
	unsigned int hdr_size = le32_to_cpu(hdr->hdr_size);
	unsigned int file_size = le32_to_cpu(hdr->file_size);

	if (params->pflag) {
		printf("Invalid image index %d\n", params->pflag);
		return EXIT_FAILURE;
	}

	fd = open(params->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1) {
		perror("Cannot open file");
		return EXIT_FAILURE;
	}
	if (write(fd, &buf[hdr_size], file_size) != file_size) {
		perror("Cannot write file");
		ret = EXIT_FAILURE;
	}
	close(fd);

	return ret;
}

static int sfspl_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_STARFIVE_SPL)
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}

static void sfspl_set_header(void *buf, struct stat *sbuf, int infd,
			     struct image_tool_params *params)
{
	struct spl_hdr *hdr = buf;
	unsigned int file_size;
	unsigned int crc;

	file_size = params->file_size - sizeof(struct spl_hdr);
	crc = crc32(0, &((unsigned char *)buf)[sizeof(struct spl_hdr)],
		    file_size);

	hdr->offset = cpu_to_le32(DEFAULT_OFFSET);
	hdr->bkp_offs = cpu_to_le32(DEFAULT_BACKUP);
	hdr->version = cpu_to_le32(DEFAULT_VERSION);
	hdr->file_size = cpu_to_le32(file_size);
	hdr->hdr_size = cpu_to_le32(sizeof(struct spl_hdr));
	hdr->crc32 = cpu_to_le32(crc);
}

static int sfspl_vrec_header(struct image_tool_params *params,
			     struct image_type_params *tparams)
{
	tparams->hdr = calloc(sizeof(struct spl_hdr), 1);

	/* No padding */
	return 0;
}

U_BOOT_IMAGE_TYPE(
	sfspl, /* id */
	"StarFive SPL Image", /* name */
	sizeof(struct spl_hdr), /* header_size */
	NULL, /* header */
	sfspl_check_params, /* check_params */
	sfspl_verify_header, /* verify header */
	sfspl_print_header, /* print header */
	sfspl_set_header, /* set header */
	sfspl_image_extract_subimage, /* extract_subimage */
	sfspl_check_image_type, /* check_image_type */
	NULL, /* fflag_handle */
	sfspl_vrec_header /* vrec_header */
);
