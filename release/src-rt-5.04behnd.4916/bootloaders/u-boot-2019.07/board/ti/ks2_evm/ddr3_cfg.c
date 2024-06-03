// SPDX-License-Identifier: GPL-2.0+
/*
 * Keystone2: DDR3 configuration
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>

#include <asm/arch/ddr3.h>
#include "ddr3_cfg.h"

struct ddr3_phy_config ddr3phy_1600_2g = {
	.pllcr          = 0x0001C000ul,
	.pgcr1_mask     = (IODDRM_MASK | ZCKSEL_MASK),
	.pgcr1_val      = ((1 << 2) | (1 << 7) | (1 << 23)),
	.ptr0           = 0x42C21590ul,
	.ptr1           = 0xD05612C0ul,
	.ptr2           = 0, /* not set in gel */
	.ptr3           = 0x0D861A80ul,
	.ptr4           = 0x0C827100ul,
	.dcr_mask       = (PDQ_MASK | MPRDQ_MASK | BYTEMASK_MASK),
	.dcr_val        = ((1 << 10)),
	.dtpr0          = 0x9D5CBB66ul,
	.dtpr1          = 0x12868300ul,
	.dtpr2          = 0x5002D200ul,
	.mr0            = 0x00001C70ul,
	.mr1            = 0x00000006ul,
	.mr2            = 0x00000018ul,
	.dtcr           = 0x710035C7ul,
	.pgcr2          = 0x00F07A12ul,
	.zq0cr1         = 0x0001005Dul,
	.zq1cr1         = 0x0001005Bul,
	.zq2cr1         = 0x0001005Bul,
	.pir_v1         = 0x00000033ul,
	.pir_v2         = 0x0000FF81ul,
};

struct ddr3_emif_config ddr3_1600_2g = {
	.sdcfg          = 0x6200CE62ul,
	.sdtim1         = 0x166C9855ul,
	.sdtim2         = 0x00001D4Aul,
	.sdtim3         = 0x435DFF53ul,
	.sdtim4         = 0x543F0CFFul,
	.zqcfg          = 0x70073200ul,
	.sdrfc          = 0x00001869ul,
};
