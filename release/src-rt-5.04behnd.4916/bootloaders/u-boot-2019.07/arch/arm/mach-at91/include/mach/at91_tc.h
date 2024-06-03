/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Jens Scharsig (js_at_ng@scharsoft.de)
 */

#ifndef AT91_TC_H
#define AT91_TC_H

typedef struct at91_tcc {
	u32		ccr;	/* 0x00 Channel Control Register */
	u32		cmr;	/* 0x04 Channel Mode Register */
	u32		reserved1[2];
	u32		cv;	/* 0x10 Counter Value */
	u32		ra;	/* 0x14 Register A */
	u32		rb;	/* 0x18 Register B */
	u32		rc;	/* 0x1C Register C */
	u32		sr;	/* 0x20 Status Register */
	u32		ier;	/* 0x24 Interrupt Enable Register */
	u32		idr;	/* 0x28 Interrupt Disable Register */
	u32		imr;	/* 0x2C Interrupt Mask Register */
	u32		reserved3[4];
} at91_tcc_t;

#define AT91_TC_CCR_CLKEN		0x00000001
#define AT91_TC_CCR_CLKDIS		0x00000002
#define AT91_TC_CCR_SWTRG		0x00000004

#define AT91_TC_CMR_CPCTRG		0x00004000

#define AT91_TC_CMR_TCCLKS_CLOCK1	0x00000000
#define AT91_TC_CMR_TCCLKS_CLOCK2	0x00000001
#define AT91_TC_CMR_TCCLKS_CLOCK3	0x00000002
#define AT91_TC_CMR_TCCLKS_CLOCK4	0x00000003
#define AT91_TC_CMR_TCCLKS_CLOCK5	0x00000004
#define AT91_TC_CMR_TCCLKS_XC0		0x00000005
#define AT91_TC_CMR_TCCLKS_XC1		0x00000006
#define AT91_TC_CMR_TCCLKS_XC2		0x00000007

typedef struct at91_tc {
	at91_tcc_t	tc[3];	/* 0x00 TC Channel 0-2 */
	u32		bcr;	/* 0xC0 TC Block Control Register */
	u32		bmr;	/* 0xC4 TC Block Mode Register */
} at91_tc_t;

#define AT91_TC_BMR_TC0XC0S_TCLK0	0x00000000
#define AT91_TC_BMR_TC0XC0S_NONE	0x00000001
#define AT91_TC_BMR_TC0XC0S_TIOA1	0x00000002
#define AT91_TC_BMR_TC0XC0S_TIOA2	0x00000003

#define AT91_TC_BMR_TC1XC1S_TCLK1	0x00000000
#define AT91_TC_BMR_TC1XC1S_NONE	0x00000004
#define AT91_TC_BMR_TC1XC1S_TIOA0	0x00000008
#define AT91_TC_BMR_TC1XC1S_TIOA2	0x0000000C

#define AT91_TC_BMR_TC2XC2S_TCLK2	0x00000000
#define AT91_TC_BMR_TC2XC2S_NONE	0x00000010
#define AT91_TC_BMR_TC2XC2S_TIOA0	0x00000020
#define AT91_TC_BMR_TC2XC2S_TIOA1	0x00000030

#endif
