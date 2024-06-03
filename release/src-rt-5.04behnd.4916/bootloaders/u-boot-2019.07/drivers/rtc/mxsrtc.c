// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX28 RTC Driver
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#include <common.h>
#include <rtc.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>

#define	MXS_RTC_MAX_TIMEOUT	1000000

/* Set time in seconds since 1970-01-01 */
int mxs_rtc_set_time(uint32_t secs)
{
	struct mxs_rtc_regs *rtc_regs = (struct mxs_rtc_regs *)MXS_RTC_BASE;
	int ret;

	writel(secs, &rtc_regs->hw_rtc_seconds);

	/*
	 * The 0x80 here means seconds were copied to analog. This information
	 * is taken from the linux kernel driver for the STMP37xx RTC since
	 * documentation doesn't mention it.
	 */
	ret = mxs_wait_mask_clr(&rtc_regs->hw_rtc_stat_reg,
		0x80 << RTC_STAT_STALE_REGS_OFFSET, MXS_RTC_MAX_TIMEOUT);

	if (ret)
		printf("MXS RTC: Timeout waiting for update\n");

	return ret;
}

int rtc_get(struct rtc_time *time)
{
	struct mxs_rtc_regs *rtc_regs = (struct mxs_rtc_regs *)MXS_RTC_BASE;
	uint32_t secs;

	secs = readl(&rtc_regs->hw_rtc_seconds);
	rtc_to_tm(secs, time);

	return 0;
}

int rtc_set(struct rtc_time *time)
{
	uint32_t secs;

	secs = rtc_mktime(time);

	return mxs_rtc_set_time(secs);
}

void rtc_reset(void)
{
	struct mxs_rtc_regs *rtc_regs = (struct mxs_rtc_regs *)MXS_RTC_BASE;
	int ret;

	/* Set time to 1970-01-01 */
	mxs_rtc_set_time(0);

	/* Reset the RTC block */
	ret = mxs_reset_block(&rtc_regs->hw_rtc_ctrl_reg);
	if (ret)
		printf("MXS RTC: Block reset timeout\n");
}
