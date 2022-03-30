// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <fsl_ddr_sdram.h>

#define CONFIG_SYS_DDR_TIMING_3_1200	0x01030000
#define CONFIG_SYS_DDR_TIMING_0_1200	0xCC550104
#define CONFIG_SYS_DDR_TIMING_1_1200	0x868FAA45
#define CONFIG_SYS_DDR_TIMING_2_1200	0x0FB8A912
#define CONFIG_SYS_DDR_MODE_1_1200	0x00441A40
#define CONFIG_SYS_DDR_MODE_2_1200	0x00100000
#define CONFIG_SYS_DDR_INTERVAL_1200	0x12480100
#define CONFIG_SYS_DDR_CLK_CTRL_1200	0x02800000

#define CONFIG_SYS_DDR_TIMING_3_1000	0x00020000
#define CONFIG_SYS_DDR_TIMING_0_1000	0xCC440104
#define CONFIG_SYS_DDR_TIMING_1_1000	0x727DF944
#define CONFIG_SYS_DDR_TIMING_2_1000	0x0FB088CF
#define CONFIG_SYS_DDR_MODE_1_1000	0x00441830
#define CONFIG_SYS_DDR_MODE_2_1000	0x00080000
#define CONFIG_SYS_DDR_INTERVAL_1000	0x0F3C0100
#define CONFIG_SYS_DDR_CLK_CTRL_1000	0x02800000

#define CONFIG_SYS_DDR_TIMING_3_900	0x00020000
#define CONFIG_SYS_DDR_TIMING_0_900	0xCC440104
#define CONFIG_SYS_DDR_TIMING_1_900	0x616ba844
#define CONFIG_SYS_DDR_TIMING_2_900	0x0fb088ce
#define CONFIG_SYS_DDR_MODE_1_900	0x00441620
#define CONFIG_SYS_DDR_MODE_2_900	0x00080000
#define CONFIG_SYS_DDR_INTERVAL_900	0x0db60100
#define CONFIG_SYS_DDR_CLK_CTRL_900	0x02800000

#define CONFIG_SYS_DDR_TIMING_3_800	0x00020000
#define CONFIG_SYS_DDR_TIMING_0_800	0xcc330104
#define CONFIG_SYS_DDR_TIMING_1_800	0x6f6b4744
#define CONFIG_SYS_DDR_TIMING_2_800	0x0fa888cc
#define CONFIG_SYS_DDR_MODE_1_800	0x00441420
#define CONFIG_SYS_DDR_MODE_2_800	0x00000000
#define CONFIG_SYS_DDR_INTERVAL_800	0x0c300100
#define CONFIG_SYS_DDR_CLK_CTRL_800	0x02800000

#define CONFIG_SYS_DDR_CS0_BNDS		0x000000FF
#define CONFIG_SYS_DDR_CS1_BNDS		0x00000000
#define CONFIG_SYS_DDR_CS2_BNDS		0x000000FF
#define CONFIG_SYS_DDR_CS3_BNDS		0x000000FF
#define CONFIG_SYS_DDR2_CS0_BNDS	0x000000FF
#define CONFIG_SYS_DDR2_CS1_BNDS	0x00000000
#define CONFIG_SYS_DDR2_CS2_BNDS	0x000000FF
#define CONFIG_SYS_DDR2_CS3_BNDS	0x000000FF
#define CONFIG_SYS_DDR_CS0_CONFIG	0xA0044202
#define CONFIG_SYS_DDR_CS0_CONFIG_2	0x00000000
#define CONFIG_SYS_DDR_CS1_CONFIG	0x80004202
#define CONFIG_SYS_DDR_CS2_CONFIG	0x00000000
#define CONFIG_SYS_DDR_CS3_CONFIG	0x00000000
#define CONFIG_SYS_DDR2_CS0_CONFIG	0x80044202
#define CONFIG_SYS_DDR2_CS1_CONFIG	0x80004202
#define CONFIG_SYS_DDR2_CS2_CONFIG	0x00000000
#define CONFIG_SYS_DDR2_CS3_CONFIG	0x00000000
#define CONFIG_SYS_DDR_INIT_ADDR	0x00000000
#define CONFIG_SYS_DDR_INIT_EXT_ADDR	0x00000000
#define CONFIG_SYS_DDR_CS1_CONFIG	0x80004202
#define CONFIG_SYS_DDR_DATA_INIT	0xdeadbeef
#define CONFIG_SYS_DDR_TIMING_4		0x00000001
#define CONFIG_SYS_DDR_TIMING_5		0x02401400
#define CONFIG_SYS_DDR_MODE_CONTROL	0x00000000
#define CONFIG_SYS_DDR_ZQ_CNTL		0x89080600
#define CONFIG_SYS_DDR_WRLVL_CNTL	0x8675F607
#define CONFIG_SYS_DDR_SDRAM_CFG	0xE7044000
#define CONFIG_SYS_DDR_SDRAM_CFG2	0x24401031
#define CONFIG_SYS_DDR_RCW_1		0x00000000
#define CONFIG_SYS_DDR_RCW_2		0x00000000
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef

fsl_ddr_cfg_regs_t ddr_cfg_regs_800 = {
	.cs[0].bnds = CONFIG_SYS_DDR_CS0_BNDS,
	.cs[1].bnds = CONFIG_SYS_DDR_CS1_BNDS,
	.cs[2].bnds = CONFIG_SYS_DDR_CS2_BNDS,
	.cs[3].bnds = CONFIG_SYS_DDR_CS3_BNDS,
	.cs[0].config = CONFIG_SYS_DDR_CS0_CONFIG,
	.cs[0].config_2 = CONFIG_SYS_DDR_CS0_CONFIG_2,
	.cs[1].config = CONFIG_SYS_DDR_CS1_CONFIG,
	.cs[2].config = CONFIG_SYS_DDR_CS2_CONFIG,
	.cs[3].config = CONFIG_SYS_DDR_CS3_CONFIG,
	.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3_800,
	.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0_800,
	.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1_800,
	.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2_800,
	.ddr_sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG,
	.ddr_sdram_cfg_2 = CONFIG_SYS_DDR_SDRAM_CFG2,
	.ddr_sdram_mode = CONFIG_SYS_DDR_MODE_1_800,
	.ddr_sdram_mode_2 = CONFIG_SYS_DDR_MODE_2_800,
	.ddr_sdram_md_cntl = CONFIG_SYS_DDR_MODE_CONTROL,
	.ddr_sdram_interval = CONFIG_SYS_DDR_INTERVAL_800,
	.ddr_data_init = CONFIG_MEM_INIT_VALUE,
	.ddr_sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL_800,
	.ddr_init_addr = CONFIG_SYS_DDR_INIT_ADDR,
	.ddr_init_ext_addr = CONFIG_SYS_DDR_INIT_EXT_ADDR,
	.timing_cfg_4 = CONFIG_SYS_DDR_TIMING_4,
	.timing_cfg_5 = CONFIG_SYS_DDR_TIMING_5,
	.ddr_zq_cntl = CONFIG_SYS_DDR_ZQ_CNTL,
	.ddr_wrlvl_cntl = CONFIG_SYS_DDR_WRLVL_CNTL,
	.ddr_sdram_rcw_1 = CONFIG_SYS_DDR_RCW_1,
	.ddr_sdram_rcw_2 = CONFIG_SYS_DDR_RCW_2
};

fsl_ddr_cfg_regs_t ddr_cfg_regs_800_2nd = {
	.cs[0].bnds = CONFIG_SYS_DDR2_CS0_BNDS,
	.cs[1].bnds = CONFIG_SYS_DDR2_CS1_BNDS,
	.cs[2].bnds = CONFIG_SYS_DDR2_CS2_BNDS,
	.cs[3].bnds = CONFIG_SYS_DDR2_CS3_BNDS,
	.cs[0].config = CONFIG_SYS_DDR2_CS0_CONFIG,
	.cs[0].config_2 = CONFIG_SYS_DDR_CS0_CONFIG_2,
	.cs[1].config = CONFIG_SYS_DDR2_CS1_CONFIG,
	.cs[2].config = CONFIG_SYS_DDR2_CS2_CONFIG,
	.cs[3].config = CONFIG_SYS_DDR2_CS3_CONFIG,
	.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3_800,
	.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0_800,
	.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1_800,
	.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2_800,
	.ddr_sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG,
	.ddr_sdram_cfg_2 = CONFIG_SYS_DDR_SDRAM_CFG2,
	.ddr_sdram_mode = CONFIG_SYS_DDR_MODE_1_800,
	.ddr_sdram_mode_2 = CONFIG_SYS_DDR_MODE_2_800,
	.ddr_sdram_md_cntl = CONFIG_SYS_DDR_MODE_CONTROL,
	.ddr_sdram_interval = CONFIG_SYS_DDR_INTERVAL_800,
	.ddr_data_init = CONFIG_MEM_INIT_VALUE,
	.ddr_sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL_800,
	.ddr_init_addr = CONFIG_SYS_DDR_INIT_ADDR,
	.ddr_init_ext_addr = CONFIG_SYS_DDR_INIT_EXT_ADDR,
	.timing_cfg_4 = CONFIG_SYS_DDR_TIMING_4,
	.timing_cfg_5 = CONFIG_SYS_DDR_TIMING_5,
	.ddr_zq_cntl = CONFIG_SYS_DDR_ZQ_CNTL,
	.ddr_wrlvl_cntl = CONFIG_SYS_DDR_WRLVL_CNTL,
	.ddr_sdram_rcw_1 = CONFIG_SYS_DDR_RCW_1,
	.ddr_sdram_rcw_2 = CONFIG_SYS_DDR_RCW_2
};

fsl_ddr_cfg_regs_t ddr_cfg_regs_900 = {
	.cs[0].bnds = CONFIG_SYS_DDR_CS0_BNDS,
	.cs[1].bnds = CONFIG_SYS_DDR_CS1_BNDS,
	.cs[2].bnds = CONFIG_SYS_DDR_CS2_BNDS,
	.cs[3].bnds = CONFIG_SYS_DDR_CS3_BNDS,
	.cs[0].config = CONFIG_SYS_DDR_CS0_CONFIG,
	.cs[0].config_2 = CONFIG_SYS_DDR_CS0_CONFIG_2,
	.cs[1].config = CONFIG_SYS_DDR_CS1_CONFIG,
	.cs[2].config = CONFIG_SYS_DDR_CS2_CONFIG,
	.cs[3].config = CONFIG_SYS_DDR_CS3_CONFIG,
	.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3_900,
	.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0_900,
	.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1_900,
	.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2_900,
	.ddr_sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG,
	.ddr_sdram_cfg_2 = CONFIG_SYS_DDR_SDRAM_CFG2,
	.ddr_sdram_mode = CONFIG_SYS_DDR_MODE_1_900,
	.ddr_sdram_mode_2 = CONFIG_SYS_DDR_MODE_2_900,
	.ddr_sdram_md_cntl = CONFIG_SYS_DDR_MODE_CONTROL,
	.ddr_sdram_interval = CONFIG_SYS_DDR_INTERVAL_900,
	.ddr_data_init = CONFIG_MEM_INIT_VALUE,
	.ddr_sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL_900,
	.ddr_init_addr = CONFIG_SYS_DDR_INIT_ADDR,
	.ddr_init_ext_addr = CONFIG_SYS_DDR_INIT_EXT_ADDR,
	.timing_cfg_4 = CONFIG_SYS_DDR_TIMING_4,
	.timing_cfg_5 = CONFIG_SYS_DDR_TIMING_5,
	.ddr_zq_cntl = CONFIG_SYS_DDR_ZQ_CNTL,
	.ddr_wrlvl_cntl = CONFIG_SYS_DDR_WRLVL_CNTL,
	.ddr_sdram_rcw_1 = CONFIG_SYS_DDR_RCW_1,
	.ddr_sdram_rcw_2 = CONFIG_SYS_DDR_RCW_2
};

fsl_ddr_cfg_regs_t ddr_cfg_regs_900_2nd = {
	.cs[0].bnds = CONFIG_SYS_DDR2_CS0_BNDS,
	.cs[1].bnds = CONFIG_SYS_DDR2_CS1_BNDS,
	.cs[2].bnds = CONFIG_SYS_DDR2_CS2_BNDS,
	.cs[3].bnds = CONFIG_SYS_DDR2_CS3_BNDS,
	.cs[0].config = CONFIG_SYS_DDR2_CS0_CONFIG,
	.cs[0].config_2 = CONFIG_SYS_DDR_CS0_CONFIG_2,
	.cs[1].config = CONFIG_SYS_DDR2_CS1_CONFIG,
	.cs[2].config = CONFIG_SYS_DDR2_CS2_CONFIG,
	.cs[3].config = CONFIG_SYS_DDR2_CS3_CONFIG,
	.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3_900,
	.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0_900,
	.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1_900,
	.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2_900,
	.ddr_sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG,
	.ddr_sdram_cfg_2 = CONFIG_SYS_DDR_SDRAM_CFG2,
	.ddr_sdram_mode = CONFIG_SYS_DDR_MODE_1_900,
	.ddr_sdram_mode_2 = CONFIG_SYS_DDR_MODE_2_900,
	.ddr_sdram_md_cntl = CONFIG_SYS_DDR_MODE_CONTROL,
	.ddr_sdram_interval = CONFIG_SYS_DDR_INTERVAL_900,
	.ddr_data_init = CONFIG_MEM_INIT_VALUE,
	.ddr_sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL_900,
	.ddr_init_addr = CONFIG_SYS_DDR_INIT_ADDR,
	.ddr_init_ext_addr = CONFIG_SYS_DDR_INIT_EXT_ADDR,
	.timing_cfg_4 = CONFIG_SYS_DDR_TIMING_4,
	.timing_cfg_5 = CONFIG_SYS_DDR_TIMING_5,
	.ddr_zq_cntl = CONFIG_SYS_DDR_ZQ_CNTL,
	.ddr_wrlvl_cntl = CONFIG_SYS_DDR_WRLVL_CNTL,
	.ddr_sdram_rcw_1 = CONFIG_SYS_DDR_RCW_1,
	.ddr_sdram_rcw_2 = CONFIG_SYS_DDR_RCW_2
};

fsl_ddr_cfg_regs_t ddr_cfg_regs_1000 = {
	.cs[0].bnds = CONFIG_SYS_DDR_CS0_BNDS,
	.cs[1].bnds = CONFIG_SYS_DDR_CS1_BNDS,
	.cs[2].bnds = CONFIG_SYS_DDR_CS2_BNDS,
	.cs[3].bnds = CONFIG_SYS_DDR_CS3_BNDS,
	.cs[0].config = CONFIG_SYS_DDR_CS0_CONFIG,
	.cs[0].config_2 = CONFIG_SYS_DDR_CS0_CONFIG_2,
	.cs[1].config = CONFIG_SYS_DDR_CS1_CONFIG,
	.cs[2].config = CONFIG_SYS_DDR_CS2_CONFIG,
	.cs[3].config = CONFIG_SYS_DDR_CS3_CONFIG,
	.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3_1000,
	.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0_1000,
	.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1_1000,
	.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2_1000,
	.ddr_sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG,
	.ddr_sdram_cfg_2 = CONFIG_SYS_DDR_SDRAM_CFG2,
	.ddr_sdram_mode = CONFIG_SYS_DDR_MODE_1_1000,
	.ddr_sdram_mode_2 = CONFIG_SYS_DDR_MODE_2_1000,
	.ddr_sdram_md_cntl = CONFIG_SYS_DDR_MODE_CONTROL,
	.ddr_sdram_interval = CONFIG_SYS_DDR_INTERVAL_1000,
	.ddr_data_init = CONFIG_MEM_INIT_VALUE,
	.ddr_sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL_1000,
	.ddr_init_addr = CONFIG_SYS_DDR_INIT_ADDR,
	.ddr_init_ext_addr = CONFIG_SYS_DDR_INIT_EXT_ADDR,
	.timing_cfg_4 = CONFIG_SYS_DDR_TIMING_4,
	.timing_cfg_5 = CONFIG_SYS_DDR_TIMING_5,
	.ddr_zq_cntl = CONFIG_SYS_DDR_ZQ_CNTL,
	.ddr_wrlvl_cntl = CONFIG_SYS_DDR_WRLVL_CNTL,
	.ddr_sdram_rcw_1 = CONFIG_SYS_DDR_RCW_1,
	.ddr_sdram_rcw_2 = CONFIG_SYS_DDR_RCW_2
};

fsl_ddr_cfg_regs_t ddr_cfg_regs_1000_2nd = {
	.cs[0].bnds = CONFIG_SYS_DDR2_CS0_BNDS,
	.cs[1].bnds = CONFIG_SYS_DDR2_CS1_BNDS,
	.cs[2].bnds = CONFIG_SYS_DDR2_CS2_BNDS,
	.cs[3].bnds = CONFIG_SYS_DDR2_CS3_BNDS,
	.cs[0].config = CONFIG_SYS_DDR2_CS0_CONFIG,
	.cs[0].config_2 = CONFIG_SYS_DDR_CS0_CONFIG_2,
	.cs[1].config = CONFIG_SYS_DDR2_CS1_CONFIG,
	.cs[2].config = CONFIG_SYS_DDR2_CS2_CONFIG,
	.cs[3].config = CONFIG_SYS_DDR2_CS3_CONFIG,
	.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3_1000,
	.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0_1000,
	.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1_1000,
	.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2_1000,
	.ddr_sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG,
	.ddr_sdram_cfg_2 = CONFIG_SYS_DDR_SDRAM_CFG2,
	.ddr_sdram_mode = CONFIG_SYS_DDR_MODE_1_1000,
	.ddr_sdram_mode_2 = CONFIG_SYS_DDR_MODE_2_1000,
	.ddr_sdram_md_cntl = CONFIG_SYS_DDR_MODE_CONTROL,
	.ddr_sdram_interval = CONFIG_SYS_DDR_INTERVAL_1000,
	.ddr_data_init = CONFIG_MEM_INIT_VALUE,
	.ddr_sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL_1000,
	.ddr_init_addr = CONFIG_SYS_DDR_INIT_ADDR,
	.ddr_init_ext_addr = CONFIG_SYS_DDR_INIT_EXT_ADDR,
	.timing_cfg_4 = CONFIG_SYS_DDR_TIMING_4,
	.timing_cfg_5 = CONFIG_SYS_DDR_TIMING_5,
	.ddr_zq_cntl = CONFIG_SYS_DDR_ZQ_CNTL,
	.ddr_wrlvl_cntl = CONFIG_SYS_DDR_WRLVL_CNTL,
	.ddr_sdram_rcw_1 = CONFIG_SYS_DDR_RCW_1,
	.ddr_sdram_rcw_2 = CONFIG_SYS_DDR_RCW_2
};

fsl_ddr_cfg_regs_t ddr_cfg_regs_1200 = {
	.cs[0].bnds = CONFIG_SYS_DDR_CS0_BNDS,
	.cs[1].bnds = CONFIG_SYS_DDR_CS1_BNDS,
	.cs[2].bnds = CONFIG_SYS_DDR_CS2_BNDS,
	.cs[3].bnds = CONFIG_SYS_DDR_CS3_BNDS,
	.cs[0].config = CONFIG_SYS_DDR_CS0_CONFIG,
	.cs[0].config_2 = CONFIG_SYS_DDR_CS0_CONFIG_2,
	.cs[1].config = CONFIG_SYS_DDR_CS1_CONFIG,
	.cs[2].config = CONFIG_SYS_DDR_CS2_CONFIG,
	.cs[3].config = CONFIG_SYS_DDR_CS3_CONFIG,
	.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3_1200,
	.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0_1200,
	.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1_1200,
	.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2_1200,
	.ddr_sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG,
	.ddr_sdram_cfg_2 = CONFIG_SYS_DDR_SDRAM_CFG2,
	.ddr_sdram_mode = CONFIG_SYS_DDR_MODE_1_1200,
	.ddr_sdram_mode_2 = CONFIG_SYS_DDR_MODE_2_1200,
	.ddr_sdram_md_cntl = CONFIG_SYS_DDR_MODE_CONTROL,
	.ddr_sdram_interval = CONFIG_SYS_DDR_INTERVAL_1200,
	.ddr_data_init = CONFIG_MEM_INIT_VALUE,
	.ddr_sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL_1200,
	.ddr_init_addr = CONFIG_SYS_DDR_INIT_ADDR,
	.ddr_init_ext_addr = CONFIG_SYS_DDR_INIT_EXT_ADDR,
	.timing_cfg_4 = CONFIG_SYS_DDR_TIMING_4,
	.timing_cfg_5 = CONFIG_SYS_DDR_TIMING_5,
	.ddr_zq_cntl = CONFIG_SYS_DDR_ZQ_CNTL,
	.ddr_wrlvl_cntl = CONFIG_SYS_DDR_WRLVL_CNTL,
	.ddr_sdram_rcw_1 = CONFIG_SYS_DDR_RCW_1,
	.ddr_sdram_rcw_2 = CONFIG_SYS_DDR_RCW_2
};

fsl_ddr_cfg_regs_t ddr_cfg_regs_1200_2nd = {
	.cs[0].bnds = CONFIG_SYS_DDR2_CS0_BNDS,
	.cs[1].bnds = CONFIG_SYS_DDR2_CS1_BNDS,
	.cs[2].bnds = CONFIG_SYS_DDR2_CS2_BNDS,
	.cs[3].bnds = CONFIG_SYS_DDR2_CS3_BNDS,
	.cs[0].config = CONFIG_SYS_DDR2_CS0_CONFIG,
	.cs[0].config_2 = CONFIG_SYS_DDR_CS0_CONFIG_2,
	.cs[1].config = CONFIG_SYS_DDR2_CS1_CONFIG,
	.cs[2].config = CONFIG_SYS_DDR2_CS2_CONFIG,
	.cs[3].config = CONFIG_SYS_DDR2_CS3_CONFIG,
	.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3_1200,
	.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0_1200,
	.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1_1200,
	.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2_1200,
	.ddr_sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG,
	.ddr_sdram_cfg_2 = CONFIG_SYS_DDR_SDRAM_CFG2,
	.ddr_sdram_mode = CONFIG_SYS_DDR_MODE_1_1200,
	.ddr_sdram_mode_2 = CONFIG_SYS_DDR_MODE_2_1200,
	.ddr_sdram_md_cntl = CONFIG_SYS_DDR_MODE_CONTROL,
	.ddr_sdram_interval = CONFIG_SYS_DDR_INTERVAL_1200,
	.ddr_data_init = CONFIG_MEM_INIT_VALUE,
	.ddr_sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL_1200,
	.ddr_init_addr = CONFIG_SYS_DDR_INIT_ADDR,
	.ddr_init_ext_addr = CONFIG_SYS_DDR_INIT_EXT_ADDR,
	.timing_cfg_4 = CONFIG_SYS_DDR_TIMING_4,
	.timing_cfg_5 = CONFIG_SYS_DDR_TIMING_5,
	.ddr_zq_cntl = CONFIG_SYS_DDR_ZQ_CNTL,
	.ddr_wrlvl_cntl = CONFIG_SYS_DDR_WRLVL_CNTL,
	.ddr_sdram_rcw_1 = CONFIG_SYS_DDR_RCW_1,
	.ddr_sdram_rcw_2 = CONFIG_SYS_DDR_RCW_2
};

fixed_ddr_parm_t fixed_ddr_parm_0[] = {
	{750, 850, &ddr_cfg_regs_800},
	{850, 950, &ddr_cfg_regs_900},
	{950, 1050, &ddr_cfg_regs_1000},
	{1050, 1250, &ddr_cfg_regs_1200},
	{0, 0, NULL}
};

fixed_ddr_parm_t fixed_ddr_parm_1[] = {
	{750, 850, &ddr_cfg_regs_800_2nd},
	{850, 950, &ddr_cfg_regs_900_2nd},
	{950, 1050, &ddr_cfg_regs_1000_2nd},
	{1050, 1250, &ddr_cfg_regs_1200_2nd},
	{0, 0, NULL}
};
