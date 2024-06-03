/* SPDX-License-Identifier: GPL-2.0+ */
/**
 * Copyright 2014 Freescale Semiconductor
 *
 * Author: Chunhe Lan <Chunhe.Lan@freescale.com>
 *
 * This file provides support for the ngPIXIS, a board-specific FPGA used on
 * some Freescale reference boards.
 */

/*
 * CPLD register set. Feel free to add board-specific #ifdefs where necessary.
 */
struct cpld_data {
	u8 chip_id1;	/* 0x00 - CPLD Chip ID1 Register */
	u8 chip_id2;	/* 0x01 - CPLD Chip ID2 Register */
	u8 sw_maj_ver;	/* 0x02 - CPLD Code Major Version Register */
	u8 sw_min_ver;	/* 0x03 - CPLD Code Minor Version Register */
	u8 hw_ver;	/* 0x04 - PCBA Version Register */
	u8 software_on;	/* 0x05 - Override Physical Switch Enable Register */
	u8 cfg_rcw_src;	/* 0x06 - RCW Source Location Control Register */
	u8 res0;	/* 0x07 - not used */
	u8 vbank;	/* 0x08 - Flash Bank Selection Control Register */
	u8 sw1_sysclk;	/* 0x09 - SW1 Status Read Back Register */
	u8 sw2_status;	/* 0x0a - SW2 Status Read Back Register */
	u8 sw3_status;	/* 0x0b - SW3 Status Read Back Register */
	u8 sw4_status;	/* 0x0c - SW4 Status Read Back Register */
	u8 sys_reset;	/* 0x0d - Reset System With Reserving Registers Value*/
	u8 global_reset;/* 0x0e - Reset System With Default Registers Value */
	u8 res1;	/* 0x0f - not used */
};

#define CPLD_BANK_SEL_MASK	0x07
#define CPLD_BANK_SEL_EN	0x04
#define CPLD_SYSTEM_RESET	0x01
#define CPLD_SELECT_BANK0	0x00
#define CPLD_SELECT_BANK4	0x04
#define CPLD_DEFAULT_BANK	0x01

/* Pointer to the CPLD register set */

u8 cpld_read(unsigned int reg);
void cpld_write(unsigned int reg, u8 value);

#define CPLD_READ(reg) cpld_read(offsetof(struct cpld_data, reg))
#define CPLD_WRITE(reg, value) \
		cpld_write(offsetof(struct cpld_data, reg), value)

