// SPDX-License-Identifier: BSD-2-Clause
/*
 * Generate Renesas RZ/N1 BootROM header (SPKG)
 * (C) Copyright 2022 Schneider Electric
 *
 * Based on spkg_utility.c
 * (C) Copyright 2016 Renesas Electronics Europe Ltd
 */

#include "imagetool.h"
#include <limits.h>
#include <image.h>
#include <stdarg.h>
#include <stdint.h>
#include <u-boot/crc.h>
#include "renesas_spkgimage.h"

/* Note: the ordering of the bitfields does not matter */
struct config_file {
	unsigned int version:1;
	unsigned int ecc_block_size:2;
	unsigned int ecc_enable:1;
	unsigned int ecc_scheme:3;
	unsigned int ecc_bytes:8;
	unsigned int blp_len;
	unsigned int padding;
};

static struct config_file conf;

static int check_range(const char *name, int val, int min, int max)
{
	if (val < min) {
		fprintf(stderr, "Warning: param '%s' adjusted to min %d\n",
			name, min);
		val = min;
	}

	if (val > max) {
		fprintf(stderr, "Warning: param '%s' adjusted to max %d\n",
			name, max);
		val = max;
	}

	return val;
}

static int spkgimage_parse_config_line(char *line, size_t line_num)
{
	char *saveptr;
	char *delim = "\t ";
	char *name = strtok_r(line, delim, &saveptr);
	char *val_str = strtok_r(NULL, delim, &saveptr);
	int value = atoi(val_str);

	if (!strcmp("VERSION", name)) {
		conf.version = check_range(name, value, 1, 15);
	} else if (!strcmp("NAND_ECC_ENABLE", name)) {
		conf.ecc_enable = check_range(name, value, 0, 1);
	} else if (!strcmp("NAND_ECC_BLOCK_SIZE", name)) {
		conf.ecc_block_size = check_range(name, value, 0, 2);
	} else if (!strcmp("NAND_ECC_SCHEME", name)) {
		conf.ecc_scheme = check_range(name, value, 0, 7);
	} else if (!strcmp("NAND_BYTES_PER_ECC_BLOCK", name)) {
		conf.ecc_bytes = check_range(name, value, 0, 255);
	} else if (!strcmp("ADD_DUMMY_BLP", name)) {
		conf.blp_len = value ? SPKG_BLP_SIZE : 0;
	} else if (!strcmp("PADDING", name)) {
		if (strrchr(val_str, 'K'))
			value = value * 1024;
		else if (strrchr(val_str, 'M'))
			value = value * 1024 * 1024;
		conf.padding = check_range(name, value, 1, INT_MAX);
	} else {
		fprintf(stderr,
			"config error: unknown keyword on line %zu\n",
			line_num);
		return -EINVAL;
	}

	return 0;
}

static int spkgimage_parse_config_file(char *filename)
{
	FILE *fcfg;
	char line[256];
	size_t line_num = 0;

	fcfg = fopen(filename, "r");
	if (!fcfg)
		return -EINVAL;

	while (fgets(line, sizeof(line), fcfg)) {
		line_num += 1;

		/* Skip blank lines and comments */
		if (line[0] == '\n' || line[0] == '#')
			continue;

		/* Strip any trailing newline */
		line[strcspn(line, "\n")] = 0;

		/* Parse the line */
		if (spkgimage_parse_config_line(line, line_num))
			return -EINVAL;
	}

	fclose(fcfg);

	/* Avoid divide-by-zero later on */
	if (!conf.padding)
		conf.padding = 1;

	return 0;
}

static int spkgimage_check_params(struct image_tool_params *params)
{
	if (!params->addr) {
		fprintf(stderr, "Error: Load Address must be set.\n");
		return -EINVAL;
	}

	if (!params->imagename || !params->imagename[0]) {
		fprintf(stderr, "Error: Image name must be set.\n");
		return -EINVAL;
	}

	if (!params->datafile) {
		fprintf(stderr, "Error: Data filename must be set.\n");
		return -EINVAL;
	}

	return 0;
}

static int spkgimage_verify_header(unsigned char *ptr, int size,
				   struct image_tool_params *param)
{
	struct spkg_file *file = (struct spkg_file *)ptr;
	struct spkg_hdr *header = (struct spkg_hdr *)ptr;
	char marker[4] = SPKG_HEADER_MARKER;
	uint32_t payload_length;
	uint32_t crc;
	uint8_t *crc_buf;

	/* Check the marker bytes */
	if (memcmp(header->marker, marker, 4)) {
		if (param->type == IH_TYPE_RENESAS_SPKG)
			fprintf(stderr, "Error: invalid marker bytes\n");
		return -EINVAL;
	}

	/* Check the CRC */
	crc = crc32(0, ptr, SPKG_HEADER_SIZE - SPKG_CRC_SIZE);
	if (crc != header->crc) {
		fprintf(stderr, "Error: invalid header CRC=\n");
		return -EINVAL;
	}

	/* Check all copies of header are the same */
	for (int i = 1; i < SPKG_HEADER_COUNT; i++) {
		if (memcmp(&header[0], &header[i], SPKG_HEADER_SIZE)) {
			fprintf(stderr, "Error: header %d mismatch\n", i);
			return -EINVAL;
		}
	}

	/* Check the payload CRC */
	payload_length = le32_to_cpu(header->payload_length) >> 8;
	crc_buf = file->payload + payload_length - SPKG_CRC_SIZE;
	crc = crc32(0, file->payload, payload_length - SPKG_CRC_SIZE);
	if (crc_buf[0] != (crc & 0xff) ||
	    crc_buf[1] != (crc >> 8 & 0xff) ||
	    crc_buf[2] != (crc >> 16 & 0xff) ||
	    crc_buf[3] != (crc >> 24 & 0xff)) {
		fprintf(stderr, "Error: invalid payload CRC\n");
		return -EINVAL;
	}

	return 0;
}

static void spkgimage_print_header(const void *ptr,
				   struct image_tool_params *image)
{
	const struct spkg_hdr *h = ptr;
	uint32_t offset = le32_to_cpu(h->execution_offset);

	printf("Image type\t: Renesas SPKG Image\n");
	printf("Marker\t\t: %c%c%c%c\n",
	       h->marker[0], h->marker[1], h->marker[2], h->marker[3]);
	printf("Version\t\t: %d\n", h->version);
	printf("ECC\t\t: ");
	if (h->ecc & 0x20)
		printf("Scheme %d, Block size %d, Strength %d\n",
		       h->ecc_scheme, (h->ecc >> 1) & 3, h->ecc_bytes);
	else
		printf("Not enabled\n");
	printf("Payload length\t: %d\n", le32_to_cpu(h->payload_length) >> 8);
	printf("Load address\t: 0x%08x\n", le32_to_cpu(h->load_address));
	printf("Execution offset: 0x%08x (%s mode)\n", offset & ~1,
	       offset & 1 ? "THUMB" : "ARM");
	printf("Header checksum\t: 0x%08x\n", le32_to_cpu(h->crc));
}

/*
 * This is the same as the macro version in include/kernel.h.
 * However we cannot include that header, because for host tools,
 * it ends up pulling in the host /usr/include/linux/kernel.h,
 * which lacks the definition of roundup().
 */
static inline uint32_t roundup(uint32_t x, uint32_t y)
{
	return ((x + y - 1) / y) * y;
}

static int spkgimage_vrec_header(struct image_tool_params *params,
				 struct image_type_params *tparams)
{
	struct stat s;
	struct spkg_file *out_buf;

	/* Parse the config file */
	if (spkgimage_parse_config_file(params->imagename)) {
		fprintf(stderr, "Error parsing config file\n");
		exit(EXIT_FAILURE);
	}

	/* Get size of input data file */
	if (stat(params->datafile, &s)) {
		fprintf(stderr, "Could not stat data file: %s: %s\n",
			params->datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}
	params->orig_file_size = s.st_size;

	/* Determine size of resulting SPKG file */
	uint32_t header_len = SPKG_HEADER_SIZE * SPKG_HEADER_COUNT;
	uint32_t payload_len = conf.blp_len + s.st_size + SPKG_CRC_SIZE;
	uint32_t total_len = header_len + payload_len;

	/* Round up to next multiple of padding size */
	uint32_t padded_len = roundup(total_len, conf.padding);

	/* Number of padding bytes to add */
	conf.padding = padded_len - total_len;

	/* Fixup payload_len to include padding bytes */
	payload_len += conf.padding;

	/* Prepare the header */
	struct spkg_hdr header = {
		.marker = SPKG_HEADER_MARKER,
		.version = conf.version,
		.ecc = (conf.ecc_enable << 5) | (conf.ecc_block_size << 1),
		.ecc_scheme = conf.ecc_scheme,
		.ecc_bytes = conf.ecc_bytes,
		.payload_length = cpu_to_le32(payload_len << 8),
		.load_address = cpu_to_le32(params->addr),
		.execution_offset = cpu_to_le32(params->ep - params->addr),
	};
	header.crc = crc32(0, (uint8_t *)&header,
			   sizeof(header) - SPKG_CRC_SIZE);

	/* The SPKG contains 8 copies of the header */
	out_buf = malloc(sizeof(struct spkg_file));
	if (!out_buf) {
		fprintf(stderr, "Error: Data filename must be set.\n");
		return -ENOMEM;
	}
	tparams->hdr = out_buf;
	tparams->header_size = sizeof(struct spkg_file);

	/* Fill the SPKG with the headers */
	for (int i = 0; i < SPKG_HEADER_COUNT; i++)
		memcpy(&out_buf->header[i], &header, sizeof(header));

	/* Extra bytes to allocate in the output file */
	return conf.blp_len + conf.padding + 4;
}

static void spkgimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				 struct image_tool_params *params)
{
	uint8_t *payload = ptr + SPKG_HEADER_SIZE * SPKG_HEADER_COUNT;
	uint8_t *file_end = payload + conf.blp_len + params->orig_file_size;
	uint8_t *crc_buf = file_end + conf.padding;
	uint32_t crc;

	/* Make room for the Dummy BLp header */
	memmove(payload + conf.blp_len, payload, params->orig_file_size);

	/* Fill the SPKG with the Dummy BLp */
	memset(payload, 0x88, conf.blp_len);

	/*
	 * mkimage copy_file() pads the input file with zeros.
	 * Replace those zeros with flash friendly one bits.
	 * The original version skipped the first 4 bytes,
	 * probably an oversight, but for consistency we
	 * keep the same behaviour.
	 */
	if (conf.padding >= 4)
		memset(file_end + 4, 0xff, conf.padding - 4);

	/* Add Payload CRC */
	crc = crc32(0, payload, crc_buf - payload);
	crc_buf[0] = crc;
	crc_buf[1] = crc >> 8;
	crc_buf[2] = crc >> 16;
	crc_buf[3] = crc >> 24;
}

static int spkgimage_check_image_types(uint8_t type)
{
	return type == IH_TYPE_RENESAS_SPKG ? 0 : -EINVAL;
}

/*
 * spkgimage type parameter definition
 */
U_BOOT_IMAGE_TYPE(
	spkgimage,
	"Renesas SPKG Image",
	0,
	NULL,
	spkgimage_check_params,
	spkgimage_verify_header,
	spkgimage_print_header,
	spkgimage_set_header,
	NULL,
	spkgimage_check_image_types,
	NULL,
	spkgimage_vrec_header
);
