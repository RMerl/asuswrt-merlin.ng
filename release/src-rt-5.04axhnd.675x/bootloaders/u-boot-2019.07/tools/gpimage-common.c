// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments Incorporated
 * Refactored common functions in to gpimage-common.c.
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
#include "gpheader.h"

/* Helper to convert size and load_addr to big endian */
void to_be32(uint32_t *gph_size, uint32_t *gph_load_addr)
{
	*gph_size = cpu_to_be32(*gph_size);
	*gph_load_addr = cpu_to_be32(*gph_load_addr);
}

int gph_verify_header(struct gp_header *gph, int be)
{
	uint32_t gph_size = gph->size;
	uint32_t gph_load_addr = gph->load_addr;

	if (be)
		to_be32(&gph_size, &gph_load_addr);

	if (!gph_size || !gph_load_addr)
		return -1;

	return 0;
}

void gph_print_header(const struct gp_header *gph, int be)
{
	uint32_t gph_size = gph->size, gph_load_addr = gph->load_addr;

	if (be)
		to_be32(&gph_size, &gph_load_addr);

	if (!gph_size) {
		fprintf(stderr, "Error: invalid image size %x\n", gph_size);
		exit(EXIT_FAILURE);
	}

	if (!gph_load_addr) {
		fprintf(stderr, "Error: invalid image load address %x\n",
			gph_load_addr);
		exit(EXIT_FAILURE);
	}
	printf("GP Header: Size %x LoadAddr %x\n", gph_size, gph_load_addr);
}

void gph_set_header(struct gp_header *gph, uint32_t size, uint32_t load_addr,
	int be)
{
	gph->size = size;
	gph->load_addr = load_addr;
	if (be)
		to_be32(&gph->size, &gph->load_addr);
}

int gpimage_check_params(struct image_tool_params *params)
{
	return	(params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag));
}
