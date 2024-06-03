/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * (C) Copyright 2012 Renesas Solutions Corp.
 */
#ifndef _GLOBALTIMER_H_
#define _GLOBALTIMER_H_

struct globaltimer {
	u32 cnt_l; /* 0x00 */
	u32 cnt_h;
	u32 ctl;
	u32 stat;
	u32 cmp_l; /* 0x10 */
	u32 cmp_h;
	u32 inc;
};

#endif /* _GLOBALTIMER_H_ */
