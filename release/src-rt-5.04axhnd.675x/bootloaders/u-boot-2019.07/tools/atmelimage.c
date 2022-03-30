// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Andreas Bießmann <andreas@biessmann.org>
 */

#include "imagetool.h"
#include "mkimage.h"

#include <image.h>

#define pr_err(fmt, args...) fprintf(stderr, "atmelimage Error: " fmt, ##args)

static int atmel_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_ATMELIMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static uint32_t nand_pmecc_header[52];

/*
 * A helper struct for parsing the mkimage -n parameter
 *
 * Keep in same order as the configs array!
 */
static struct pmecc_config {
	int use_pmecc;
	int sector_per_page;
	int spare_size;
	int ecc_bits;
	int sector_size;
	int ecc_offset;
} pmecc;

/*
 * Strings used for configure the PMECC header via -n mkimage switch
 *
 * We estimate a coma separated list of key=value pairs. The mkimage -n
 * parameter argument should not contain any whitespace.
 *
 * Keep in same order as struct pmecc_config!
 */
static const char * const configs[] = {
	"usePmecc",
	"sectorPerPage",
	"spareSize",
	"eccBits",
	"sectorSize",
	"eccOffset"
};

static int atmel_find_pmecc_parameter_in_token(const char *token)
{
	size_t pos;
	char *param;

	debug("token: '%s'\n", token);

	for (pos = 0; pos < ARRAY_SIZE(configs); pos++) {
		if (strncmp(token, configs[pos], strlen(configs[pos])) == 0) {
			param = strstr(token, "=");
			if (!param)
				goto err;

			param++;
			debug("\t%s parameter: '%s'\n", configs[pos], param);

			switch (pos) {
			case 0:
				pmecc.use_pmecc = strtol(param, NULL, 10);
				return EXIT_SUCCESS;
			case 1:
				pmecc.sector_per_page = strtol(param, NULL, 10);
				return EXIT_SUCCESS;
			case 2:
				pmecc.spare_size = strtol(param, NULL, 10);
				return EXIT_SUCCESS;
			case 3:
				pmecc.ecc_bits = strtol(param, NULL, 10);
				return EXIT_SUCCESS;
			case 4:
				pmecc.sector_size = strtol(param, NULL, 10);
				return EXIT_SUCCESS;
			case 5:
				pmecc.ecc_offset = strtol(param, NULL, 10);
				return EXIT_SUCCESS;
			}
		}
	}

err:
	pr_err("Could not find parameter in token '%s'\n", token);
	return EXIT_FAILURE;
}

static int atmel_parse_pmecc_params(char *txt)
{
	char *token;

	token = strtok(txt, ",");
	while (token != NULL) {
		if (atmel_find_pmecc_parameter_in_token(token))
			return EXIT_FAILURE;

		token = strtok(NULL, ",");
	}

	return EXIT_SUCCESS;
}

static int atmel_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	uint32_t *ints = (uint32_t *)ptr;
	size_t pos;
	size_t size = image_size;

	/* check if we have an PMECC header attached */
	for (pos = 0; pos < ARRAY_SIZE(nand_pmecc_header); pos++)
		if (ints[pos] >> 28 != 0xC)
			break;

	if (pos == ARRAY_SIZE(nand_pmecc_header)) {
		ints += ARRAY_SIZE(nand_pmecc_header);
		size -= sizeof(nand_pmecc_header);
	}

	/* check the seven interrupt vectors of binary */
	for (pos = 0; pos < 7; pos++) {
		debug("atmelimage: interrupt vector #%zu is 0x%08X\n", pos+1,
		      ints[pos]);
		/*
		 * all vectors except the 6'th one must contain valid
		 * LDR or B Opcode
		 */
		if (pos == 5)
			/* 6'th vector has image size set, check later */
			continue;
		if ((ints[pos] & 0xff000000) == 0xea000000)
			/* valid B Opcode */
			continue;
		if ((ints[pos] & 0xfffff000) == 0xe59ff000)
			/* valid LDR (I=0, P=1, U=1, B=0, W=0, L=1) */
			continue;
		/* ouch, one of the checks has missed ... */
		return 1;
	}

	return ints[5] != cpu_to_le32(size);
}

static void atmel_print_pmecc_header(const uint32_t word)
{
	int val;

	printf("\t\tPMECC header\n");

	printf("\t\t====================\n");

	val = (word >> 18) & 0x1ff;
	printf("\t\teccOffset: %9i\n", val);

	val = (((word >> 16) & 0x3) == 0) ? 512 : 1024;
	printf("\t\tsectorSize: %8i\n", val);

	if (((word >> 13) & 0x7) <= 2)
		val = (2 << ((word >> 13) & 0x7));
	else
		val = (12 << (((word >> 13) & 0x7) - 3));
	printf("\t\teccBitReq: %9i\n", val);

	val = (word >> 4) & 0x1ff;
	printf("\t\tspareSize: %9i\n", val);

	val = (1 << ((word >> 1) & 0x3));
	printf("\t\tnbSectorPerPage: %3i\n", val);

	printf("\t\tusePmecc: %10i\n", word & 0x1);
	printf("\t\t====================\n");
}

static void atmel_print_header(const void *ptr)
{
	uint32_t *ints = (uint32_t *)ptr;
	size_t pos;

	/* check if we have an PMECC header attached */
	for (pos = 0; pos < ARRAY_SIZE(nand_pmecc_header); pos++)
		if (ints[pos] >> 28 != 0xC)
			break;

	if (pos == ARRAY_SIZE(nand_pmecc_header)) {
		printf("Image Type:\tATMEL ROM-Boot Image with PMECC Header\n");
		atmel_print_pmecc_header(ints[0]);
		pos += 5;
	} else {
		printf("Image Type:\tATMEL ROM-Boot Image without PMECC Header\n");
		pos = 5;
	}
	printf("\t\t6'th vector has %u set\n", le32_to_cpu(ints[pos]));
}

static void atmel_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	/* just save the image size into 6'th interrupt vector */
	uint32_t *ints = (uint32_t *)ptr;
	size_t cnt;
	size_t pos = 5;
	size_t size = sbuf->st_size;

	for (cnt = 0; cnt < ARRAY_SIZE(nand_pmecc_header); cnt++)
		if (ints[cnt] >> 28 != 0xC)
			break;

	if (cnt == ARRAY_SIZE(nand_pmecc_header)) {
		pos += ARRAY_SIZE(nand_pmecc_header);
		size -= sizeof(nand_pmecc_header);
	}

	ints[pos] = cpu_to_le32(size);
}

static int atmel_check_params(struct image_tool_params *params)
{
	if (strlen(params->imagename) > 0)
		if (atmel_parse_pmecc_params(params->imagename))
			return EXIT_FAILURE;

	return !(!params->eflag &&
		!params->fflag &&
		!params->xflag &&
		((params->dflag && !params->lflag) ||
		 (params->lflag && !params->dflag)));
}

static int atmel_vrec_header(struct image_tool_params *params,
				struct image_type_params *tparams)
{
	uint32_t tmp;
	size_t pos;

	if (strlen(params->imagename) == 0)
		return EXIT_SUCCESS;

	tmp = 0xC << 28;

	tmp |= (pmecc.ecc_offset & 0x1ff) << 18;

	switch (pmecc.sector_size) {
	case 512:
		tmp |= 0 << 16;
		break;
	case 1024:
		tmp |= 1 << 16;
		break;

	default:
		pr_err("Wrong sectorSize (%i) for PMECC header\n",
		       pmecc.sector_size);
		return EXIT_FAILURE;
	}

	switch (pmecc.ecc_bits) {
	case 2:
		tmp |= 0 << 13;
		break;
	case 4:
		tmp |= 1 << 13;
		break;
	case 8:
		tmp |= 2 << 13;
		break;
	case 12:
		tmp |= 3 << 13;
		break;
	case 24:
		tmp |= 4 << 13;
		break;

	default:
		pr_err("Wrong eccBits (%i) for PMECC header\n",
		       pmecc.ecc_bits);
		 return EXIT_FAILURE;
	}

	tmp |= (pmecc.spare_size & 0x1ff) << 4;

	switch (pmecc.sector_per_page) {
	case 1:
		tmp |= 0 << 1;
		break;
	case 2:
		tmp |= 1 << 1;
		break;
	case 4:
		tmp |= 2 << 1;
		break;
	case 8:
		tmp |= 3 << 1;
		break;

	default:
		pr_err("Wrong sectorPerPage (%i) for PMECC header\n",
		       pmecc.sector_per_page);
		return EXIT_FAILURE;
	}

	if (pmecc.use_pmecc)
		tmp |= 1;

	for (pos = 0; pos < ARRAY_SIZE(nand_pmecc_header); pos++)
		nand_pmecc_header[pos] = tmp;

	debug("PMECC header filled 52 times with 0x%08X\n", tmp);

	tparams->header_size = sizeof(nand_pmecc_header);
	tparams->hdr = nand_pmecc_header;

	return EXIT_SUCCESS;
}

U_BOOT_IMAGE_TYPE(
	atmelimage,
	"ATMEL ROM-Boot Image support",
	0,
	NULL,
	atmel_check_params,
	atmel_verify_header,
	atmel_print_header,
	atmel_set_header,
	NULL,
	atmel_check_image_type,
	NULL,
	atmel_vrec_header
);
