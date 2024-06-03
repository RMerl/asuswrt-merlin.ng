/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Intel Quark MRC bindings include several properties
 * as part of an Intel Quark MRC node. In most cases,
 * the value of these properties uses the standard values
 * defined in this header.
 */

#ifndef _DT_BINDINGS_QRK_MRC_H_
#define _DT_BINDINGS_QRK_MRC_H_

/* MRC platform data flags */
#define MRC_FLAG_ECC_EN		0x00000001
#define MRC_FLAG_SCRAMBLE_EN	0x00000002
#define MRC_FLAG_MEMTEST_EN	0x00000004
/* 0b DDR "fly-by" topology else 1b DDR "tree" topology */
#define MRC_FLAG_TOP_TREE_EN	0x00000008
/* If set ODR signal is asserted to DRAM devices on writes */
#define MRC_FLAG_WR_ODT_EN	0x00000010

/* DRAM width */
#define DRAM_WIDTH_X8		0
#define DRAM_WIDTH_X16		1
#define DRAM_WIDTH_X32		2

/* DRAM speed */
#define DRAM_FREQ_800		0
#define DRAM_FREQ_1066		1

/* DRAM type */
#define DRAM_TYPE_DDR3		0
#define DRAM_TYPE_DDR3L		1

/* DRAM rank mask */
#define DRAM_RANK(n)		(1 << (n))

/* DRAM channel mask */
#define DRAM_CHANNEL(n)		(1 << (n))

/* DRAM channel width */
#define DRAM_CHANNEL_WIDTH_X8	0
#define DRAM_CHANNEL_WIDTH_X16	1
#define DRAM_CHANNEL_WIDTH_X32	2

/* DRAM address mode */
#define DRAM_ADDR_MODE0		0
#define DRAM_ADDR_MODE1		1
#define DRAM_ADDR_MODE2		2

/* DRAM refresh rate */
#define DRAM_REFRESH_RATE_195US	1
#define DRAM_REFRESH_RATE_39US	2
#define DRAM_REFRESH_RATE_785US	3

/* DRAM SR temprature range */
#define DRAM_SRT_RANGE_NORMAL	0
#define DRAM_SRT_RANGE_EXTENDED	1

/* DRAM ron value */
#define DRAM_RON_34OHM		0
#define DRAM_RON_40OHM		1

/* DRAM rtt nom value */
#define DRAM_RTT_NOM_40OHM	0
#define DRAM_RTT_NOM_60OHM	1
#define DRAM_RTT_NOM_120OHM	2

/* DRAM rd odt value */
#define DRAM_RD_ODT_OFF		0
#define DRAM_RD_ODT_60OHM	1
#define DRAM_RD_ODT_120OHM	2
#define DRAM_RD_ODT_180OHM	3

/* DRAM density */
#define DRAM_DENSITY_512M	0
#define DRAM_DENSITY_1G		1
#define DRAM_DENSITY_2G		2
#define DRAM_DENSITY_4G		3

#endif /* _DT_BINDINGS_QRK_MRC_H_ */
