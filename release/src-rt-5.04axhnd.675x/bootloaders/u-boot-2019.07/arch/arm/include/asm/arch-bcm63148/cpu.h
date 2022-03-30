/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#ifndef _63148_CPU_H
#define _63148_CPU_H

#define B15_CTRL_BASE		0x80020000
#define BOOTLUT_BASE		0xffff0000

/*
 * B15 CFG
 */
typedef struct B15ArchRegion {
	uint32_t addr_ulimit;
	uint32_t addr_llimit;
	uint32_t permission;
	uint32_t access_right_ctrl;
} B15ArchRegion;

typedef struct B15Arch {
	B15ArchRegion region[8];
	uint32_t unused[95];
	uint32_t scratch;
} B15Arch;

typedef struct B15CpuBusRange {
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
} B15CpuBusRange;

typedef struct B15CpuAccessRightViol {
	uint32_t addr;
	uint32_t upper_addr;
	uint32_t detail_addr;
} B15CpuAccessRightViol;

typedef struct B15CpuBPCMAVS {
	uint32_t bpcm_id;
	uint32_t bpcm_capability;
	uint32_t bpcm_ctrl;
	uint32_t bpcm_status;
	uint32_t avs_rosc_ctrl;
	uint32_t avs_rosc_threshold;
	uint32_t avs_rosc_cnt;
	uint32_t avs_pwd_ctrl;
} B15CpuBPCMAVS;

typedef struct B15CpuCtrl {
	B15CpuBusRange bus_range[11];	/* 0x0 */
	uint32_t secure_reset_hndshake;
	uint32_t secure_soft_reset;
	B15CpuAccessRightViol access_right_viol[2];	/* 0x60 */
	uint32_t rac_cfg0;
	uint32_t rac_cfg1;
	uint32_t rac_flush;		/* 0x80 */
	uint32_t cpu_power_cfg;
	uint32_t cpu0_pwr_zone_ctrl;
	uint32_t cpu1_pwr_zone_ctrl;
	uint32_t cpu2_pwr_zone_ctrl;	/* 0x90 */
	uint32_t cpu3_pwr_zone_ctrl;
	uint32_t l2biu_pwr_zone_ctrl;
	uint32_t cpu0_pwr_zone_cfg1;
	uint32_t cpu0_pwr_zone_cfg2;	/* 0xa0 */
	uint32_t cpu1_pwr_zone_cfg1;
	uint32_t cpu1_pwr_zone_cfg2;
	uint32_t cpu2_pwr_zone_cfg1;
	uint32_t cpu2_pwr_zone_cfg2;	/* 0xb0 */
	uint32_t cpu3_pwr_zone_cfg1;
	uint32_t cpu3_pwr_zone_cfg2;
	uint32_t l2biu_pwr_zone_cfg1;
	uint32_t l2biu_pwr_zone_cfg2;	/* 0xc0 */
	uint32_t cpu0_pwr_freq_scalar_ctrl;
	uint32_t cpu1_pwr_freq_scalar_ctrl;
	uint32_t cpu2_pwr_freq_scalar_ctrl;
	uint32_t cpu3_pwr_freq_scalar_ctrl;	/* 0xd0 */
	uint32_t l2biu_pwr_freq_scalar_ctrl;
	B15CpuBPCMAVS cpu_bpcm_avs[4];	/* 0xd8 */
	B15CpuBPCMAVS l2biu_bpcm_avs;	/* 0x158 */
	uint32_t reset_cfg;		/* 0x178 */
	uint32_t clock_cfg;
	uint32_t misc_cfg;		/* 0x180 */
	uint32_t credit;
	uint32_t therm_throttle_temp;
	uint32_t term_throttle_irq_cfg;
	uint32_t therm_irq_high;		/* 0x190 */
	uint32_t therm_irq_low;
	uint32_t therm_misc_threshold;
	uint32_t therm_irq_misc;
	uint32_t defeature;		/* 0x1a0 */
	uint32_t defeature_key;
	uint32_t debug_rom_addr;
	uint32_t debug_self_addr;
	uint32_t debug_tracectrl;		/* 0x1b0 */
	uint32_t axi_cfg;
	uint32_t revision;
	uint32_t ubus_cfg_window[8];	/* 0x1bc */
	uint32_t ubus_cfg;		/* 0x1dc */
	uint32_t unused[135];
	uint32_t scratch;			/* 0x3fc */
} B15CpuCtrl;

typedef struct B15Ctrl {
	uint32_t unused0[1024];
	B15Arch arch;			/* 0x1000 */
	uint32_t unused1[896];
	B15CpuCtrl cpu_ctrl;		/* 0x2000 */
} B15Ctrl;

#define B15CTRL ((volatile B15Ctrl *const) B15_CTRL_BASE)

#endif
