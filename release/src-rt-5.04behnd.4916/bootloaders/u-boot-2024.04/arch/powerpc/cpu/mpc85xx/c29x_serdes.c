// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <config.h>
#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

#define SRDS1_MAX_LANES		4

static u32 serdes1_prtcl_map;

struct serdes_config {
	u32 protocol;
	u8 lanes[SRDS1_MAX_LANES];
};

static const struct serdes_config serdes1_cfg_tbl[] = {
	/* SerDes 1 */
	{1, {PCIE1, PCIE1, PCIE1, PCIE1} },
	{2, {PCIE1, PCIE1, PCIE1, PCIE1} },
	{3, {PCIE1, PCIE1, NONE, NONE} },
	{4, {PCIE1, PCIE1, NONE, NONE} },
	{5, {PCIE1, NONE, NONE, NONE} },
	{6, {PCIE1, NONE, NONE, NONE} },
	{}
};

int is_serdes_configured(enum srds_prtcl device)
{
	if (!(serdes1_prtcl_map & (1 << NONE)))
		fsl_serdes_init();

	return (1 << device) & serdes1_prtcl_map;
}

void fsl_serdes_init(void)
{
	ccsr_gur_t *gur = (void *)CFG_SYS_MPC85xx_GUTS_ADDR;
	u32 pordevsr = in_be32(&gur->pordevsr);
	u32 srds_cfg = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >>
				MPC85xx_PORDEVSR_IO_SEL_SHIFT;
	const struct serdes_config *ptr;
	int lane;

	if (serdes1_prtcl_map & (1 << NONE))
		return;

	debug("PORDEVSR[IO_SEL_SRDS] = %x\n", srds_cfg);

	if (srds_cfg > ARRAY_SIZE(serdes1_cfg_tbl)) {
		printf("Invalid PORDEVSR[IO_SEL_SRDS] = %d\n", srds_cfg);
		return;
	}

	ptr = &serdes1_cfg_tbl[srds_cfg];
	if (!ptr->protocol)
		return;

	for (lane = 0; lane < SRDS1_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = ptr->lanes[lane];
		serdes1_prtcl_map |= (1 << lane_prtcl);
	}

	/* Set the first bit to indicate serdes has been initialized */
	serdes1_prtcl_map |= (1 << NONE);
}
