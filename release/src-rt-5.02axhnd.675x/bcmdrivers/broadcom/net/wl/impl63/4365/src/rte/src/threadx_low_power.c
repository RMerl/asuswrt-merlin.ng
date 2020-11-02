/** \file threadx.c
 *
 * Initialization and support routines for threadX.
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
 * $Id: threadx_low_power.c 480442 2014-05-23 20:23:30Z $
 */

/* These functions are implementations for the templates provided by threadX in tx_low_power.c.
 * The functions have been renamed to avoid symbol conflicts and are implemented in this source
 * file to avoid modifying the threadX source.
 *
 * ThreadX expects when the device goes into low power mode, threadx_low_power_enter() is
 * invoked so that the hardware interrupt is configured for the next earliest timer expiry.
 * When the device awakes, threadx_low_power_exit() is invoked to determine the elapsed time
 * and to advance the threadx timers and invoking any expired timers. Threadx_low_power_exit()
 * does nothing if the device was not in low power mode. While the device is not in low
 * power mode, threadX expects a periodic timer tick to invoke _tx_timer_interrupt() to
 * advance the timers.
 *
 * Modifications have been made to this module to suit the Broadcom device which does not
 * have a periodic timer tick. Threadx_low_power_enter() will program the PMU with the next
 * earliest timer expiry. Threadx_low_power_exit() will unconditionally (regardless of
 * power mode) determine the elapsed time and advance the threadx timers and invoke
 * _tx_timer_interrupt().
 */

#include <osl_ext.h>

#include <tx_api.h>
#include <tx_low_power.h>

#include "rte_priv.h"

#include <threadx_low_power.h>
#include "threadx_low_power_priv.h"

VOID _tx_timer_interrupt(VOID);

/* in tx_low_power.c */
extern ULONG   tx_low_power_last_timer;
extern ULONG   tx_low_power_next_expiration;
extern ULONG   tx_low_power_current_timer;
extern ULONG   tx_low_power_adjust_ticks;

static ULONG   threadx_total_ticks = 0;

#ifdef THREADX_LOW_POWER_TEST

#define LOG_DATA_SIZE	200
static int threadx_low_power_log_count = 0;
static int threadx_low_power_log[LOG_DATA_SIZE][4];

static void debug_log(int d0, int d1, int d2, int d3)
{
	if (threadx_total_ticks > 2200 &&
		threadx_low_power_log_count < LOG_DATA_SIZE) {
		int *log = &threadx_low_power_log[threadx_low_power_log_count][0];
		log[0] = d0;
		log[1] = d1;
		log[2] = d2;
		log[3] = d3;
		threadx_low_power_log_count++;
	}
}
#endif	/* THREADX_LOW_POWER_TEST */

/* enter low power mode */
void threadx_low_power_enter(void)
{
	TX_INTERRUPT_SAVE_AREA

	ULONG   any_expired;

	/* Disable interrupts while we prepare for low power mode.  */
	TX_DISABLE

	/* At this point, we want to enter low power mode, since nothing
	 * meaningful is going on in the system. However, in order to keep
	 * the ThreadX timer services accurate, we must first determine the
	 * next ThreadX timer expiration in terms of ticks. This is
	 * accomplished via the tx_timer_get_next API.
	 */
	any_expired =  tx_timer_get_next(&tx_low_power_next_expiration);

	/* Determine if any timers have expired. */
	if (any_expired)
	{

		/* Reprogram the internal timer source such that the next timer
		 * interrupt is equal to:  next_timer_expiration*tick_frequency.
		 * In most applications, the tick_frequency is 10ms, but this is
		 * completely application specific in ThreadX.
		 */

		/**** ADD Code to reprogram the timer here!  */

		/* tx_timer_get_next() returns less 1 tick */
		tx_low_power_next_expiration += 1;

		/* maximim 32-bits */
		if (tx_low_power_next_expiration > 0xffffffff) {
			hnd_set_irq_timer(0xffffffff);
		} else {
			uint32 time_since_update = threadx_low_power_time_since_update();
			uint32 time_next_expiry = OSL_TICKS_TO_MSEC(tx_low_power_next_expiration);
			uint32 time_next_interrupt = 0;

			if (time_next_expiry > time_since_update) {
				time_next_interrupt = time_next_expiry - time_since_update;
			}

#ifdef THREADX_LOW_POWER_TEST
			debug_log(0 | time_since_update << 8, tx_time_get(),
				hnd_time(), time_next_interrupt);
#endif	/* THREADX_LOW_POWER_TEST */

			/* set timer interrupt for next expiry */
			hnd_set_irq_timer(time_next_interrupt);
		}
	}
	else
	{

		/* Reprogram the internal timer to the maximum value possible,
		 * since there are no ThreadX timers pending.
		 */

		/**** ADD Code to reprogram the timer here!  */

		/* set timer interrupt for maximum timeout */
		hnd_set_irq_timer(0xffffffff);

	}

	/* Re-enable interrupts before low power mode is entered.  */
	TX_RESTORE
}

/* exit low power mode */
void threadx_low_power_exit(void)
{
	uint32 elapsed_time;

	/**** ADD Code to pickup the current timer value.  */
	tx_low_power_current_timer = hnd_time();

	/* ADD Code to determine how many timer ticks (interrupts) that
	 * the ThreadX time should be incremented to properly adjust
	 * for the time in low power mode. The result is assumed to be
	 * placed in tx_low_power_adjust_ticks.
	 */
	elapsed_time = tx_low_power_current_timer - tx_low_power_last_timer;
	tx_low_power_adjust_ticks = OSL_MSEC_TO_TICKS(elapsed_time);

	/* Determine if the ThreadX timer needs incrementing.  */
	if (tx_low_power_adjust_ticks)
	{
		tx_low_power_last_timer = tx_low_power_current_timer;
		threadx_total_ticks += tx_low_power_adjust_ticks;

		/* Yes, the ThreadX time must be incremented.
		 * Call tx_time_increment to accomplish this.
		 */

		/* advance time by ticks less 1 */
		tx_time_increment(tx_low_power_adjust_ticks - 1);

#ifdef THREADX_LOW_POWER_TEST
		debug_log(1, tx_time_get(),
			tx_low_power_current_timer, tx_low_power_adjust_ticks);
#endif	/* THREADX_LOW_POWER_TEST */

		/* advance time by 1 tick and invoke expired timer callbacks */
		_tx_timer_interrupt();
	}
}

/* initialize low power mode */
void threadx_low_power_init(void)
{
	/* initialize last timer */
	tx_low_power_last_timer = hnd_time();
}

/* msec since last time update */
uint32 threadx_low_power_time_since_update(void)
{
	osl_ext_interrupt_state_t state = osl_ext_interrupt_disable();
	uint32 msec = hnd_time() - tx_low_power_last_timer;
	osl_ext_interrupt_restore(state);
	return msec;
}

/* force PMU h/w timer update */
void threadx_low_power_timer_update(void)
{
	threadx_low_power_enter();
}
