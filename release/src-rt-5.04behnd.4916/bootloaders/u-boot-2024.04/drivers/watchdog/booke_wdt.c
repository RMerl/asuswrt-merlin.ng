// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Watchdog timer for PowerPC Book-E systems
 */

#include <div64.h>
#include <dm.h>
#include <wdt.h>
#include <asm/processor.h>

#define WDTP_MASK TCR_WP(0x3f)

/* For the specified period, determine the number of seconds
 * corresponding to the reset time.  There will be a watchdog
 * exception at approximately 3/5 of this time.
 *
 * The formula to calculate this is given by:
 * 2.5 * (2^(63-period+1)) / timebase_freq
 *
 * In order to simplify things, we assume that period is
 * at least 1.  This will still result in a very long timeout.
 */
static unsigned long long period_to_sec(unsigned int period)
{
	unsigned long long tmp = 1ULL << (64 - period);
	unsigned long tmp2 = get_tbclk();

	/* tmp may be a very large number and we don't want to overflow,
	 * so divide the timebase freq instead of multiplying tmp
	 */
	tmp2 = tmp2 / 5 * 2;

	do_div(tmp, tmp2);
	return tmp;
}

/*
 * This procedure will find the highest period which will give a timeout
 * greater than the one required. e.g. for a bus speed of 66666666 and
 * and a parameter of 2 secs, then this procedure will return a value of 38.
 */
static unsigned int sec_to_period(unsigned int secs)
{
	unsigned int period;

	for (period = 63; period > 0; period--) {
		if (period_to_sec(period) >= secs)
			return period;
	}
	return 0;
}

static int booke_wdt_reset(struct udevice *dev)
{
	mtspr(SPRN_TSR, TSR_ENW | TSR_WIS);

	return 0;
}

static int booke_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	u32 val;
	unsigned int timeout = DIV_ROUND_UP(timeout_ms, 1000);

	/* clear status before enabling watchdog */
	booke_wdt_reset(dev);
	val = mfspr(SPRN_TCR);
	val &= ~WDTP_MASK;
	val |= (TCR_WIE | TCR_WRC(WRC_CHIP) | TCR_WP(sec_to_period(timeout)));

	mtspr(SPRN_TCR, val);

	return 0;
}

static int booke_wdt_stop(struct udevice *dev)
{
	u32 val;

	val = mfspr(SPRN_TCR);
	val &= ~(TCR_WIE | WDTP_MASK);
	mtspr(SPRN_TCR, val);

	/* clear status to make sure nothing is pending */
	booke_wdt_reset(dev);

	return 0;
}

static const struct wdt_ops booke_wdt_ops = {
	.start = booke_wdt_start,
	.stop = booke_wdt_stop,
	.reset = booke_wdt_reset,
};

static const struct udevice_id booke_wdt_ids[] = {
	{ .compatible = "fsl,booke-wdt" },
	{}
};

U_BOOT_DRIVER(booke_wdt) = {
	.name = "booke_wdt",
	.id = UCLASS_WDT,
	.of_match = booke_wdt_ids,
	.ops = &booke_wdt_ops,
};
