/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ifndef _STV0991_GPT_H
#define _STV0991_GPT_H

#include <asm/arch-stv0991/hardware.h>

struct gpt_regs {
	u32 cr1;
	u32 cr2;
	u32 reserved_1;
	u32 dier;	/* dma_int_en */
	u32 sr;		/* status reg */
	u32 egr;	/* event gen */
	u32 reserved_2[3];	/* offset 0x18--0x20*/
	u32 cnt;
	u32 psc;
	u32 arr;
};

struct gpt_regs *const gpt1_regs_ptr =
	(struct gpt_regs *) GPTIMER1_BASE_ADDR;

/* Timer control1 register  */
#define GPT_CR1_CEN			0x0001
#define GPT_MODE_AUTO_RELOAD		(1 << 7)

/* Timer prescalar reg */
#define GPT_PRESCALER_128		0x128

/* Auto reload register for free running config */
#define GPT_FREE_RUNNING		0xFFFF

/* Timer, HZ specific defines */
#define CONFIG_STV0991_HZ		1000
#define CONFIG_STV0991_HZ_CLOCK		(27*1000*1000)/GPT_PRESCALER_128

#endif
