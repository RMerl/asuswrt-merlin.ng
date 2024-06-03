/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014
 * Texas Instruments Incorporated
 * Refactored common functions in to gpimage-common.c. Include this common
 * header file
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

#ifndef _GPIMAGE_H_
#define _GPIMAGE_H_

/* common headers for gpimage and omapimage formats */
struct gp_header {
	uint32_t size;
	uint32_t load_addr;
};
#define GPIMAGE_HDR_SIZE (sizeof(struct gp_header))

/* common functions across gpimage and omapimage handlers */
int valid_gph_size(uint32_t size);
int valid_gph_load_addr(uint32_t load_addr);
int gph_verify_header(struct gp_header *gph, int be);
void gph_print_header(const struct gp_header *gph, int be);
void gph_set_header(struct gp_header *gph, uint32_t size, uint32_t load_addr,
			int be);
int gpimage_check_params(struct image_tool_params *params);
#endif
