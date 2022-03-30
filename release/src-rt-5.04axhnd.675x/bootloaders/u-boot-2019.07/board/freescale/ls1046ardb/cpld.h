/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor
 */

#ifndef __CPLD_H__
#define __CPLD_H__

/*
 * CPLD register set of LS1046ARDB board-specific.
 * CPLD Revision:  V2.1
 */
struct cpld_data {
	u8 cpld_ver;		/* 0x0 - CPLD Major Revision Register */
	u8 cpld_ver_sub;	/* 0x1 - CPLD Minor Revision Register */
	u8 pcba_ver;		/* 0x2 - PCBA Revision Register */
	u8 system_rst;		/* 0x3 - system reset register */
	u8 soft_mux_on;		/* 0x4 - Switch Control Enable Register */
	u8 cfg_rcw_src1;	/* 0x5 - RCW Source Location POR Regsiter 1 */
	u8 cfg_rcw_src2;	/* 0x6 - RCW Source Location POR Regsiter 2 */
	u8 vbank;		/* 0x7 - QSPI Flash Bank Setting Register */
	u8 sysclk_sel;		/* 0x8 - System clock POR Register */
	u8 uart_sel;		/* 0x9 - UART1 Connection Control Register */
	u8 sd1refclk_sel;	/* 0xA - */
	u8 rgmii_1588_sel;	/* 0xB - */
	u8 reg_1588_clk_sel;	/* 0xC - */
	u8 status_led;		/* 0xD - */
	u8 global_rst;		/* 0xE - */
	u8 sd_emmc;             /* 0xF - SD/EMMC Interface Control Regsiter */
	u8 vdd_en;              /* 0x10 - VDD Voltage Control Enable Register */
	u8 vdd_sel;             /* 0x11 - VDD Voltage Control Register */
};

u8 cpld_read(unsigned int reg);
void cpld_write(unsigned int reg, u8 value);
void cpld_rev_bit(unsigned char *value);
void cpld_select_core_volt(bool en_0v9);

#define CPLD_READ(reg) cpld_read(offsetof(struct cpld_data, reg))
#define CPLD_WRITE(reg, value)  \
	cpld_write(offsetof(struct cpld_data, reg), value)

/* CPLD on IFC */
#define CPLD_SW_MUX_BANK_SEL	0x40
#define CPLD_BANK_SEL_MASK	0x07
#define CPLD_BANK_SEL_ALTBANK	0x04
#define CPLD_CFG_RCW_SRC_QSPI	0x044
#define CPLD_CFG_RCW_SRC_SD	0x040
#endif
