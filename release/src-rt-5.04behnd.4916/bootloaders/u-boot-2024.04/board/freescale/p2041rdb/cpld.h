/* SPDX-License-Identifier: GPL-2.0+ */
/**
 * Copyright 2011 Freescale Semiconductor
 * Author: Mingkai Hu <Mingkai.hu@freescale.com>
 *
 * This file provides support for the ngPIXIS, a board-specific FPGA used on
 * some Freescale reference boards.
 */

/*
 * CPLD register set. Feel free to add board-specific #ifdefs where necessary.
 */
typedef struct cpld_data {
	u8 cpld_ver;		/* 0x0 - CPLD Major Revision Register */
	u8 cpld_ver_sub;	/* 0x1 - CPLD Minor Revision Register */
	u8 pcba_ver;		/* 0x2 - PCBA Revision Register */
	u8 system_rst;		/* 0x3 - system reset register */
	u8 res0;		/* 0x4 - not used */
	u8 sw_ctl_on;		/* 0x5 - Switch Control Enable Register */
	u8 por_cfg;		/* 0x6 - POR Control Register */
	u8 switch_strobe;	/* 0x7 - Multiplexed pin Select Register */
	u8 jtag_sel;		/* 0x8 - JTAG or AURORA Selection */
	u8 sdbank1_clk;		/* 0x9 - SerDes Bank1 Reference clock */
	u8 sdbank2_clk;		/* 0xa - SerDes Bank2 Reference clock */
	u8 fbank_sel;		/* 0xb - Flash bank selection */
	u8 serdes_mux;		/* 0xc - Multiplexed pin Select Register */
	u8 sw[1];		/* 0xd - SW2 Status */
	u8 system_rst_default;	/* 0xe - system reset to default register */
	u8 sysclk_sw1;		/* 0xf - sysclk configuration register */
} __attribute__ ((packed)) cpld_data_t;

#define SERDES_MUX_LANE_6_MASK	0x2
#define SERDES_MUX_LANE_6_SHIFT	1
#define SERDES_MUX_LANE_A_MASK	0x1
#define SERDES_MUX_LANE_A_SHIFT	0
#define SERDES_MUX_LANE_C_MASK	0x4
#define SERDES_MUX_LANE_C_SHIFT	2
#define SERDES_MUX_LANE_D_MASK	0x8
#define SERDES_MUX_LANE_D_SHIFT	3
#define CPLD_SWITCH_BANK_ENABLE	0x40
#define CPLD_SYSCLK_83		0x1	/* system clock 83.3MHz */
#define CPLD_SYSCLK_100		0x2	/* system clock 100MHz */

/* Pointer to the CPLD register set */
#define cpld ((cpld_data_t *)CPLD_BASE)

/* The CPLD SW register that corresponds to board switch X, where x >= 1 */
#define CPLD_SW(x)		(cpld->sw[(x) - 2])

u8 cpld_read(unsigned int reg);
void cpld_write(unsigned int reg, u8 value);

#define CPLD_READ(reg) cpld_read(offsetof(cpld_data_t, reg))
#define CPLD_WRITE(reg, value) cpld_write(offsetof(cpld_data_t, reg), value)
