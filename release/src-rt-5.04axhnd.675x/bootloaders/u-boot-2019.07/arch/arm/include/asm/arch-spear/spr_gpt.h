/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#ifndef _SPR_GPT_H
#define _SPR_GPT_H

struct gpt_regs {
	u8 reserved[0x80];
	u32 control;
	u32 status;
	u32 compare;
	u32 count;
	u32 capture_re;
	u32 capture_fe;
};

/*
 * TIMER_CONTROL register settings
 */

#define GPT_PRESCALER_MASK		0x000F
#define GPT_PRESCALER_1			0x0000
#define GPT_PRESCALER_2 		0x0001
#define GPT_PRESCALER_4 		0x0002
#define GPT_PRESCALER_8 		0x0003
#define GPT_PRESCALER_16		0x0004
#define GPT_PRESCALER_32		0x0005
#define GPT_PRESCALER_64		0x0006
#define GPT_PRESCALER_128		0x0007
#define GPT_PRESCALER_256		0x0008

#define GPT_MODE_SINGLE_SHOT		0x0010
#define GPT_MODE_AUTO_RELOAD		0x0000

#define GPT_ENABLE			0x0020

#define GPT_CAPT_MODE_MASK		0x00C0
#define GPT_CAPT_MODE_NONE		0x0000
#define GPT_CAPT_MODE_RE		0x0040
#define GPT_CAPT_MODE_FE		0x0080
#define GPT_CAPT_MODE_BOTH		0x00C0

#define GPT_INT_MATCH			0x0100
#define GPT_INT_FE			0x0200
#define GPT_INT_RE			0x0400

/*
 * TIMER_STATUS register settings
 */

#define GPT_STS_MATCH			0x0001
#define GPT_STS_FE			0x0002
#define GPT_STS_RE			0x0004

/*
 * TIMER_COMPARE register settings
 */

#define GPT_FREE_RUNNING		0xFFFF

/* Timer, HZ specific defines */
#define CONFIG_SPEAR_HZ			1000
#define CONFIG_SPEAR_HZ_CLOCK		8300000

#endif
