// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Author: Zhuoyu Zhang <Zhuoyu.Zhang@freescale.com>
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch-ls102xa/immap_ls102xa.h>
#include <asm/arch-ls102xa/config.h>
#include <linux/compiler.h>
#include <hwconfig.h>
#include <fsl_devdis.h>

void device_disable(const struct devdis_table *tbl, uint32_t num)
{
	int i;
	struct ccsr_gur __iomem *gur = (void *)CFG_SYS_FSL_GUTS_ADDR;

	/*
	 * Extract hwconfig from environment and disable unused device.
	 */
	for (i = 0; i < num; i++) {
		if (hwconfig_sub("devdis", tbl[i].name))
			setbits_be32(&gur->devdisr + tbl[i].offset,
				tbl[i].mask);
	}
}
