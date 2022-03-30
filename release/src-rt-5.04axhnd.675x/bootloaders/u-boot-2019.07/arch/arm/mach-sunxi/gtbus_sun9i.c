// SPDX-License-Identifier: GPL-2.0+
/*
 * GTBUS initialisation for sun9i
 *
 * (C) Copyright 2016 Theobroma Systems Design und Consulting GmbH
 *                    Philipp Tomsich <philipp.tomsich@theobroma-systems.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gtbus_sun9i.h>
#include <asm/arch/sys_proto.h>

#ifdef CONFIG_SPL_BUILD

void gtbus_init(void)
{
	struct sunxi_gtbus_reg * const gtbus =
		(struct sunxi_gtbus_reg *)SUNXI_GTBUS_BASE;

	/*
	 * We use the same setting that Allwinner used in Boot0 for now.
	 * It may be advantageous to adjust these for various workloads
	 * (e.g. headless use cases that focus on IO throughput).
	 */
	writel((GT_PRIO_HIGH << GT_PORT_FE0) |
	       (GT_PRIO_HIGH << GT_PORT_BE1) |
	       (GT_PRIO_HIGH << GT_PORT_BE2) |
	       (GT_PRIO_HIGH << GT_PORT_IEP0) |
	       (GT_PRIO_HIGH << GT_PORT_FE1) |
	       (GT_PRIO_HIGH << GT_PORT_BE0) |
	       (GT_PRIO_HIGH << GT_PORT_FE2) |
	       (GT_PRIO_HIGH << GT_PORT_IEP1),
	       &gtbus->mst_read_prio_cfg[0]);

	writel(GP_MST_CFG_DEFAULT, &gtbus->mst_cfg[GT_PORT_FE0]);
	writel(GP_MST_CFG_DEFAULT, &gtbus->mst_cfg[GT_PORT_FE0]);
	writel(GP_MST_CFG_DEFAULT, &gtbus->mst_cfg[GT_PORT_BE1]);
	writel(GP_MST_CFG_DEFAULT, &gtbus->mst_cfg[GT_PORT_BE2]);
	writel(GP_MST_CFG_DEFAULT, &gtbus->mst_cfg[GT_PORT_IEP0]);
	writel(GP_MST_CFG_DEFAULT, &gtbus->mst_cfg[GT_PORT_FE1]);
	writel(GP_MST_CFG_DEFAULT, &gtbus->mst_cfg[GT_PORT_BE0]);
	writel(GP_MST_CFG_DEFAULT, &gtbus->mst_cfg[GT_PORT_FE2]);
	writel(GP_MST_CFG_DEFAULT, &gtbus->mst_cfg[GT_PORT_IEP1]);
}

#endif
