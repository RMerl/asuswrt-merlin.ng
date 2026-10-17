/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 NXP
 */

#ifndef __ASM_ARCH_IMX8ULP_RDC_H
#define __ASM_ARCH_IMX8ULP_RDC_H

enum rdc_type {
	RDC_TRDC,
	RDC_XRDC,
};

int release_rdc(enum rdc_type type);
void xrdc_mrc_region_set_access(int mrc_index, u32 addr, u32 access);
int xrdc_config_mrc_dx_perm(u32 mrc_con, u32 region, u32 dom, u32 dxsel);
int xrdc_config_mrc_w0_w1(u32 mrc_con, u32 region, u32 w0, u32 size);
int xrdc_config_mrc_w3_w4(u32 mrc_con, u32 region, u32 w3, u32 w4);
int xrdc_config_pdac(u32 bridge, u32 index, u32 dom, u32 perm);
int xrdc_config_pdac_openacc(u32 bridge, u32 index);
int trdc_mbc_set_access(u32 mbc_x, u32 dom_x, u32 mem_x, u32 blk_x, bool sec_access);
int trdc_mrc_region_set_access(u32 mrc_x, u32 dom_x, u32 addr_start, u32 addr_end, bool sec_access);

void xrdc_init_mda(void);
void xrdc_init_mrc(void);
void xrdc_init_pdac_msc(void);

#endif
