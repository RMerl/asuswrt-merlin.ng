/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#ifndef _BMU_H_
#define _BMU_H_

#define BMU_VERSION		0x000
#define BMU_CTRL		0x004
#define BMU_UCAST_CONFIG	0x008
#define BMU_UCAST_BASE_ADDR	0x00c
#define BMU_BUF_SIZE		0x010
#define BMU_BUF_CNT		0x014
#define BMU_THRES		0x018
#define BMU_INT_SRC		0x020
#define BMU_INT_ENABLE		0x024
#define BMU_ALLOC_CTRL		0x030
#define BMU_FREE_CTRL		0x034
#define BMU_FREE_ERR_ADDR	0x038
#define BMU_CURR_BUF_CNT	0x03c
#define BMU_MCAST_CNT		0x040
#define BMU_MCAST_ALLOC_CTRL	0x044
#define BMU_REM_BUF_CNT		0x048
#define BMU_LOW_WATERMARK	0x050
#define BMU_HIGH_WATERMARK	0x054
#define BMU_INT_MEM_ACCESS	0x100

struct bmu_cfg {
	u32 baseaddr;
	u32 count;
	u32 size;
};

#define BMU1_BUF_SIZE	LMEM_BUF_SIZE_LN2
#define BMU2_BUF_SIZE	DDR_BUF_SIZE_LN2

#endif /* _BMU_H_ */
