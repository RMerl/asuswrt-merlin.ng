/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#ifndef _6858_CPU_H
#define _6858_CPU_H

#define BIUCFG_BASE       0x81060000
#define CCI400_BASE       0x81090000

typedef struct CCI400_SlaveIntf {
#define SNOOP_CTRL_ENABLE_SNOOP            0x1
    uint32_t snoop_ctrl;        /* 0x0 */
    uint32_t share_ovr;         /* 0x4 */
    uint32_t rsvd1[62];         /* 0x8 - 0xff */
    uint32_t arqos_ovr;         /* 0x100 */
    uint32_t awqos_ovr;         /* 0x104 */
    uint32_t rsvd2[2];          /* 0x108 - 0x10f */
    uint32_t qos_max_ot;        /* 0x110 */
    uint32_t rsvd3[955];        /* 0x114 - 0xfff */
}CCI400_SlaveIntf;


typedef struct CCI400 {
    uint32_t ctrl_ovr;        /* 0x0 */
    uint32_t spec_ctrl;       /* 0x4 */
#define SECURE_ACCESS_UNSECURE_ENABLE      0x1
    uint32_t secr_acc;        /* 0x8 */
    uint32_t status;          /* 0xc */
    uint32_t impr_err;        /* 0x10 */
    uint32_t rdvd[59];        /* 0x14 */
    uint32_t pmu_ctrl;        /* 0x100 */
    uint32_t rsvd1[959];      /* 0x104 - 0xfff */
    CCI400_SlaveIntf si[5]; /* 0x1000 - 0x5fff */
}CCI400;

#define CCI400 ((volatile CCI400 * const) CCI400_BASE)

typedef struct BIUCFG_Access {
	uint32_t permission;	/* 0x0 */
	uint32_t sbox;		/* 0x4 */
	uint32_t cpu_defeature;	/* 0x8 */
	uint32_t dbg_security;	/* 0xc */
	uint32_t rsvd1[32];	/* 0x10 - 0x8f */
	uint64_t violation[2];	/* 0x90 - 0x9f */
	uint32_t ts_access[2];	/* 0xa0 - 0xa7 */
	uint32_t rsvd2[22];	/* 0xa8 - 0xff */
} BIUCFG_Access;

typedef struct BIUCFG_Cluster {
	uint32_t permission;	/* 0x0 */
	uint32_t config;	/* 0x4 */
	uint32_t status;	/* 0x8 */
	uint32_t control;	/* 0xc */
	uint32_t cpucfg;	/* 0x10 */
	uint32_t dbgrom;	/* 0x14 */
	uint32_t rsvd1[2];	/* 0x18 - 0x1f */
	uint32_t rvbar_addr[4];	/* 0x20 - 0x2f */
	uint32_t rsvd2[52];	/* 0x30 - 0xff */
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
