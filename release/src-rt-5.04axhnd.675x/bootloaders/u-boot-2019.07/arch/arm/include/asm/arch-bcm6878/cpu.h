/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#ifndef _6878_CPU_H
#define _6878_CPU_H

#define BOOTLUT_BASE      0xffff0000
#define BIUCFG_BASE       0x81060000

typedef struct BIUCFG_Access {
	uint32_t permission;	/* 0x0 */
	uint32_t reserved0;		/* 0x4 */
	uint32_t cpu_defeature;	/* 0x8 */
	uint32_t dbg_security;	/* 0xc */
	uint32_t rsvd1[36];	/* 0x10 - 0x9f */
	uint32_t ts_access;	/* 0xa0 - 0xa3 */
	uint32_t rsvd2[23];	/* 0xa4 - 0xff */
} BIUCFG_Access;

typedef struct BIUCFG_Cluster {
	uint32_t permission;	/* 0x0 */
	uint32_t config;	/* 0x4 */
	uint32_t status;	/* 0x8 */
	uint32_t control;	/* 0xc */
	uint32_t cpucfg;	/* 0x10 */
	uint32_t dbgrom;	/* 0x14 */
	uint32_t rsvd1[2];	/* 0x18 - 0x1f */
	uint32_t rvbar_addr[8];	/* 0x20 - 0x3f */
	uint32_t rsvd2[48];	/* 0x40 - 0xff */
} BIUCFG_Cluster;

typedef struct BIUCFG_AuxClkCtrl {
	uint32_t clk_control;	/* 0x0 */
	uint32_t clk_ramp;	/* 0x4 */
	uint32_t clk_pattern;	/* 0x8 */
	uint32_t rsvd;		/* 0xC */
} BIUCFG_AuxClkCtrl;

typedef struct BIUCFGux {
	uint32_t permission;	/* 0 */
	uint32_t rsvd1[3];	/* 0x04 - 0x0c */
	BIUCFG_AuxClkCtrl cluster_clkctrl[2];	/* 0x10 - 0x2c */
	uint32_t rsvd2[52];	/* 0x30 - 0xFF */
} BIUCFG_Aux;

typedef struct BIUCFG {
	BIUCFG_Access access;	/* 0x0 - 0xff */
	BIUCFG_Cluster cluster[1];	/* 0x100 - 0x1ff */
	uint32_t rsvd1[320];	/* 0x200 - 0x6ff */
	BIUCFG_Aux aux;		/* 0x700 - 0x7ff */
	uint32_t rsrvd2[512];	/* 0x800 - 0xfff */
	uint32_t TSO_CNTCR;	/* 0x1000 */
	uint32_t rsvd2[2047];	/* 0x1004 - 0x2fff */
} BIUCFG;

#define BIUCFG ((volatile BIUCFG * const) BIUCFG_BASE)

#endif
