// SPDX-License-Identifier: GPL-2.0+
/*
 * SPL specific code for Compulab CM-T54 board
 *
 * Copyright (C) 2014, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Dmitry Lifshitz <lifshitz@compulab.co.il>
 */

#include <asm/emif.h>

const struct emif_regs emif_regs_ddr3_532_mhz_cm_t54 = {
#if defined(CONFIG_DRAM_1G) || defined(CONFIG_DRAM_512M)
	.sdram_config_init              = 0x618522B2,
	.sdram_config                   = 0x618522B2,
#elif defined(CONFIG_DRAM_2G)
	.sdram_config_init              = 0x618522BA,
	.sdram_config                   = 0x618522BA,
#endif
	.sdram_config2			= 0x0,
	.ref_ctrl                       = 0x00001040,
	.sdram_tim1                     = 0xEEEF36F3,
	.sdram_tim2                     = 0x348F7FDA,
	.sdram_tim3                     = 0x027F88A8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x1007190B,
	.temp_alert_config              = 0x00000000,

	.emif_ddr_phy_ctlr_1_init       = 0x0030400B,
	.emif_ddr_phy_ctlr_1            = 0x0034400B,
	.emif_ddr_ext_phy_ctrl_1        = 0x04040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x00000000,
	.emif_ddr_ext_phy_ctrl_3        = 0x00000000,
	.emif_ddr_ext_phy_ctrl_4        = 0x00000000,
	.emif_ddr_ext_phy_ctrl_5        = 0x4350D435,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x40000305,
};

const struct dmm_lisa_map_regs lisa_map_cm_t54 = {
	.dmm_lisa_map_0 = 0x0,
	.dmm_lisa_map_1 = 0x0,

#ifdef CONFIG_DRAM_2G
	.dmm_lisa_map_2 = 0x80740300,
#elif defined(CONFIG_DRAM_1G)
	.dmm_lisa_map_2 = 0x80640300,
#elif defined(CONFIG_DRAM_512M)
	.dmm_lisa_map_2 = 0x80500100,
#endif
	.dmm_lisa_map_3 = 0x00000000,
	.is_ma_present	= 0x1,
};

void emif_get_reg_dump(u32 emif_nr, const struct emif_regs **regs)
{
	*regs = &emif_regs_ddr3_532_mhz_cm_t54;
}

void emif_get_dmm_regs(const struct dmm_lisa_map_regs **dmm_lisa_regs)
{
	*dmm_lisa_regs = &lisa_map_cm_t54;
}
