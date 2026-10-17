// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021
 * lixinde         <lixinde@phytium.com.cn>
 * weichangzheng   <weichangzheng@phytium.com.cn>
 */

#include <stdio.h>
#include <linux/arm-smccc.h>
#include <init.h>
#include "cpu.h"

struct ddr_spd {
	/******************* read from spd *****************/
	u8  dimm_type;	/* 1: RDIMM;2: UDIMM;3: SODIMM;4: LRDIMM */
	u8  data_width;	/* 0: x4; 1: x8; 2: x16 */
	u8  mirror_type;/* 0: stardard; 1: mirror */
	u8  ecc_type;	/* 0: no-ecc; 1:ecc */
	u8  dram_type;	/* 0xB: DDR3; 0xC: DDR4 */
	u8  rank_num;
	u8  row_num;
	u8  col_num;

	u8  bg_num;	/*only DDR4*/
	u8  bank_num;
	u16 module_manufacturer_id;
	u16 taamin;
	u16 trcdmin;

	u16 trpmin;
	u16 trasmin;
	u16 trcmin;
	u16 tfawmin;

	u16 trrd_smin;	/*only DDR4*/
	u16 trrd_lmin;	/*only DDR4*/
	u16 tccd_lmin;	/*only DDR4*/
	u16 twrmin;

	u16 twtr_smin;	/*only DDR4*/
	u16 twtr_lmin;	/*only DDR4*/
	u16 twtrmin;	/*only DDR3*/
	u16 trrdmin;	/*only DDR3*/

	/******************* RCD control words *****************/
	u8  f0rc03; /*bit[3:2]:CS		bit[1:0]:CA  */
	u8  f0rc04; /*bit[3:2]:ODT		bit[1:0]:CKE */
	u8  f0rc05; /*bit[3:2]:CLK-A side	bit[1:0]:CLK-B side */
	u8  bc00;
	u8  bc01;
	u8  bc02;
	u8  bc03;
	u8  bc04;

	u8  bc05;
	u8  f5bc5x;
	u8  f5bc6x;
	/******************* LRDIMM special *****************/
	u8  vrefdq_pr0;
	u8  vrefdq_mdram;
	u8  rtt_mdram_1866;
	u8  rtt_mdram_2400;
	u8  rtt_mdram_3200;

	u8  drive_dram;
	u8  odt_dram_1866;
	u8  odt_dram_2400;
	u8  odt_dram_3200;
	u8  park_dram_1866;
	u8  park_dram_2400;
	u8  park_dram_3200;
	u8  rcd_num;
} __attribute((aligned(4)));

struct mcu_config {
	u32 magic;
	u32 version;
	u32 size;
	u8 rev1[4];

	u8 ch_enable;
	u8 misc1_enable;
	u8 misc2_enable;
	u8 force_spd_enable;
	u8 misc3_enable;
	u8 train_debug;
	u8 train_recover;
	u8 rev2[9];

	struct ddr_spd ddr_spd_info[2];
} __attribute((aligned(4)));

static void get_mcu_up_info_default(struct mcu_config *pm)
{
	pm->magic		= PARAMETER_MCU_MAGIC;
	pm->version		= PARAM_MCU_VERSION;
	pm->size		= PARAM_MCU_SIZE;
	pm->ch_enable		= PARAM_CH_ENABLE;
	pm->misc1_enable	= PARAM_ECC_ENABLE;
	pm->force_spd_enable	= PARAM_FORCE_SPD_DISABLE;
	pm->misc3_enable	= PARAM_MCU_MISC_ENABLE;
	pm->train_recover	= 0x0;
}

static u8 init_dimm_param(u8 ch, struct mcu_config *pm)
{
	debug("manual config dimm info...\n");
	pm->ddr_spd_info[ch].dimm_type = UDIMM_TYPE;
	pm->ddr_spd_info[ch].data_width = DIMM_X8;
	pm->ddr_spd_info[ch].mirror_type = NO_MIRROR;
	pm->ddr_spd_info[ch].ecc_type = NO_ECC_TYPE;
	pm->ddr_spd_info[ch].dram_type = DDR4_TYPE;
	pm->ddr_spd_info[ch].rank_num = 1;
	pm->ddr_spd_info[ch].row_num  = 16;
	pm->ddr_spd_info[ch].col_num = 10;
	pm->ddr_spd_info[ch].bg_num = 4;
	pm->ddr_spd_info[ch].bank_num = 4;
	pm->ddr_spd_info[ch].taamin = 13750;
	pm->ddr_spd_info[ch].trcdmin = 13750;

	pm->ddr_spd_info[ch].trpmin = 13750;
	pm->ddr_spd_info[ch].trasmin = 32000;
	pm->ddr_spd_info[ch].trcmin =  45750;
	pm->ddr_spd_info[ch].tfawmin = 21000;

	pm->ddr_spd_info[ch].trrd_smin = 3000;
	pm->ddr_spd_info[ch].trrd_lmin = 4900;
	pm->ddr_spd_info[ch].tccd_lmin = 5000;
	pm->ddr_spd_info[ch].twrmin = 15000;

	pm->ddr_spd_info[ch].twtr_smin = 2500;
	pm->ddr_spd_info[ch].twtr_lmin = 7500;

	return 0;
}

void get_default_mcu_info(u8 *data)
{
	get_mcu_up_info_default((struct mcu_config *)data);
}

void fix_mcu_info(u8 *data)
{
	struct mcu_config *mcu_info = (struct mcu_config *)data;

	for (int ch = 0; ch < 2; ch++)
		init_dimm_param(ch, mcu_info);
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
