/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2012-2017 Altera Corporation <www.altera.com>
 */

#ifndef _RESET_MANAGER_GEN5_H_
#define _RESET_MANAGER_GEN5_H_

#include <dt-bindings/reset/altr,rst-mgr.h>

void socfpga_bridges_set_handoff_regs(bool h2f, bool lwh2f, bool f2h);
void socfpga_bridges_reset(int enable);

struct socfpga_reset_manager {
	u32	status;
	u32	ctrl;
	u32	counts;
	u32	padding1;
	u32	mpu_mod_reset;
	u32	per_mod_reset;
	u32	per2_mod_reset;
	u32	brg_mod_reset;
	u32	misc_mod_reset;
	u32	padding2[12];
	u32	tstscratch;
};

/*
 * SocFPGA Cyclone V/Arria V reset IDs, bank mapping is as follows:
 * 0 ... mpumodrst
 * 1 ... permodrst
 * 2 ... per2modrst
 * 3 ... brgmodrst
 * 4 ... miscmodrst
 */
#define RSTMGR_EMAC0		RSTMGR_DEFINE(1, 0)
#define RSTMGR_EMAC1		RSTMGR_DEFINE(1, 1)
#define RSTMGR_NAND		RSTMGR_DEFINE(1, 4)
#define RSTMGR_QSPI		RSTMGR_DEFINE(1, 5)
#define RSTMGR_L4WD0		RSTMGR_DEFINE(1, 6)
#define RSTMGR_OSC1TIMER0	RSTMGR_DEFINE(1, 8)
#define RSTMGR_UART0		RSTMGR_DEFINE(1, 16)
#define RSTMGR_SPIM0		RSTMGR_DEFINE(1, 18)
#define RSTMGR_SPIM1		RSTMGR_DEFINE(1, 19)
#define RSTMGR_SDMMC		RSTMGR_DEFINE(1, 22)
#define RSTMGR_DMA		RSTMGR_DEFINE(1, 28)
#define RSTMGR_SDR		RSTMGR_DEFINE(1, 29)

#endif /* _RESET_MANAGER_GEN5_H_ */
