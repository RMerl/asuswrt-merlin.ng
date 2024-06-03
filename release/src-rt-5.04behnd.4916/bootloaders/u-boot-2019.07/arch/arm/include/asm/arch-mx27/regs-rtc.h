/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX27 RTC Register Definitions
 *
 * Copyright (C) 2012 Philippe Reynes <tremyfr@yahoo.fr>
 */

#ifndef __MX27_REGS_RTC_H__
#define __MX27_REGS_RTC_H__

#ifndef	__ASSEMBLY__
struct rtc_regs {
	u32 hourmin;
	u32 seconds;
	u32 alrm_hm;
	u32 alrm_sec;
	u32 rtcctl;
	u32 rtcisr;
	u32 rtcienr;
	u32 stpwch;
	u32 dayr;
	u32 dayalarm;
};
#endif /* __ASSEMBLY__*/

#endif	/* __MX28_REGS_RTC_H__ */
