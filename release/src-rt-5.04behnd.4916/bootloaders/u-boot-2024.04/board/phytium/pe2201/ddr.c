// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.
 * lixinde          <lixinde@phytium.com.cn>
 * weichangzheng    <weichangzheng@phytium.com.cn>
 */

#include <stdio.h>
#include <linux/arm-smccc.h>
#include <init.h>
#include "cpu.h"

struct ddr_spd {
	/***************** read from spd ******************/
	u8 dimm_type;        /* 1: RDIMM;	2: UDIMM;	3: SODIMM;	4: LRDIMM */
	u8 data_width;       /* 0: x4;		1: x8;		2: x16;		3: x32 */
	u8 mirror_type;      /* 0: standard;	1: mirror */
	u8 ecc_type;         /* 0: no-ecc;	1: ecc */
	u8 dram_type;        /* 0xB: DDR3;	0xC: DDR4 */
	u8 rank_num;
	u8 row_num;
	u8 col_num;

	u8 bg_num;           /* DDR4/DDR5 */
	u8 bank_num;
	u16 module_manufacturer_id;
	u16 taamin;
	u16 trcdmin;

	u16 trpmin;
	u16 trasmin;
	u16 trcmin;
	u16 tfawmin;         /* only DDR3/DDR4 */

	u16 trrd_smin;       /* only DDR4 */
	u16 trrd_lmin;       /* only DDR4 */
	u16 tccd_lmin;       /* only DDR4 */
	u16 twrmin;

	u16 twtr_smin;       /* only DDR4 */
	u16 twtr_lmin;       /* only DDR4 */
	u32 trfc1min;

	u32 trfc2min;
	u32 trfc4_rfcsbmin;  /* DDR4: tRFC4min;	DDR5: tRFCsbmin */
	u8 resv[8];

	/***************** RCD control words ******************/
	u8 f0rc03;           /* bit[3:2]:CS	bit[1:0]:CA */
	u8 f0rc04;           /* bit[3:2]:ODT  bit[1:0]:CKE */
	u8 f0rc05;           /* bit[3:2]:CLK-A side  bit[1:0]:CLK-B side */
	u8 rcd_num;          /* Registers used on RDIMM */

	u8 lrdimm_resv[4];
	u8 lrdimm_resv1[8];
	u8 lrdimm_resv2[8];
} __attribute((aligned(4)));

struct mcu_config {
	u32 magic;
	u32 version;
	u32 size;
	u8 rev1[4];

	u8 ch_enable;
	u8 resv1[7];

	u64 misc_enable;

	u8 train_debug;
	u8 train_recover;
	u8 train_param_type;
	u8 train_param_1;    /* DDR4: cpu_odt */
	u8 train_param_2;    /* DDR4: cpu_drv */
	u8 train_param_3;    /* DDR4: mr_drv */
	u8 train_param_4;    /* DDR4: rtt_nom */
	u8 train_param_5;    /* DDR4: rtt_park */

	u8 train_param_6;    /* DDR4: rtt_wr */
	u8 resv2[7];

	/***************** for LPDDR4 dq swap ******************/
	u32 data_byte_swap;
	u32 slice0_dq_swizzle;

	u32 slice1_dq_swizzle;
	u32 slice2_dq_swizzle;

	u32 slice3_dq_swizzle;
	u32 slice4_dq_swizzle;

	u32 slice5_dq_swizzle;
	u32 slice6_dq_swizzle;

	u32 slice7_dq_swizzle;
	u8 resv3[4];
	u8 resv4[8];

	struct ddr_spd ddr_spd_info;
} __attribute((aligned(4)));

static void get_mcu_up_info_default(struct mcu_config *pm)
{
	pm->magic		= PARAMETER_MCU_MAGIC;
	pm->version		= PARAM_MCU_VERSION;
	pm->size		= PARAM_MCU_SIZE;
	pm->ch_enable	= PARAM_CH_ENABLE;
}

static u8 init_dimm_param(struct mcu_config *pm)
{
	debug("manual config dimm info...\n");
	pm->misc_enable = 0x2001;
	pm->train_debug = 0x0;
	pm->train_recover = 0x0;
	pm->train_param_type = 0x0;
	pm->train_param_1 = 0x0;
	pm->train_param_2 = 0x0;
	pm->train_param_3 = 0x0;
	pm->train_param_4 = 0x0;
	pm->train_param_5 = 0x0;
	pm->train_param_6 = 0x0;

	pm->data_byte_swap = 0x76543210;
	pm->slice0_dq_swizzle = 0x3145726;

	pm->slice1_dq_swizzle = 0x54176230;
	pm->slice2_dq_swizzle = 0x57604132;

	pm->slice3_dq_swizzle = 0x20631547;
	pm->slice4_dq_swizzle = 0x16057423;

	pm->slice5_dq_swizzle = 0x16057423;
	pm->slice6_dq_swizzle = 0x16057423;

	pm->slice7_dq_swizzle = 0x16057423;

	pm->ddr_spd_info.dimm_type = RDIMM_TYPE;
	pm->ddr_spd_info.data_width = DIMM_X16;
	pm->ddr_spd_info.mirror_type = NO_MIRROR;
	pm->ddr_spd_info.ecc_type = NO_ECC_TYPE;
	pm->ddr_spd_info.dram_type = LPDDR4_TYPE;
	pm->ddr_spd_info.rank_num = 0x1;
	pm->ddr_spd_info.row_num = 0x10;
	pm->ddr_spd_info.col_num = 0xa;
	pm->ddr_spd_info.bg_num = 0x0;
	pm->ddr_spd_info.bank_num = 0x8;
	pm->ddr_spd_info.taamin = 0x0;
	pm->ddr_spd_info.trcdmin = 0x0;

	pm->ddr_spd_info.trpmin = 0x0;
	pm->ddr_spd_info.trasmin = 0x0;
	pm->ddr_spd_info.trcmin = 0x0;
	pm->ddr_spd_info.tfawmin = 0x0;

	pm->ddr_spd_info.trrd_smin = 0x0;
	pm->ddr_spd_info.trrd_lmin = 0x0;
	pm->ddr_spd_info.tccd_lmin = 0x0;
	pm->ddr_spd_info.twrmin = 0x0;

	pm->ddr_spd_info.twtr_smin = 0x0;
	pm->ddr_spd_info.twtr_lmin = 0x0;

	return 0;
}

void get_default_mcu_info(u8 *data)
{
	get_mcu_up_info_default((struct mcu_config *)data);
}

void fix_mcu_info(u8 *data)
{
	struct mcu_config *mcu_info = (struct mcu_config *)data;

	init_dimm_param(mcu_info);
}

void ddr_init(void)
{
	u8 buffer[0x100];
	struct arm_smccc_res res;

	get_default_mcu_info(buffer);
	fix_mcu_info(buffer);

	arm_smccc_smc(CPU_INIT_MEM, 0, (u64)buffer, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		panic("DRAM init failed :0x%lx\n", res.a0);
}
