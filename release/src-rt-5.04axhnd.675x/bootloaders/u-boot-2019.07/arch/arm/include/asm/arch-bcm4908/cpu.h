/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>

#define BIUCTRL_BASE                0x81062000
#define BOOTLUT_BASE                0xffff0000

typedef struct BIUArchRegion {
	uint32_t addr_ulimit;
	uint32_t addr_llimit;
	uint32_t permission;
	uint32_t access_right_ctrl;
} BIUArchRegion;

typedef struct BIUArch {
	BIUArchRegion region[8];	/* 0x0 */
	uint32_t unused[95];	/* 0x80 */
	uint32_t scratch;	/* 0x1fc */
} BIUArch;

#define BIUARCH ((volatile BIUArch * const) BIUARCH_BASE)

typedef struct BIUCpuBusRange {
#define ULIMIT_SHIFT 4
#define BUSNUM_MASK 0x0000000FU

#define BUSNUM_UBUS 1
#define BUSNUM_RBUS 2
#define BUSNUM_RSVD 3
#define BUSNUM_MCP0 4
#define BUSNUM_MCP1 5
#define BUSNUM_MCP2 6

	uint32_t ulimit;
	uint32_t llimit;
} BIUCpuBusRange;

typedef struct BIUCpuAccessRightViol {
	uint32_t addr;
	uint32_t upper_addr;
	uint32_t detail_addr;
} BIUCpuAccessRightViol;

typedef struct BIUCpuBPCMAVS {
	uint32_t bpcm_id;
	uint32_t bpcm_capability;
} BIUCpuBPCMAVS;

typedef struct BIUCtrl {
	BIUCpuBusRange bus_range[11];	/* 0x0 */
	uint32_t secure_reset_hndshake;
	uint32_t secure_soft_reset;
	BIUCpuAccessRightViol access_right_viol[2];	/* 0x60 */
	uint32_t rac_cfg0;
	uint32_t rac_cfg1;
	uint32_t rac_cfg2;	/* 0x80 */
	uint32_t rac_flush;
	uint32_t power_cfg;
#define BIU_CPU_CTRL_PWR_CFG_CPU0_BPCM_INIT_ON_SHIFT 4
#define BIU_CPU_CTRL_PWR_CFG_CPU0_BPCM_INIT_ON      (0x1<<BIU_CPU_CTRL_PWR_CFG_CPU0_BPCM_INIT_ON_SHIFT)
	uint32_t reset_cfg;
#define BIU_CPU_CTRL_RST_CFG_CPU0_RESET_SHIFT       0
#define BIU_CPU_CTRL_RST_CFG_CPU0_RESET             (0x1<<BIU_CPU_CTRL_RST_CFG_CPU0_RESET_SHIFT)
	uint32_t clock_cfg;	/* 0x90 */
#define BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_SHIFT    8
#define BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_MASK     (0xf<<BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_DIV1     (0<<BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_DIV2     (1<<BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_DIV3     (2<<BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_DIV4     (3<<BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_SAFE_CLOCK_MODE_SHIFT  4
#define BIU_CPU_CTRL_CLK_CFG_SAFE_CLOCK_MODE_MASK   (1<<BIU_CPU_CTRL_CLK_CFG_SAFE_CLOCK_MODE_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT        0
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_MASK         (0xf<<BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_DIV1         (0<<BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_DIV2         (1<<BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_DIV4         (2<<BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_DIV8         (3<<BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT)
#define BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_DIV16        (4<<BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_SHIFT)
	uint32_t cluster_clk_ctrl[2];
	uint32_t cluster_clk_pattern[2];
	uint32_t cluster_clk_ramp[2];
	uint32_t misc_cfg;
	uint32_t credit;	/* 0xb0 */
	uint32_t mcp_flow;
	uint32_t periphbase_gic;
	uint32_t periphbase_gic_web;
	uint32_t wfx_state;	/* 0xc0 */
	uint32_t cpu_pwr_zone_ctrl[8];
#define BIU_CPU_CTRL_PWR_ZONE_CTRL_ZONE_RESET_SHIFT 31
#define BIU_CPU_CTRL_PWR_ZONE_CTRL_ZONE_RESET       (1<<BIU_CPU_CTRL_PWR_ZONE_CTRL_ZONE_RESET_SHIFT)
#define BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_UP_REQ_SHIFT 10
#define BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_UP_REQ       (1<<BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_UP_REQ_SHIFT)
#define BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_DN_REQ_SHIFT 9
#define BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_DN_REQ       (1<<BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_DN_REQ_SHIFT)
	uint32_t cpu_pwr_zone_cfg1[8];
	uint32_t cpu_pwr_zone_cfg2[8];
	uint32_t c0l2_pwr_zone_ctrl;	/* 0x124 */
	uint32_t c0l2_pwr_zone_cfg1;
	uint32_t c0l2_pwr_zone_cfg2;
	uint32_t c1l2_pwr_zone_ctrl;
	uint32_t c1l2_pwr_zone_cfg1;
	uint32_t c1l2_pwr_zone_cfg2;
	uint32_t sysif_pwr_zone_ctrl;
	uint32_t sysif_pwr_zone_cfg1;
	uint32_t sysif_pwr_zone_cfg2;
	BIUCpuBPCMAVS cpu_bpcm_avs[8];	/* 0x148 */
	BIUCpuBPCMAVS l2biu_bpcm_avs[3];	/* 0x188 */
	uint32_t therm_throttle_temp;	/* 0x1a0 */
	uint32_t term_throttle_irq_cfg;
	uint32_t therm_irq_high;
	uint32_t therm_irq_low;
	uint32_t therm_misc_threshold;	/* 0x1b0 */
	uint32_t therm_irq_misc;
	uint32_t defeature;
	uint32_t defeature2;
	uint32_t defeature_key;	/* 0x1c0 */
	uint32_t debug_rom_addr;
	uint32_t debug_tracectrl;
	uint32_t axi_cfg;
	uint32_t revision;	/* 0x1d0 */
	uint32_t patchlevel;
	uint32_t ubus_cfg;	/* 0x1d8 */
	uint32_t ubus_cfg_window[8];
	uint32_t power_state;
	uint32_t phys_config;	/* 0x200 */
	uint32_t unused[126];	/* 0x204 */
	uint32_t scratch;	/* 0x3fc */
} BIUCtrl;

#define BIUCTRL ((volatile BIUCtrl * const) BIUCTRL_BASE)

typedef struct Boot_LUT {
	uint32_t bootLut[8];	/* 0x00 */
	uint32_t bootLutRst;	/* 0x20 */
	uint32_t bootLutUnd;	/* 0x24 */
	uint32_t bootLutSwi;	/* 0x28 */
	uint32_t bootLutPrf;	/* 0x2c */
	uint32_t bootLutAbt;	/* 0x30 */
	uint32_t bootLutUnu;	/* 0x34 */
	uint32_t bootLutIrq;	/* 0x38 */
	uint32_t bootLutFiq;	/* 0x3c */
	uint32_t bootLutPer;	/* 0x40 */
} Boot_LUT;
#define BOOT_LUT ((volatile Boot_LUT * const) BOOTLUT_BASE)
