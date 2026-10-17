/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Freescale Semiconductor
 * Copyright 2021 NXP
 */

#ifndef __FSL_MC_H__
#define __FSL_MC_H__

#include <linux/bitops.h>

struct bd_info;

#define MC_CCSR_BASE_ADDR \
	((struct mc_ccsr_registers __iomem *)0x8340000)

#define GCR1_P1_STOP		BIT(31)
#define GCR1_P2_STOP		BIT(30)
#define GCR1_P1_DE_RST		BIT(23)
#define GCR1_P2_DE_RST		BIT(22)
#define GCR1_M1_DE_RST		BIT(15)
#define GCR1_M2_DE_RST		BIT(14)
#define GCR1_M_ALL_DE_RST	(GCR1_M1_DE_RST | GCR1_M2_DE_RST)
#define GSR_FS_MASK		0x3fffffff

#define SOC_MC_PORTALS_BASE_ADDR    ((void __iomem *)0x00080C000000)
#define SOC_QBMAN_PORTALS_BASE_ADDR ((void __iomem *)0x000818000000)
#define SOC_MC_PORTAL_STRIDE	    0x10000

#define SOC_MC_PORTAL_ADDR(_portal_id) \
	((void __iomem *)((uintptr_t)SOC_MC_PORTALS_BASE_ADDR + \
	 (_portal_id) * SOC_MC_PORTAL_STRIDE))

#define MC_PORTAL_OFFSET_TO_PORTAL_ID(_portal_offset) \
	((_portal_offset) / SOC_MC_PORTAL_STRIDE)

struct mc_ccsr_registers {
	u32 reg_gcr1;
	u32 reserved1;
	u32 reg_gsr;
	u32 reserved2;
	u32 reg_sicbalr;
	u32 reg_sicbahr;
	u32 reg_sicapr;
	u32 reserved3;
	u32 reg_mcfbalr;
	u32 reg_mcfbahr;
	u32 reg_mcfapr;
	u32 reserved4[0x2f1];
	u32 reg_psr;
	u32 reserved5;
	u32 reg_brr[2];
	u32 reserved6[0x80];
	u32 reg_error[];
};

struct log_header {
	u32 magic_word;
	char reserved[4];
	u32 buf_start;
	u32 buf_length;
	u32 last_byte;
};

void fdt_fsl_mc_fixup_iommu_map_entry(void *blob);
int get_mc_boot_status(void);
int get_dpl_apply_status(void);
int is_lazy_dpl_addr_valid(void);
void fdt_fixup_mc_ddr(u64 *base, u64 *size);
void fdt_reserve_mc_mem(void *blob, u32 mc_icid);
#ifdef CFG_SYS_LS_MC_DRAM_AIOP_IMG_OFFSET
int get_aiop_apply_status(void);
#endif
u64 mc_get_dram_addr(void);
unsigned long mc_get_dram_block_size(void);
int fsl_mc_ldpaa_init(struct bd_info *bis);
int fsl_mc_ldpaa_exit(struct bd_info *bd);
void mc_env_boot(void);
#endif
