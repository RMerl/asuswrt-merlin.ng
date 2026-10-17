/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 NXP
 */

#ifndef __ASM_ARCH_IMX9_TRDC_H
#define __ASM_ARCH_IMX9_TRDC_H

int trdc_mbc_set_control(ulong trdc_reg, u32 mbc_x, u32 glbac_id, u32 glbac_val);
int trdc_mbc_blk_config(ulong trdc_reg, u32 mbc_x, u32 dom_x, u32 mem_x, u32 blk_x,
			bool sec_access, u32 glbac_id);
int trdc_mrc_set_control(ulong trdc_reg, u32 mrc_x, u32 glbac_id, u32 glbac_val);
int trdc_mrc_region_config(ulong trdc_reg, u32 mrc_x, u32 dom_x, u32 addr_start,
			   u32 addr_end, bool sec_access, u32 glbac_id);

void trdc_early_init(void);
void trdc_init(void);

#endif
