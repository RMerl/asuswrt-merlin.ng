// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 */

#include <common.h>

#define TIMER_ENABLE	(1 << 7)
#define TIMER_MODE_MSK	(1 << 6)
#define TIMER_MODE_FR	(0 << 6)
#define TIMER_MODE_PD	(1 << 6)

#define TIMER_INT_EN	(1 << 5)
#define TIMER_PRS_MSK	(3 << 2)
#define TIMER_PRS_8S	(1 << 3)
#define TIMER_SIZE_MSK	(1 << 2)
#define TIMER_ONE_SHT	(1 << 0)

int timer_init (void)
{
	ulong	tmr_ctrl_val;

	/* 1st disable the Timer */
	tmr_ctrl_val = *(volatile ulong *)(CONFIG_SYS_TIMERBASE + 8);
	tmr_ctrl_val &= ~TIMER_ENABLE;
	*(volatile ulong *)(CONFIG_SYS_TIMERBASE + 8) = tmr_ctrl_val;

	/*
	 * The Timer Control Register has one Undefined/Shouldn't Use Bit
	 * So we should do read/modify/write Operation
	 */

	/*
	 * Timer Mode : Free Running
	 * Interrupt : Disabled
	 * Prescale : 8 Stage, Clk/256
	 * Tmr Siz : 16 Bit Counter
	 * Tmr in Wrapping Mode
	 */
	tmr_ctrl_val = *(volatile ulong *)(CONFIG_SYS_TIMERBASE + 8);
	tmr_ctrl_val &= ~(TIMER_MODE_MSK | TIMER_INT_EN | TIMER_PRS_MSK | TIMER_SIZE_MSK | TIMER_ONE_SHT );
	tmr_ctrl_val |= (TIMER_ENABLE | TIMER_PRS_8S);

	*(volatile ulong *)(CONFIG_SYS_TIMERBASE + 8) = tmr_ctrl_val;

	return 0;
}

