/** @file rte_pmu.c
 *
 * RTE support for PMU
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: rte_pmu.c 591083 2015-10-07 02:46:20Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <osl_ext.h>
#include <sbchipc.h>
#include <hndpmu.h>
#include <hndsoc.h>
#include <rte_chipc.h>
#include "rte_chipc_priv.h"
#include "rte_pmu_priv.h"
#include <rte_isr.h>
#include "rte_priv.h"

/* selects core based on AOB_ENAB() */
#define PMUREGADDR(sih, pmur, ccr, member) \
	(AOB_ENAB(sih) ? (&(pmur)->member) : (&(ccr)->member))

/* prevents backplane stall caused by subsequent writes to 'ilp domain' PMU registers */
#define HND_PMU_SYNC_WR(sih, pmur, ccr, osh, r, v) do { \
	if ((sih) && (sih)->pmurev >= 22) { \
		while (R_REG(osh, PMUREGADDR(sih, pmur, ccr, pmustatus)) & \
		       PST_SLOW_WR_PENDING) { \
			; /* empty */ \
		} \
	} \
	W_REG(osh, r, v); \
	(void)R_REG(osh, r); \
} while (0)

#ifndef RTE_POLL
static void hnd_pmu_isr(void *cbdata);
#ifdef THREAD_SUPPORT
static void hnd_pmu_dpc(void *cbdata);
#endif // endif
static void hnd_pmu_cc_isr(void* cbdata, uint32 ccintst);
#ifdef THREAD_SUPPORT
static void hnd_pmu_cc_dpc(void* cbdata, uint32 ccintst);
#endif // endif
static void hnd_pmu_run(void *cbdata, uint32 intst);
#endif /* !RTE_POLL */

static pmuregs_t *hnd_pmur = NULL;	/* PMU core regs */

/*
 * ticks per msec - used by hnd_update_now() and is based on either 80Mhz
 * clock or 32Khz clock depending on the compile-time decision.
 */
/* ILP clock speed default to 32KHz */

#define PMUTICK_CALC_COUNT_SHIFT 4	/* 1<<4 times around loop) */

/* ms_per_pmutick is scaled (shifted) to improve accuracy */
#define MS_PER_PMUTICK_DEFAULT_SCALE 32
static uint32 ms_per_pmutick =	((uint64) 1000 << MS_PER_PMUTICK_DEFAULT_SCALE) / ILP_CLOCK;
static uint32 ms_per_pmutick_scale = MS_PER_PMUTICK_DEFAULT_SCALE;

/* pmuticks_per_ms is now scaled (shifted) to improve accuracy */
#define PMUTICKS_PER_MS_SCALE_DEFAULT PMUTICK_CALC_COUNT_SHIFT
static uint32 pmuticks_per_ms = (ILP_CLOCK << PMUTICKS_PER_MS_SCALE_DEFAULT) / 1000;
static uint32 pmuticks_per_ms_scale = PMUTICKS_PER_MS_SCALE_DEFAULT;

/* PmuRev0 has a 10-bit PMU RsrcReq timer which can last 31.x msec
 * at 32KHz clock. To work around this limitation we chop larger timer to
 * multiple maximum 31 msec timers. When these 31 msec timers expire the ISR
 * will be running at 32KHz to save power.
 */
static uint max_timer_dur = (1 << 10) - 1;	/* Now in ticks!! */

void
BCMATTACHFN(hnd_pmu_init)(si_t *sih)
{
	uint32 pmutimer, startcycles, cycles, rem;
	osl_t *osh = si_osh(sih);
	int i;
	uint32 ticks, ticks_high, ticks_low;
	uint64 ticks64;

	/* si_pmu_init() is called in si_attach() */

	/* get pmu core reg space addr */
	if (AOB_ENAB(sih)) {
		uint coreidx = si_coreidx(sih);

		hnd_pmur = si_setcore(sih, PMU_CORE_ID, 0);
		ASSERT(hnd_pmur != NULL);

		/* Restore to CC */
		si_setcoreidx(sih, coreidx);
	}
	/* pmu is a subcore in chipc hence use chipc reg space addr */
	else {
		ASSERT(hnd_ccr != NULL);
	}

	/*
	 * Compute the pmu ticks per ms.  This is done by counting a
	 * few PMU timer transitions and using the ARM cyclecounter as
	 * a more accurate clock to measure the PMU tick interval.
	 */

	/* Loop until we see a change */
	pmutimer = R_REG(osh, PMUREGADDR(sih, hnd_pmur, hnd_ccr, pmutimer));
	while (pmutimer == R_REG(osh, PMUREGADDR(sih, hnd_pmur, hnd_ccr, pmutimer))) {
		; /* empty */
	}
	/* There is a clock boundary crosssing so do one more read */
	pmutimer = R_REG(osh, PMUREGADDR(sih, hnd_pmur, hnd_ccr, pmutimer));

	/* The PMU timer just changed - start the cyclecount timer */
	OSL_GETCYCLES(startcycles);

	for (i = 0; i < (1 << PMUTICK_CALC_COUNT_SHIFT); i++) {
		while (pmutimer == R_REG(osh, PMUREGADDR(sih, hnd_pmur, hnd_ccr, pmutimer))) {
			/* empty */
		}
		pmutimer = R_REG(osh, PMUREGADDR(sih, hnd_pmur, hnd_ccr, pmutimer));
	}

	OSL_GETCYCLES(cycles);
	cycles -= startcycles;
	/*
	 * Calculate the pmuticks_per_ms with scaling for greater
	 * accuracy.  We scale by the amount needed to shift the
	 * c0counts_per_ms so the leading bit is set.  Since the
	 * divisor (cycles counted) is implicity shifted by
	 * PMUTICK_CALC_COUNT_SHIFT so that reduces the scale.
	 *
	 * We round up because we want the first tick AFTER the
	 * requested ms - otherwise we will get an extraneuous
	 * interrupt one tick early.
	 */

	pmuticks_per_ms_scale = CLZ(c0counts_per_ms) - PMUTICK_CALC_COUNT_SHIFT;
	pmuticks_per_ms =  ((c0counts_per_ms << CLZ(c0counts_per_ms)) / cycles);
	pmuticks_per_ms++;		/* Round up */

	/* Calculate the PMU clock frequency and set it */
	ticks64 = ((uint64) 1000) * pmuticks_per_ms;	/* ticks per sec */
	ticks_high = ticks64 >> 32;
	ticks_low = (uint32) ticks64;
	ticks = ticks_low >> pmuticks_per_ms_scale;
	ticks += ticks_high << (32 - pmuticks_per_ms_scale);

	si_pmu_ilp_clock_set(ticks);	/* Set */

	/*
	 * Do long-division to get a value that is the
	 * ms_per_pmutick scaled to have 31 bits of accuracy.
	 * Stopping one bit short (i.e., not using 32 bits of
	 * accuracy) leaves a spare bit to handle overflows during
	 * certain 32-bit math operations below.  Since we know that
	 * the pmuticks happen more often than once per ms we know
	 * that the scale will be >32.  This fact is used in other
	 * calculations.
	 */

	rem = cycles;			/* Initial numerator */
	ms_per_pmutick_scale = PMUTICK_CALC_COUNT_SHIFT;
	ms_per_pmutick = 0;

	while ((ms_per_pmutick & 0xc0000000) == 0) {
		uint32 partial, lz;
		/* Scale the remaining dividend */
		lz = MIN(CLZ(rem), CLZ(ms_per_pmutick) - 1);
		ms_per_pmutick <<= lz;
		rem <<= lz;
		ms_per_pmutick_scale += lz;

		partial = rem / c0counts_per_ms;
		ms_per_pmutick += partial;
		rem -= partial * c0counts_per_ms;
	}

	if (sih->pmurev >= 1) {
		max_timer_dur = ((1 << 24) - 1);
	} else {
		max_timer_dur = ((1 << 10) - 1);
	}

#ifndef RTE_POLL
	/* Register the timer interrupt handler */
	if (AOB_ENAB(sih) && (sih->pmurev <= 28)) {
		hnd_isr_register(0, PMU_CORE_ID, 0, hnd_pmu_isr, NULL, SI_BUS);
#ifdef THREAD_SUPPORT
		hnd_dpc_register(0, PMU_CORE_ID, 0, hnd_pmu_dpc, NULL, SI_BUS);
#endif /* THREAD_SUPPORT */
	} else {
		si_cc_register_isr(sih, hnd_pmu_cc_isr, CI_PMU, NULL);
#ifdef THREAD_SUPPORT
		si_cc_register_dpc(sih, hnd_pmu_cc_dpc, CI_PMU, NULL);
#endif /* THREAD_SUPPORT */
	}
#endif	/* RTE_POLL */
}

/* PmuRev1 has a 24-bit PMU RsrcReq timer. However it pushes all other bits
 * upward. To make the code to run for all revs we use a variable to tell how
 * many bits we need to shift.
 */

#define flags_shift	14

static uint64 cur_ticks = 0;
static uint64 ms_remainder = 0;

/* accumulate time delta between two PMU timer reads.
 * used in hnd_update_now()
 */
uint32
hnd_pmu_accu_tick(uint32 diff)
{
	uint32 ms, rem_high, rem_low;

	cur_ticks += diff;

	/* Calculate the ms and the remainder */
	ms_remainder += (uint64) diff * ms_per_pmutick;

	/*
	 * We want to calculate ms_remainder >> ms_per_pmutick_scale
	 * but that would be a 64-bit op and the compiler would
	 * generate a call to a 64-bit shift library routine which we
	 * do not support.  So we do the shift in 32-bit pices.  Note
	 * that we take advantage of knowing that the scale is > 32.
	 */
	rem_low = (uint32) ms_remainder;
	rem_high = ms_remainder >> 32;
	ms = rem_high >> (ms_per_pmutick_scale - 32);
	rem_high &= (1 << (ms_per_pmutick_scale - 32)) - 1;

	ms_remainder = ((uint64) rem_high << 32) | rem_low;

	return ms;
}

/*
 * The number of ticks is limited by max_timer_dur so we break up
 * requests that are too large and keep track of the extra.
 */
static uint32 timer_extra_ticks = 0;
static uint64 timer_extra_ticks_end;

#ifdef THREAD_SUPPORT
static void hnd_pmu_set_timer_ex(uint32 ticks);

void
hnd_pmu_set_timer(uint32 ticks)
{
	osl_ext_interrupt_state_t state = osl_ext_interrupt_disable();
	hnd_pmu_set_timer_ex(ticks);
	osl_ext_interrupt_restore(state);
}
#endif // endif

#ifdef THREAD_SUPPORT
static void
hnd_pmu_set_timer_ex(uint32 ticks)
#else
void
hnd_pmu_set_timer(uint32 ticks)
#endif // endif
{
	uint32 req;

	if (ticks > max_timer_dur) {
		/* don't req HT if we are breaking a large timer to multiple max h/w duration */
		timer_extra_ticks = ticks - max_timer_dur;
		timer_extra_ticks_end = cur_ticks + ticks;
		ticks = max_timer_dur;
		req = (PRRT_ALP_REQ | PRRT_INTEN) << flags_shift;
	} else {
		req = (PRRT_HT_REQ | PRRT_INTEN) << flags_shift;
		timer_extra_ticks = 0;

		if (hnd_sih->pmurev >= 15) {
			req |= PRRT_HQ_REQ << flags_shift;
		}
	}

	if (ticks == 0) {
		ticks = 2;		/* Need a 1->0 transition */
	}

	if (hnd_sih->pmurev >= 26) {
		HND_PMU_SYNC_WR(hnd_sih, hnd_pmur, hnd_ccr, hnd_osh,
		                PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, res_req_timer),
		                req | ticks |
		                ((PRRT_ALP_REQ | PRRT_HT_REQ | PRRT_HQ_REQ) << flags_shift));
	} else {
		HND_PMU_SYNC_WR(hnd_sih, hnd_pmur, hnd_ccr, hnd_osh,
		                PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, res_req_timer),
		                req | ticks);
	}

	if (hnd_sih->pmurev >= 26) {
		W_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmuintmask0),
		      RSRC_INTR_MASK_TIMER_INT_0);
		(void)R_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmuintmask0));
	}
}

/* convert msec to closest h/w ticks */
uint32
hnd_pmu_ms2tick(uint32 ms)
{
	uint32 ticks, ticks_high, ticks_low;
	uint64 ticks64;

	/*
	 *  Convert the ms to ticks rounding up so that we take the
	 *  first rupt AFTER the ms has expired.
	 *
	 *  Use 64-bit math with scaling to preserve accurracy.  This
	 *  is just "tick = ticks64  >> pmuticks_per_ms_scale" but
	 *  using only 32 bit shift operations.
	 */
	ticks64 = ((uint64) ms) * pmuticks_per_ms;
	ticks_high = ticks64 >> 32;
	ticks_low = (uint32) ticks64;
	ticks = ticks_low >> pmuticks_per_ms_scale;
	ticks += ticks_high << (32 - pmuticks_per_ms_scale);

	return ticks;
}

uint32
hnd_pmu_get_tick(void)
{
	uint32 ticks;

	/* PR88659: pmutimer is updated on ILP clock asynchronous to the CPU read.  Its
	 * value may change DURING the read, so the read must be verified and retried (but
	 * not in a loop, in case CPU is running at ILP).
	 */
	if ((hnd_sih == NULL) || (hnd_ccr == NULL))
		return 0;

	ticks = R_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmutimer));
	if (ticks != R_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmutimer)))
		ticks = R_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmutimer));

	return ticks;
}

void
hnd_pmu_ack_timer(void)
{
	HND_PMU_SYNC_WR(hnd_sih, hnd_pmur, hnd_ccr, hnd_osh,
	                PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, res_req_timer), 0);
}

#ifndef RTE_POLL
static void
hnd_pmu_isr(void *cbdata)
{
#ifdef THREAD_SUPPORT
	/* deassert interrupt */
	if (hnd_sih->pmurev >= 26) {
		W_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmuintstatus),
			RSRC_INTR_MASK_TIMER_INT_0);
		(void)R_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmuintstatus));
	} else
		W_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmustatus), PST_INTPEND);
#else
	hnd_pmu_run(cbdata, 0);
#endif // endif
}

#ifdef THREAD_SUPPORT
static void
hnd_pmu_dpc(void *cbdata)
{
	hnd_pmu_run(cbdata, 0);
}
#endif // endif

#ifdef THREAD_SUPPORT
static int pmu_count[2];
#endif // endif

static void
hnd_pmu_cc_isr(void* cbdata, uint32 ccintst)
{
#ifdef THREAD_SUPPORT
	/* deassert interrupt */
	pmu_count[0]++;
	W_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmustatus), PST_INTPEND);
	(void)R_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmustatus));
#else
	hnd_pmu_run(cbdata, 0);
#endif // endif
}

#ifdef THREAD_SUPPORT
static void
hnd_pmu_cc_dpc(void* cbdata, uint32 ccintst)
{
	pmu_count[1]++;
	hnd_pmu_run(cbdata, 0);
}
#endif // endif

static void hnd_pmu_upd_timer(void);

static void
hnd_pmu_run(void *cbdata, uint32 ccintst)
{
	/* Handle pmu timer interrupt here */

	/* Clear the pmustatus.intpend bit */
	if (hnd_sih->pmurev >= 26) {
		W_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmuintstatus),
		      RSRC_INTR_MASK_TIMER_INT_0);
		(void)R_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmuintstatus));
	} else {
		W_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmustatus), PST_INTPEND);
		(void)R_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmustatus));
	}

	if (AOB_ENAB(hnd_sih)) {
		/* Clear resource req timer bit */
		W_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmuintstatus), PMUREQTIMER);
		(void)R_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, pmuintstatus));
	}

	if ((R_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, res_req_timer)) &
	     (PRRT_REQ_ACTIVE << flags_shift))) {
#ifndef BCM_OL_DEV
		hnd_ack_irq_timer();
#endif /* BCM_OL_DEV */
		hnd_pmu_upd_timer();
	}
}

static void
hnd_pmu_upd_timer(void)
{
	if (timer_extra_ticks) {
		/* Part way through a long timer */
		hnd_update_now();

		if (timer_extra_ticks_end > cur_ticks) {
			hnd_pmu_set_timer(timer_extra_ticks_end - cur_ticks);
		} else {
			/* Need a 0-tick timer to kick start HT clock */
			hnd_pmu_set_timer(0);
		}
	}
}
#endif	/* !RTE_POLL */

void
hnd_pmu_set_gtimer_period()
{
	uint32 alp = 0x10199;

	if (hnd_sih->pmurev >= 31)
		alp = 0x80333330;

	/* Configure ALP period, 0x199 = 16384/40 for using 40MHz crystal */
	W_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, slowclkperiod), alp);

	/* Configure ILP period, 0xcccc = 65536/1.25 for using 1.25MHz crystal */
	W_REG(hnd_osh, PMUREGADDR(hnd_sih, hnd_pmur, hnd_ccr, ilpperiod), 0x10000);
}

/* ========================== misc =========================== */
#if  defined(WLSRVSDB)
uint32 hnd_clk_count(void);
uint32
hnd_clk_count(void)
{
	return hnd_pmu_get_tick();
}
#endif /* WLSRVSDB */
