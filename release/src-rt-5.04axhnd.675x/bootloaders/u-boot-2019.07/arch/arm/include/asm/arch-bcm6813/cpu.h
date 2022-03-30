/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#ifndef _6813_CPU_H
#define _6813_CPU_H

#include <common.h>


typedef struct CCI500_SlaveIntf {
	#define SNOOP_CTRL_ENABLE_SNOOP            0x1
	uint32_t snoop_ctrl;	/* 0x0 */
	#define SHARE_OVR_SHAREABLE_OVR_SHIFT      0x0
	#define SHARE_OVR_SHAREABLE_OVR_MASK       0x3
	#define SHARE_OVR_SHAREABLE_OVR_NONSHR     0x2
	#define SHARE_OVR_SHAREABLE_OVR_SHR        0x3
	uint32_t share_ovr;	/* 0x4 */
	uint32_t rsvd1[62];	/* 0x8 - 0xff */
	uint32_t arqos_ovr;	/* 0x100 */
	uint32_t awqos_ovr;	/* 0x104 */
	uint32_t rsvd2[2];	/* 0x108 - 0x10f */
	uint32_t qos_max_ot;	/* 0x110 */
	uint32_t rsvd3[955];	/* 0x114 - 0xfff */
}CCI500_SlaveIntf;

typedef struct CCI500_EventCounter {
	uint32_t sel;	/* 0x0 */
	uint32_t data;	/* 0x4 */
	uint32_t ctrl;	/* 0x8 */
	uint32_t clr_ovfl;	/* 0xC */
	uint32_t rsvd[16380];	/* 0x10 - 0xffff */
}CCI500_EventCounter;

typedef struct CCI500 {
	#define CONTROL_OVERRIDE_SNOOP_DISABLE     0x1
	#define CONTROL_OVERRIDE_SNOOP_FLT_DISABLE 0x4
	uint32_t ctrl_ovr;	/* 0x0 */
	uint32_t rsvd1;	/* 0x4 */
	#define SECURE_ACCESS_UNSECURE_ENABLE      0x1
	uint32_t secr_acc;	/* 0x8 */
	uint32_t status;	/* 0xc */
	#define STATUS_CHANGE_PENDING              0x1
	uint32_t impr_err;	/* 0x10 */
	uint32_t qos_threshold;	/* 0x14 */
	uint32_t rsvd2[58];	/* 0x18 - 0xff */
	uint32_t pmu_ctrl;	/* 0x100 */
	#define DBG_CTRL_EN_INTF_MON               0x1
	uint32_t debug_ctrl;	/* 0x104 */
	uint32_t rsvd3[958];	/* 0x108 - 0xfff */
	#define SLAVEINTF_COHERENCY_PORT           0x0
	#define SLAVEINTF_CPU_CLUSTER              0x1
	CCI500_SlaveIntf si[7];	/* 0x1000 - 0x7fff */
	uint32_t rsvd4[8192];	/* 0x8000 - 0xffff */
	CCI500_EventCounter evt_cntr[8];	/* 0x10000 - 0x8ffff */
}CCI500;

#define CCI500_BASE	0x81100000
#define CCI500 ((volatile CCI500 * const) CCI500_BASE)

typedef struct UBUS4_RANGE_CHK_CFG {
	uint32_t control;	/* 0x0 */
	uint32_t srcpid[8];	/* 0x4 - 0x23 */
	uint32_t seclev;	/* 0x24 */
	uint32_t base;		/* 0x28 */
	uint32_t base_up;  	/* 0x2c */
}UBUS4_RANGE_CHK_CFG;

typedef struct UBUS4_RANGE_CHK_SETUP {
	uint32_t lock;			/* 0x0 */
	uint32_t log_inf[3];  		/* 0x4 - 0xf */
	UBUS4_RANGE_CHK_CFG cfg[16];	/* 0x10 - 0x30f */
}UBUS4_RANGE_CHK_SETUP;

#define UBUS4_COHERENCY_PORT_BASE	0x810A0000
#define UBUS4_RANGE_CHK_SETUP_OFFSET    0x0
#define UBUS4_RANGE_CHK_SETUP_BASE	(UBUS4_COHERENCY_PORT_BASE+UBUS4_RANGE_CHK_SETUP_OFFSET)
#define UBUS4_RANGE_CHK_SETUP 		((volatile UBUS4_RANGE_CHK_SETUP * const) UBUS4_RANGE_CHK_SETUP_BASE)

typedef struct BIUCFG_Access {
	uint32_t permission;	/* 0x0 */
	uint32_t sbox;	/* 0x4 */
	uint32_t cpu_defeature;	/* 0x8 */
	uint32_t dbg_security;	/* 0xc */
	uint32_t rsvd1[32];	/* 0x10 - 0x8f */
	uint64_t violation[2];	/* 0x90 - 0x9f */
	uint32_t ts_access[2];	/* 0xa0 - 0xa7 */
	uint32_t rsvd2[22];	/* 0xa8 - 0xff */
}BIUCFG_Access;

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
	uint32_t rsvd[2];	/* 0x08 - 0x0f */
	uint32_t bac_event;	/* 0x10 */
	uint32_t rsvd_1[3];	/* 0x14 - 0x1f */
	uint32_t bac_ccicfg;	/* 0x20 */
	uint32_t bac_cciaddr;	/* 0x24 */
	uint32_t rsvd_2[4];	/* 0x28 - 0x37 */
	uint32_t bac_ccievs2;	/* 0x38 */
	uint32_t bac_ccievs3;	/* 0x3c */
	uint32_t bac_ccievs4;	/* 0x40 */
	uint32_t rsvd_3[3];	/* 0x44 - 0x4f */
	uint32_t bac_ccievm0;	/* 0x50 */
	uint32_t bac_ccievm1;	/* 0x54 */
	uint32_t rsvd_4[2];	/* 0x58 - 0x5f */
	uint32_t bac_dapapbcfg;	/* 0x60 */
	uint32_t bac_status;	/* 0x64 */
	uint32_t rsvd_5[2];	/* 0x68 - 0x6f */
	uint32_t cpu_therm_irq_cfg;	/* 0x70 */
	uint32_t cpu_therm_threshold_cfg;	/* 0x74 */
	uint32_t rsvd_6;	/* 0x78 */
	uint32_t cpu_therm_temp;	/* 0x7c */
	uint32_t rsvd_7[32];	/* 0x80 - 0xff */
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

typedef struct BIUCFG_TS0_CTRL {
	uint32_t CNTCR;		/* 0x00 */
	uint32_t CNTSR;		/* 0x04 */
	uint32_t CNTCVL;	/* 0x08 */
	uint32_t CNTCVU;	/* 0x0c */
	uint32_t rsvd_0[4];	/* 0x10 - 0x1f */
	uint32_t CNTFID0;	/* 0x20 */
	uint32_t rsvd_1[1015];	/* 0x24 - 0xfff */
} BIUCFG_TS0_CTRL;

typedef struct BIUCFG {
	BIUCFG_Access access;	/* 0x0 - 0xff*/
	BIUCFG_Cluster cluster[2];	/* 0x100 - 0x2ff*/
	BIUCFG_Bac bac;	/* 0x300 - 0x3ff */
	uint32_t anonymous[192];	/* 0x400 - 0x6ff */
	BIUCFG_Aux aux;	/* 0x700 - 0x7ff */
	uint32_t anonymous_1[512];	/* 0x800 - 0xfff */
	BIUCFG_TS0_CTRL ts0_ctrl;	/* 0x1000 - 0x1fff */
}BIUCFG;

#define BIUCFG_BASE 0x81060000
#define BIUCFG ((volatile BIUCFG * const) BIUCFG_BASE)

#endif
