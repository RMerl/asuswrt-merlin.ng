/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Broadcom Ltd.
 */

#ifndef _6765_CPU_H
#define _6765_CPU_H

#define BOOTLUT_BASE					0xffff0000
#define BIUCFG_BASE						0x81060000
#define UBUS4CCB_RANGE_CHK_SETUP_BASE	0x81203000

typedef struct UBUS4CCB_RANGE_CHK_CFG {
	uint32_t control;	/* 0x00 */
	uint32_t start;		/* 0x04 */
	uint32_t end;		/* 0x08 */
	uint32_t seclev;	/* 0x0c */
	uint32_t secprot;	/* 0x10 */
	uint32_t rsvd[3];	/* 0x14 - 0x1f */
	uint32_t srcpid[8];	/* 0x20 - 0x3f */
}UBUS4CCB_RANGE_CHK_CFG;

typedef struct UBUS4CCB_RANGE_CHK_SETUP {
	uint32_t acc;			/* 0x00 */
	uint32_t ver;			/* 0x04 */
	uint32_t rsvd0[2];		/* 0x08 -0x0f */
	UBUS4CCB_RANGE_CHK_CFG cfg[16];	/* 0x10 - 0x40f */
	uint32_t rsvd1[32];		/* 0x410 -0x48f */
	uint32_t log_inf[3];  	/* 0x490 - 0x49b */
}UBUS4CCB_RANGE_CHK_SETUP;

#define UBUS4CCB_RANGE_CHK_SETUP 		((volatile UBUS4CCB_RANGE_CHK_SETUP * const) UBUS4CCB_RANGE_CHK_SETUP_BASE)

typedef struct BIUCFG_Access {
	uint32_t permission;	/* 0x0 */
	uint32_t rsvd0;			/* 0x4 */
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
	uint64_t rvbar_addr[4];	/* 0x20 - 0x3f */
	uint32_t rsvd2[48];	/* 0x40 - 0xff */
}BIUCFG_Cluster;

typedef struct BIUCFG_Bac {
	uint32_t bac_permission;	/* 0x00 */
	uint32_t bac_periphbase;	/* 0x04 */
	uint32_t rsvd[2];		/* 0x08 - 0x0f */
	uint32_t bac_event;		/* 0x10 */
	uint32_t rsvd_1[3];		/* 0x14 - 0x1f */
	uint32_t bac_ccicfg;		/* 0x20 */
	uint32_t bac_cciaddr;		/* 0x24 */
	uint32_t rsvd_2[4];		/* 0x28 - 0x37 */
	uint32_t bac_ccievs2;		/* 0x38 */
	uint32_t bac_ccievs3;		/* 0x3c */
	uint32_t bac_ccievs4;		/* 0x40 */
	uint32_t rsvd_3[3];		/* 0x44 - 0x4f */
	uint32_t bac_ccievm0;		/* 0x50 */
	uint32_t bac_ccievm1;		/* 0x54 */
	uint32_t rsvd_4[2];		/* 0x58 - 0x5f */
	uint32_t bac_dapapbcfg;		/* 0x60 */
	uint32_t bac_status;		/* 0x64 */
	uint32_t rsvd_5[2];		/* 0x68 - 0x6f */
	uint32_t cpu_therm_irq_cfg; 	/* 0x70 */
	uint32_t cpu_therm_threshold_cfg; /* 0x74 */
	uint32_t rsvd_6;		/* 0x78 */
	uint32_t cpu_therm_temp;	/* 0x7c */
	uint32_t rsvd_7[32];		/* 0x80 - 0xff */
} BIUCFG_Bac;


typedef struct BIUCFG_Aux {
	uint32_t aux_permission;	/* 0x00 */
	uint32_t rsvd[3];	/* 0x04 - 0x0f */
	uint32_t c0_clk_control;	/* 0x10 */
	uint32_t c0_clk_ramp;	/* 0x14 */
	uint32_t c0_clk_pattern;	/* 0x18 */
	uint32_t rsvd_1;	/* 0x1c */
	uint32_t c1_clk_control;	/* 0x20 */
	uint32_t c1_clk_ramp;	/* 0x24 */
	uint32_t c1_clk_pattern;	/* 0x28 */
	uint32_t rsvd_2[53];	/* 0x2c - 0xff */
} BIUCFG_Aux;

typedef struct BIUCFG_CTMR_CTRL {
	uint32_t CTRL;		/* 0x00 */
#define CTMR_CTRL_ENABLE_TMR				0x1
#define CTMR_CTRL_ENABLE_ACCESS				(0xACCE55 << 8)
	uint32_t STAT;		/* 0x04 */
	uint32_t GET_L;		/* 0x08 */
	uint32_t GET_H;		/* 0x0c */
	uint32_t SET_L;		/* 0x10 */
	uint32_t SET_H;		/* 0x14 */
	uint32_t rsvd_0[2];	/* 0x18 - 0x1f */
	uint32_t BFREQ;		/* 0x20 */
	uint32_t rsvd_1[3];	/* 0x24 - 0x2f */
	uint32_t STMR_EN;	/* 0x30 */
	uint32_t rsvd_2[19];	/* 0x34 - 0x7f */
	uint32_t STMR_GET[8];	/* 0x80 - 0x9f*/
	uint32_t rsvd_3[984];	/* 0xa0 - 0xfff */
} BIUCFG_CTMR_CTRL;

typedef struct BIUCFG {
	BIUCFG_Access access;	/* 0x0 - 0xff */
	BIUCFG_Cluster cluster[2];	/* 0x100 - 0x2ff */
	BIUCFG_Bac bac;		/* 0x300 - 0x3ff */
	uint32_t rsvd1[192];	/* 0x400 - 0x6ff */
	BIUCFG_Aux aux;		/* 0x700 - 0x7ff */
	uint32_t rsrvd2[512];	/* 0x800 - 0xfff */
	BIUCFG_CTMR_CTRL ctmr_ctrl;	/* 0x1000 - 0x1fff */
} BIUCFG;

#define BIUCFG ((volatile BIUCFG * const) BIUCFG_BASE)

#endif
