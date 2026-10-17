/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * GTBUS initialisation for sun9i
 *
 * (C) Copyright 2016 Theobroma Systems Design und Consulting GmbH
 *                    Philipp Tomsich <philipp.tomsich@theobroma-systems.com>
 */

#ifndef _SUNXI_GTBUS_SUN9I_H
#define _SUNXI_GTBUS_SUN9I_H

#include <linux/types.h>

struct sunxi_gtbus_reg {
	u32 mst_cfg[36];           /* 0x000 */
	u8  reserved1[0x70];       /* 0x090 */
	u32 bw_wdw_cfg;            /* 0x100 */
	u32 mst_read_prio_cfg[2];  /* 0x104 */
	u32 lvl2_mst_cfg;          /* 0x10c */
	u32 sw_clk_on;             /* 0x110 */
	u32 sw_clk_off;            /* 0x114 */
	u32 pmu_mst_en;            /* 0x118 */
	u32 pmu_cfg;               /* 0x11c */
	u32 pmu_cnt[19];           /* 0x120 */
	u32 reserved2[0x94];       /* 0x16c */
	u32 cci400_config[3];      /* 0x200 */
	u32 cci400_status[2];      /* 0x20c */
};

/* for register GT_MST_CFG_REG(n) */
#define GT_ENABLE_REQ           (1<<31) /* clock on */
#define GT_DISABLE_REQ          (1<<30) /* clock off */
#define GT_QOS_SHIFT            28
#define GT_THD1_SHIFT           16
#define GT_REQN_MAX             0xf /* max no master requests in one cycle */
#define GT_REQN_SHIFT           12
#define GT_THD0_SHIFT           0

#define GT_QOS_MAX              0x3
#define GT_THD_MAX              0xfff
#define GT_BW_WDW_MAX           0xffff

/* mst_read_prio_cfg */
#define GT_PRIO_LOW     0
#define GT_PRIO_HIGH    1

/* GTBUS port ids */
#define GT_PORT_CPUM1   0
#define GT_PORT_CPUM2   1
#define GT_PORT_SATA    2
#define	GT_PORT_USB3    3
#define	GT_PORT_FE0     4
#define	GT_PORT_BE1     5
#define	GT_PORT_BE2     6
#define	GT_PORT_IEP0    7
#define	GT_PORT_FE1     8
#define	GT_PORT_BE0     9
#define	GT_PORT_FE2     10
#define	GT_PORT_IEP1    11
#define	GT_PORT_VED     12
#define	GT_PORT_VEE     13
#define	GT_PORT_FD      14
#define	GT_PORT_CSI     15
#define	GT_PORT_MP      16
#define	GT_PORT_HSI     17
#define	GT_PORT_SS      18
#define	GT_PORT_TS      19
#define	GT_PORT_DMA     20
#define	GT_PORT_NDFC0   21
#define	GT_PORT_NDFC1   22
#define	GT_PORT_CPUS    23
#define	GT_PORT_TH      24
#define	GT_PORT_GMAC    25
#define	GT_PORT_USB0    26
#define	GT_PORT_MSTG0   27
#define	GT_PORT_MSTG1   28
#define	GT_PORT_MSTG2   29
#define	GT_PORT_MSTG3   30
#define	GT_PORT_USB1    31
#define	GT_PORT_GPU0    32
#define	GT_PORT_GPU1    33
#define	GT_PORT_USB2    34
#define	GT_PORT_CPUM0   35

#define GP_MST_CFG_DEFAULT \
	((GT_QOS_MAX << GT_QOS_SHIFT)   | \
	 (GT_THD_MAX << GT_THD1_SHIFT)  | \
	 (GT_REQN_MAX << GT_REQN_SHIFT) | \
	 (GT_THD_MAX << GT_THD0_SHIFT))

#endif
