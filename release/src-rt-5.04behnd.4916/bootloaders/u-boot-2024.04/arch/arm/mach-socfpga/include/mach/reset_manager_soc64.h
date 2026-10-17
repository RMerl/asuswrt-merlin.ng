/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  Copyright (C) 2016-2019 Intel Corporation <www.intel.com>
 */

#ifndef _RESET_MANAGER_SOC64_H_
#define _RESET_MANAGER_SOC64_H_

void reset_deassert_peripherals_handoff(void);
int cpu_has_been_warmreset(void);
void print_reset_info(void);
void socfpga_bridges_reset(int enable);

#define RSTMGR_SOC64_STATUS	0x00
#define RSTMGR_SOC64_MPUMODRST	0x20
#define RSTMGR_SOC64_PER0MODRST	0x24
#define RSTMGR_SOC64_PER1MODRST	0x28
#define RSTMGR_SOC64_BRGMODRST	0x2c

#define RSTMGR_MPUMODRST_CORE0		0
#define RSTMGR_PER0MODRST_OCP_MASK	0x0020bf00
#define RSTMGR_BRGMODRST_DDRSCH_MASK	0X00000040
#define RSTMGR_BRGMODRST_FPGA2SOC_MASK	0x00000004

/* SDM, Watchdogs and MPU warm reset mask */
#define RSTMGR_STAT_SDMWARMRST		BIT(1)
#define RSTMGR_STAT_MPU0RST_BITPOS	8
#define RSTMGR_STAT_L4WD0RST_BITPOS	16
#define RSTMGR_L4WD_MPU_WARMRESET_MASK	(RSTMGR_STAT_SDMWARMRST | \
		GENMASK(RSTMGR_STAT_MPU0RST_BITPOS + 3, \
			RSTMGR_STAT_MPU0RST_BITPOS) | \
		GENMASK(RSTMGR_STAT_L4WD0RST_BITPOS + 3, \
			RSTMGR_STAT_L4WD0RST_BITPOS))

/*
 * SocFPGA Stratix10 reset IDs, bank mapping is as follows:
 * 0 ... mpumodrst
 * 1 ... per0modrst
 * 2 ... per1modrst
 * 3 ... brgmodrst
 */
#define RSTMGR_L4WD0		RSTMGR_DEFINE(2, 0)
#define RSTMGR_OSC1TIMER0	RSTMGR_DEFINE(2, 4)
#define RSTMGR_UART0		RSTMGR_DEFINE(2, 16)

#endif /* _RESET_MANAGER_SOC64_H_ */
