/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * DDR3
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef _DDR3_H_
#define _DDR3_H_

#include <asm/arch/hardware.h>

struct ddr3_phy_config {
	unsigned int pllcr;
	unsigned int pgcr1_mask;
	unsigned int pgcr1_val;
	unsigned int ptr0;
	unsigned int ptr1;
	unsigned int ptr2;
	unsigned int ptr3;
	unsigned int ptr4;
	unsigned int dcr_mask;
	unsigned int dcr_val;
	unsigned int dtpr0;
	unsigned int dtpr1;
	unsigned int dtpr2;
	unsigned int mr0;
	unsigned int mr1;
	unsigned int mr2;
	unsigned int dtcr;
	unsigned int pgcr2;
	unsigned int zq0cr1;
	unsigned int zq1cr1;
	unsigned int zq2cr1;
	unsigned int pir_v1;
	unsigned int datx8_2_mask;
	unsigned int datx8_2_val;
	unsigned int datx8_3_mask;
	unsigned int datx8_3_val;
	unsigned int datx8_4_mask;
	unsigned int datx8_4_val;
	unsigned int datx8_5_mask;
	unsigned int datx8_5_val;
	unsigned int datx8_6_mask;
	unsigned int datx8_6_val;
	unsigned int datx8_7_mask;
	unsigned int datx8_7_val;
	unsigned int datx8_8_mask;
	unsigned int datx8_8_val;
	unsigned int pir_v2;
};

struct ddr3_emif_config {
	unsigned int sdcfg;
	unsigned int sdtim1;
	unsigned int sdtim2;
	unsigned int sdtim3;
	unsigned int sdtim4;
	unsigned int zqcfg;
	unsigned int sdrfc;
};

struct ddr3_spd_cb {
	char   dimm_name[32];
	struct ddr3_phy_config phy_cfg;
	struct ddr3_emif_config emif_cfg;
	unsigned int ddrspdclock;
	int    ddr_size_gbyte;
};

u32 ddr3_init(void);
void ddr3_reset_ddrphy(void);
void ddr3_init_ecc(u32 base, u32 ddr3_size);
void ddr3_disable_ecc(u32 base);
void ddr3_check_ecc_int(u32 base);
int ddr3_ecc_support_rmw(u32 base);
void ddr3_err_reset_workaround(void);
void ddr3_enable_ecc(u32 base, int test);
void ddr3_init_ddrphy(u32 base, struct ddr3_phy_config *phy_cfg);
void ddr3_init_ddremif(u32 base, struct ddr3_emif_config *emif_cfg);
int ddr3_get_size(void);

#endif
