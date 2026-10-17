/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010 Linaro
 * Matt Waddel, <matt.waddel@linaro.org>
 */
#ifndef _SYSTIMER_H_
#define _SYSTIMER_H_

/* AMBA timer register base address */
#define SYSTIMER_BASE		0x10011000

#define SYSHZ_CLOCK		1000000		/* Timers -> 1Mhz */
#define SYSTIMER_RELOAD		0xFFFFFFFF
#define SYSTIMER_EN		(1 << 7)
#define SYSTIMER_32BIT		(1 << 1)
#define SYSTIMER_PRESC_16	(1 << 2)
#define SYSTIMER_PRESC_256	(1 << 3)

struct systimer {
	u32 timer0load;		/* 0x00 */
	u32 timer0value;
	u32 timer0control;
	u32 timer0intclr;
	u32 timer0ris;
	u32 timer0mis;
	u32 timer0bgload;
	u32 timer1load;		/* 0x20 */
	u32 timer1value;
	u32 timer1control;
	u32 timer1intclr;
	u32 timer1ris;
	u32 timer1mis;
	u32 timer1bgload;
};
#endif /* _SYSTIMER_H_ */
