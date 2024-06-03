// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/immap_ls102xa.h>
#include <linux/errno.h>
#include <asm/io.h>
#include "fsl_ls1_serdes.h"

#ifdef CONFIG_SYS_FSL_SRDS_1
static u64 serdes1_prtcl_map;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
static u64 serdes2_prtcl_map;
#endif

int is_serdes_configured(enum srds_prtcl device)
{
	u64 ret = 0;

#ifdef CONFIG_SYS_FSL_SRDS_1
	if (!(serdes1_prtcl_map & (1ULL << NONE)))
		fsl_serdes_init();

	ret |= (1ULL << device) & serdes1_prtcl_map;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	if (!(serdes2_prtcl_map & (1ULL << NONE)))
		fsl_serdes_init();

	ret |= (1ULL << device) & serdes2_prtcl_map;
#endif

	return !!ret;
}

int serdes_get_first_lane(u32 sd, enum srds_prtcl device)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 cfg = in_be32(&gur->rcwsr[4]);
	int i;

	switch (sd) {
#ifdef CONFIG_SYS_FSL_SRDS_1
	case FSL_SRDS_1:
		cfg &= RCWSR4_SRDS1_PRTCL_MASK;
		cfg >>= RCWSR4_SRDS1_PRTCL_SHIFT;
		break;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	case FSL_SRDS_2:
		cfg &= RCWSR4_SRDS2_PRTCL_MASK;
		cfg >>= RCWSR4_SRDS2_PRTCL_SHIFT;
		break;
#endif
	default:
		printf("invalid SerDes%d\n", sd);
		break;
	}
	/* Is serdes enabled at all? */
	if (unlikely(cfg == 0))
		return -ENODEV;

	for (i = 0; i < SRDS_MAX_LANES; i++) {
		if (serdes_get_prtcl(sd, cfg, i) == device)
			return i;
	}

	return -ENODEV;
}

u64 serdes_init(u32 sd, u32 sd_addr, u32 sd_prctl_mask, u32 sd_prctl_shift)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u64 serdes_prtcl_map = 0;
	u32 cfg;
	int lane;

	cfg = in_be32(&gur->rcwsr[4]) & sd_prctl_mask;
	cfg >>= sd_prctl_shift;
	printf("Using SERDES%d Protocol: %d (0x%x)\n", sd + 1, cfg, cfg);

	if (!is_serdes_prtcl_valid(sd, cfg))
		printf("SERDES%d[PRTCL] = 0x%x is not valid\n", sd + 1, cfg);

	for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes_get_prtcl(sd, cfg, lane);

		serdes_prtcl_map |= (1ULL << lane_prtcl);
	}

	/* Set the first bit to indicate serdes has been initialized */
	serdes_prtcl_map |= (1ULL << NONE);

	return serdes_prtcl_map;
}

void fsl_serdes_init(void)
{
#ifdef CONFIG_SYS_FSL_SRDS_1
	if (!(serdes1_prtcl_map & (1ULL << NONE)))
		serdes1_prtcl_map = serdes_init(FSL_SRDS_1,
					CONFIG_SYS_FSL_SERDES_ADDR,
					RCWSR4_SRDS1_PRTCL_MASK,
					RCWSR4_SRDS1_PRTCL_SHIFT);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	if (!(serdes2_prtcl_map & (1ULL << NONE)))
		serdes2_prtcl_map = serdes_init(FSL_SRDS_2,
					CONFIG_SYS_FSL_SERDES_ADDR +
					FSL_SRDS_2 * 0x1000,
					RCWSR4_SRDS2_PRTCL_MASK,
					RCWSR4_SRDS2_PRTCL_SHIFT);
#endif
}

const char *serdes_clock_to_string(u32 clock)
{
	switch (clock) {
	case SRDS_PLLCR0_RFCK_SEL_100:
		return "100";
	case SRDS_PLLCR0_RFCK_SEL_125:
		return "125";
	default:
		return "100";
	}
}
