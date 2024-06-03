/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2014
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA20_MC_H_
#define _TEGRA20_MC_H_

/**
 * Defines the memory controller registers we need/care about
 */
struct mc_ctlr {
	u32 reserved0[3];			/* offset 0x00 - 0x08 */
	u32 mc_emem_cfg;			/* offset 0x0C */
	u32 mc_emem_adr_cfg;			/* offset 0x10 */
	u32 mc_emem_arb_cfg0;			/* offset 0x14 */
	u32 mc_emem_arb_cfg1;			/* offset 0x18 */
	u32 mc_emem_arb_cfg2;			/* offset 0x1C */
	u32 reserved1;				/* offset 0x20 */
	u32 mc_gart_cfg;			/* offset 0x24 */
	u32 mc_gart_entry_addr;			/* offset 0x28 */
	u32 mc_gart_entry_data;			/* offset 0x2C */
	u32 mc_gart_error_req;			/* offset 0x30 */
	u32 mc_gart_error_addr;			/* offset 0x34 */
	u32 reserved2;				/* offset 0x38 */
	u32 mc_timeout_ctrl;			/* offset 0x3C */
	u32 reserved3[6];			/* offset 0x40 - 0x54 */
	u32 mc_decerr_emem_others_status;	/* offset 0x58 */
	u32 mc_decerr_emem_others_adr;		/* offset 0x5C */
	u32 reserved4[40];			/* offset 0x60 - 0xFC */
	u32 reserved5[93];			/* offset 0x100 - 0x270 */
};

#endif	/* _TEGRA20_MC_H_ */
