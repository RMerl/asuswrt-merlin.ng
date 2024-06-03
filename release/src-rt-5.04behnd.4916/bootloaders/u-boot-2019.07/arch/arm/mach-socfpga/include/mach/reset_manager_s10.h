/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2016-2018 Intel Corporation <www.intel.com>
 *
 */

#ifndef	_RESET_MANAGER_S10_
#define	_RESET_MANAGER_S10_

void reset_cpu(ulong addr);
int cpu_has_been_warmreset(void);

void socfpga_bridges_reset(int enable);

void socfpga_per_reset(u32 reset, int set);
void socfpga_per_reset_all(void);

struct socfpga_reset_manager {
	u32	status;
	u32	mpu_rst_stat;
	u32	misc_stat;
	u32	padding1;
	u32	hdsk_en;
	u32	hdsk_req;
	u32	hdsk_ack;
	u32	hdsk_stall;
	u32	mpumodrst;
	u32	per0modrst;
	u32	per1modrst;
	u32	brgmodrst;
	u32	padding2;
	u32     cold_mod_reset;
	u32	padding3;
	u32     dbg_mod_reset;
	u32     tap_mod_reset;
	u32	padding4;
	u32	padding5;
	u32     brg_warm_mask;
	u32	padding6[3];
	u32     tst_stat;
	u32	padding7;
	u32     hdsk_timeout;
	u32     mpul2flushtimeout;
	u32     dbghdsktimeout;
};

#define RSTMGR_MPUMODRST_CORE0		0
#define RSTMGR_PER0MODRST_OCP_MASK	0x0020bf00
#define RSTMGR_BRGMODRST_DDRSCH_MASK	0X00000040
#define RSTMGR_BRGMODRST_FPGA2SOC_MASK	0x00000004

/* Watchdogs and MPU warm reset mask */
#define RSTMGR_L4WD_MPU_WARMRESET_MASK	0x000F0F00

/*
 * Define a reset identifier, from which a permodrst bank ID
 * and reset ID can be extracted using the subsequent macros
 * RSTMGR_RESET() and RSTMGR_BANK().
 */
#define RSTMGR_BANK_OFFSET	8
#define RSTMGR_BANK_MASK	0x7
#define RSTMGR_RESET_OFFSET	0
#define RSTMGR_RESET_MASK	0x1f
#define RSTMGR_DEFINE(_bank, _offset)		\
	((_bank) << RSTMGR_BANK_OFFSET) | ((_offset) << RSTMGR_RESET_OFFSET)

/* Extract reset ID from the reset identifier. */
#define RSTMGR_RESET(_reset)			\
	(((_reset) >> RSTMGR_RESET_OFFSET) & RSTMGR_RESET_MASK)

/* Extract bank ID from the reset identifier. */
#define RSTMGR_BANK(_reset)			\
	(((_reset) >> RSTMGR_BANK_OFFSET) & RSTMGR_BANK_MASK)

/*
 * SocFPGA Stratix10 reset IDs, bank mapping is as follows:
 * 0 ... mpumodrst
 * 1 ... per0modrst
 * 2 ... per1modrst
 * 3 ... brgmodrst
 */
#define RSTMGR_EMAC0		RSTMGR_DEFINE(1, 0)
#define RSTMGR_EMAC1		RSTMGR_DEFINE(1, 1)
#define RSTMGR_EMAC2		RSTMGR_DEFINE(1, 2)
#define RSTMGR_USB0		RSTMGR_DEFINE(1, 3)
#define RSTMGR_USB1		RSTMGR_DEFINE(1, 4)
#define RSTMGR_NAND		RSTMGR_DEFINE(1, 5)
#define RSTMGR_SDMMC		RSTMGR_DEFINE(1, 7)
#define RSTMGR_EMAC0_OCP	RSTMGR_DEFINE(1, 8)
#define RSTMGR_EMAC1_OCP	RSTMGR_DEFINE(1, 9)
#define RSTMGR_EMAC2_OCP	RSTMGR_DEFINE(1, 10)
#define RSTMGR_USB0_OCP		RSTMGR_DEFINE(1, 11)
#define RSTMGR_USB1_OCP		RSTMGR_DEFINE(1, 12)
#define RSTMGR_NAND_OCP		RSTMGR_DEFINE(1, 13)
#define RSTMGR_SDMMC_OCP	RSTMGR_DEFINE(1, 15)
#define RSTMGR_DMA		RSTMGR_DEFINE(1, 16)
#define RSTMGR_SPIM0		RSTMGR_DEFINE(1, 17)
#define RSTMGR_SPIM1		RSTMGR_DEFINE(1, 18)
#define RSTMGR_L4WD0		RSTMGR_DEFINE(2, 0)
#define RSTMGR_L4WD1		RSTMGR_DEFINE(2, 1)
#define RSTMGR_L4WD2		RSTMGR_DEFINE(2, 2)
#define RSTMGR_L4WD3		RSTMGR_DEFINE(2, 3)
#define RSTMGR_OSC1TIMER0	RSTMGR_DEFINE(2, 4)
#define RSTMGR_I2C0		RSTMGR_DEFINE(2, 8)
#define RSTMGR_I2C1		RSTMGR_DEFINE(2, 9)
#define RSTMGR_I2C2		RSTMGR_DEFINE(2, 10)
#define RSTMGR_I2C3		RSTMGR_DEFINE(2, 11)
#define RSTMGR_I2C4		RSTMGR_DEFINE(2, 12)
#define RSTMGR_UART0		RSTMGR_DEFINE(2, 16)
#define RSTMGR_UART1		RSTMGR_DEFINE(2, 17)
#define RSTMGR_GPIO0		RSTMGR_DEFINE(2, 24)
#define RSTMGR_GPIO1		RSTMGR_DEFINE(2, 25)
#define RSTMGR_SDR		RSTMGR_DEFINE(3, 6)

/* Create a human-readable reference to SoCFPGA reset. */
#define SOCFPGA_RESET(_name)	RSTMGR_##_name

#endif /* _RESET_MANAGER_S10_ */
