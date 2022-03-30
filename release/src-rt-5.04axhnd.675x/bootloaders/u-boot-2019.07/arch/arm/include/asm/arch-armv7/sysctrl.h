/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010 Linaro
 * Matt Waddel, <matt.waddel@linaro.org>
 */
#ifndef _SYSCTRL_H_
#define _SYSCTRL_H_

/* System controller (SP810) register definitions */
#define SP810_TIMER0_ENSEL	(1 << 15)
#define SP810_TIMER1_ENSEL	(1 << 17)
#define SP810_TIMER2_ENSEL	(1 << 19)
#define SP810_TIMER3_ENSEL	(1 << 21)

struct sysctrl {
	u32 scctrl;		/* 0x000 */
	u32 scsysstat;
	u32 scimctrl;
	u32 scimstat;
	u32 scxtalctrl;
	u32 scpllctrl;
	u32 scpllfctrl;
	u32 scperctrl0;
	u32 scperctrl1;
	u32 scperen;
	u32 scperdis;
	u32 scperclken;
	u32 scperstat;
	u32 res1[0x006];
	u32 scflashctrl;	/* 0x04c */
	u32 res2[0x3a4];
	u32 scsysid0;		/* 0xee0 */
	u32 scsysid1;
	u32 scsysid2;
	u32 scsysid3;
	u32 scitcr;
	u32 scitir0;
	u32 scitir1;
	u32 scitor;
	u32 sccntctrl;
	u32 sccntdata;
	u32 sccntstep;
	u32 res3[0x32];
	u32 scperiphid0;	/* 0xfe0 */
	u32 scperiphid1;
	u32 scperiphid2;
	u32 scperiphid3;
	u32 scpcellid0;
	u32 scpcellid1;
	u32 scpcellid2;
	u32 scpcellid3;
};
#endif /* _SYSCTRL_H_ */
