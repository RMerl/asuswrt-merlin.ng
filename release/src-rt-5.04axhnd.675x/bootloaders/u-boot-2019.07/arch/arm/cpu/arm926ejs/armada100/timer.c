// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/armada100.h>

/*
 * Timer registers
 * Refer Section A.6 in Datasheet
 */
struct armd1tmr_registers {
	u32 clk_ctrl;	/* Timer clk control reg */
	u32 match[9];	/* Timer match registers */
	u32 count[3];	/* Timer count registers */
	u32 status[3];
	u32 ie[3];
	u32 preload[3];	/* Timer preload value */
	u32 preload_ctrl[3];
	u32 wdt_match_en;
	u32 wdt_match_r;
	u32 wdt_val;
	u32 wdt_sts;
	u32 icr[3];
	u32 wdt_icr;
	u32 cer;	/* Timer count enable reg */
	u32 cmr;
	u32 ilr[3];
	u32 wcr;
	u32 wfar;
	u32 wsar;
	u32 cvwr;
};

#define TIMER			0	/* Use TIMER 0 */
/* Each timer has 3 match registers */
#define MATCH_CMP(x)		((3 * TIMER) + x)
#define TIMER_LOAD_VAL 		0xffffffff
#define	COUNT_RD_REQ		0x1

DECLARE_GLOBAL_DATA_PTR;
/* Using gd->arch.tbu from timestamp and gd->arch.tbl for lastdec */

/* For preventing risk of instability in reading counter value,
 * first set read request to register cvwr and then read same
 * register after it captures counter value.
 */
ulong read_timer(void)
{
	struct armd1tmr_registers *armd1timers =
		(struct armd1tmr_registers *) ARMD1_TIMER_BASE;
	volatile int loop=100;

	writel(COUNT_RD_REQ, &armd1timers->cvwr);
	while (loop--);
	return(readl(&armd1timers->cvwr));
}

static ulong get_timer_masked(void)
{
	ulong now = read_timer();

	if (now >= gd->arch.tbl) {
		/* normal mode */
		gd->arch.tbu += now - gd->arch.tbl;
	} else {
		/* we have an overflow ... */
		gd->arch.tbu += now + TIMER_LOAD_VAL - gd->arch.tbl;
	}
	gd->arch.tbl = now;

	return gd->arch.tbu;
}

ulong get_timer(ulong base)
{
	return ((get_timer_masked() / (CONFIG_SYS_HZ_CLOCK / 1000)) -
		base);
}

void __udelay(unsigned long usec)
{
	ulong delayticks;
	ulong endtime;

	delayticks = (usec * (CONFIG_SYS_HZ_CLOCK / 1000000));
	endtime = get_timer_masked() + delayticks;

	while (get_timer_masked() < endtime);
}

/*
 * init the Timer
 */
int timer_init(void)
{
	struct armd1apb1_registers *apb1clkres =
		(struct armd1apb1_registers *) ARMD1_APBC1_BASE;
	struct armd1tmr_registers *armd1timers =
		(struct armd1tmr_registers *) ARMD1_TIMER_BASE;

	/* Enable Timer clock at 3.25 MHZ */
	writel(APBC_APBCLK | APBC_FNCLK | APBC_FNCLKSEL(3), &apb1clkres->timers);

	/* load value into timer */
	writel(0x0, &armd1timers->clk_ctrl);
	/* Use Timer 0 Match Resiger 0 */
	writel(TIMER_LOAD_VAL, &armd1timers->match[MATCH_CMP(0)]);
	/* Preload value is 0 */
	writel(0x0, &armd1timers->preload[TIMER]);
	/* Enable match comparator 0 for Timer 0 */
	writel(0x1, &armd1timers->preload_ctrl[TIMER]);

	/* Enable timer 0 */
	writel(0x1, &armd1timers->cer);
	/* init the gd->arch.tbu and gd->arch.tbl value */
	gd->arch.tbl = read_timer();
	gd->arch.tbu = 0;

	return 0;
}

#define MPMU_APRR_WDTR	(1<<4)
#define TMR_WFAR	0xbaba	/* WDT Register First key */
#define TMP_WSAR	0xeb10	/* WDT Register Second key */

/*
 * This function uses internal Watchdog Timer
 * based reset mechanism.
 * Steps to write watchdog registers (protected access)
 * 1. Write key value to TMR_WFAR reg.
 * 2. Write key value to TMP_WSAR reg.
 * 3. Perform write operation.
 */
void reset_cpu (unsigned long ignored)
{
	struct armd1mpmu_registers *mpmu =
		(struct armd1mpmu_registers *) ARMD1_MPMU_BASE;
	struct armd1tmr_registers *armd1timers =
		(struct armd1tmr_registers *) ARMD1_TIMER_BASE;
	u32 val;

	/* negate hardware reset to the WDT after system reset */
	val = readl(&mpmu->aprr);
	val = val | MPMU_APRR_WDTR;
	writel(val, &mpmu->aprr);

	/* reset/enable WDT clock */
	writel(APBC_APBCLK | APBC_FNCLK | APBC_RST, &mpmu->wdtpcr);
	readl(&mpmu->wdtpcr);
	writel(APBC_APBCLK | APBC_FNCLK, &mpmu->wdtpcr);
	readl(&mpmu->wdtpcr);

	/* clear previous WDT status */
	writel(TMR_WFAR, &armd1timers->wfar);
	writel(TMP_WSAR, &armd1timers->wsar);
	writel(0, &armd1timers->wdt_sts);

	/* set match counter */
	writel(TMR_WFAR, &armd1timers->wfar);
	writel(TMP_WSAR, &armd1timers->wsar);
	writel(0xf, &armd1timers->wdt_match_r);

	/* enable WDT reset */
	writel(TMR_WFAR, &armd1timers->wfar);
	writel(TMP_WSAR, &armd1timers->wsar);
	writel(0x3, &armd1timers->wdt_match_en);

	while(1);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk (void)
{
	return (ulong)CONFIG_SYS_HZ;
}
