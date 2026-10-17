/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ifndef _STV0991_WD_RST_H
#define _STV0991_WD_RST_H
#include <asm/arch-stv0991/hardware.h>

struct stv0991_wd_ru {
	u32 wdru_config;
	u32 wdru_ctrl1;
	u32 wdru_ctrl2;
	u32 wdru_tim;
	u32 wdru_count;
	u32 wdru_stat;
	u32 wdru_wrlock;
};

struct stv0991_wd_ru *const stv0991_wd_ru_ptr = \
		(struct stv0991_wd_ru *)WDRU_BASE_ADDR;

/* Watchdog control register */
#define WDRU_RST_SYS		0x1

#endif
