/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Ryan CHEN, ST Micoelectronics, ryan.chen@st.com
 */

#ifndef __SYSCTRL_H
#define __SYSCTRL_H

struct syscntl_regs {
	u32 scctrl;
	u32 scsysstat;
	u32 scimctrl;
	u32 scimsysstat;
	u32 scxtalctrl;
	u32 scpllctrl;
	u32 scpllfctrl;
	u32 scperctrl0;
	u32 scperctrl1;
	u32 scperen;
	u32 scperdis;
	const u32 scperclken;
	const u32 scperstat;
};

#define MODE_SHIFT          0x00000003

#define NORMAL              0x00000004
#define SLOW                0x00000002
#define DOZE                0x00000001
#define SLEEP               0x00000000

#define PLL_TIM             0x01FFFFFF

#endif
