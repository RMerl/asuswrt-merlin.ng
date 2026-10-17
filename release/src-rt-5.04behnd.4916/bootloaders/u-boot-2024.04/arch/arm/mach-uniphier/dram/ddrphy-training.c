// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011-2014 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <time.h>

#include "ddrphy-init.h"
#include "ddrphy-regs.h"

/* for LD4, Pro4, sLD8 */
#define NR_DATX8_PER_DDRPHY	2

void ddrphy_prepare_training(void __iomem *phy_base, int rank)
{
	void __iomem *dx_base = phy_base + PHY_DX_BASE;
	int dx;
	u32 tmp;

	for (dx = 0; dx < NR_DATX8_PER_DDRPHY; dx++) {
		tmp = readl(dx_base + PHY_DX_GCR);
		/* Specify the rank that should be write leveled */
		tmp &= ~PHY_DX_GCR_WLRKEN_MASK;
		tmp |= (1 << (PHY_DX_GCR_WLRKEN_SHIFT + rank)) &
			PHY_DX_GCR_WLRKEN_MASK;
		writel(tmp, dx_base + PHY_DX_GCR);
		dx_base += PHY_DX_STRIDE;
	}

	tmp = readl(phy_base + PHY_DTCR);
	/* Specify the rank used during data bit deskew and eye centering */
	tmp &= ~PHY_DTCR_DTRANK_MASK;
	tmp |= (rank << PHY_DTCR_DTRANK_SHIFT) & PHY_DTCR_DTRANK_MASK;
	/* Use Multi-Purpose Register for DQS gate training */
	tmp |= PHY_DTCR_DTMPR;
	/* Specify the rank enabled for data-training */
	tmp &= ~PHY_DTCR_RANKEN_MASK;
	tmp |= (1 << (PHY_DTCR_RANKEN_SHIFT + rank)) & PHY_DTCR_RANKEN_MASK;
	writel(tmp, phy_base + PHY_DTCR);
}

struct ddrphy_init_sequence {
	char *description;
	u32 init_flag;
	u32 done_flag;
	u32 err_flag;
};

static const struct ddrphy_init_sequence init_sequence[] = {
	{
		"DRAM Initialization",
		PHY_PIR_DRAMRST | PHY_PIR_DRAMINIT,
		PHY_PGSR0_DIDONE,
		PHY_PGSR0_DIERR
	},
	{
		"Write Leveling",
		PHY_PIR_WL,
		PHY_PGSR0_WLDONE,
		PHY_PGSR0_WLERR
	},
	{
		"Read DQS Gate Training",
		PHY_PIR_QSGATE,
		PHY_PGSR0_QSGDONE,
		PHY_PGSR0_QSGERR
	},
	{
		"Write Leveling Adjustment",
		PHY_PIR_WLADJ,
		PHY_PGSR0_WLADONE,
		PHY_PGSR0_WLAERR
	},
	{
		"Read Bit Deskew",
		PHY_PIR_RDDSKW,
		PHY_PGSR0_RDDONE,
		PHY_PGSR0_RDERR
	},
	{
		"Write Bit Deskew",
		PHY_PIR_WRDSKW,
		PHY_PGSR0_WDDONE,
		PHY_PGSR0_WDERR
	},
	{
		"Read Eye Training",
		PHY_PIR_RDEYE,
		PHY_PGSR0_REDONE,
		PHY_PGSR0_REERR
	},
	{
		"Write Eye Training",
		PHY_PIR_WREYE,
		PHY_PGSR0_WEDONE,
		PHY_PGSR0_WEERR
	}
};

int ddrphy_training(void __iomem *phy_base)
{
	int i;
	u32 pgsr0;
	u32 init_flag = PHY_PIR_INIT;
	u32 done_flag = PHY_PGSR0_IDONE;
	int timeout = 50000; /* 50 msec is long enough */
#ifdef DEBUG
	ulong start = get_timer(0);
#endif

	for (i = 0; i < ARRAY_SIZE(init_sequence); i++) {
		init_flag |= init_sequence[i].init_flag;
		done_flag |= init_sequence[i].done_flag;
	}

	writel(init_flag, phy_base + PHY_PIR);

	do {
		if (--timeout < 0) {
			pr_err("timeout during DDR training\n");
			return -ETIMEDOUT;
		}
		udelay(1);
		pgsr0 = readl(phy_base + PHY_PGSR0);
	} while ((pgsr0 & done_flag) != done_flag);

	for (i = 0; i < ARRAY_SIZE(init_sequence); i++) {
		if (pgsr0 & init_sequence[i].err_flag) {
			pr_err("%s failed\n", init_sequence[i].description);
			return -EIO;
		}
	}

#ifdef DEBUG
	pr_debug("DDR training: elapsed time %ld msec\n", get_timer(start));
#endif

	return 0;
}
