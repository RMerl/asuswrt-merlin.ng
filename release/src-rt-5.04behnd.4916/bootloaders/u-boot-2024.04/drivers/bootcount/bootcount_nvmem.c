// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 * (C) Copyright 2018 Robert Bosch Power Tools GmbH.
 *
 * A bootcount driver for the RTC IP block found on many TI platforms.
 * This requires the RTC clocks, etc, to be enabled prior to use and
 * not all boards with this IP block on it will have the RTC in use.
 */

#include <bootcount.h>
#include <asm/davinci_rtc.h>

#define	BC_VERSION	2

void bootcount_store(ulong bootcount)
{
	u8 upgrade_available = 0;
	ulong val = 0;
	struct davinci_rtc *reg =
		(struct davinci_rtc *)CONFIG_SYS_BOOTCOUNT_ADDR;

	val = raw_bootcount_load(&reg->scratch2);
	upgrade_available = (val >> 8) & 0x000000ff;

	/* Only update bootcount during upgrade process */
	if (!upgrade_available)
		bootcount = 0;

	val = (bootcount & 0x000000ff) |
	      (upgrade_available << 8) |
	      (BC_VERSION << 16) |
	      (CONFIG_SYS_BOOTCOUNT_MAGIC << 24);

	/*
	 * write RTC kick registers to enable write
	 * for RTC Scratch registers. Scratch register 2 is
	 * used for bootcount value.
	 */
	writel(RTC_KICK0R_WE, &reg->kick0r);
	writel(RTC_KICK1R_WE, &reg->kick1r);
	raw_bootcount_store(&reg->scratch2, val);
}

ulong bootcount_load(void)
{
	unsigned long val = 0;
	struct davinci_rtc *reg =
		(struct davinci_rtc *)CONFIG_SYS_BOOTCOUNT_ADDR;

	val = raw_bootcount_load(&reg->scratch2);
	if ((val >> 24) != CONFIG_SYS_BOOTCOUNT_MAGIC)
		return 0;
	else
		return val & 0x000000ff;
}
