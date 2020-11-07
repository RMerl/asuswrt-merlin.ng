/** \file hndarm_gt.c
 *
 * ARM Generic Timer (GT) support routines.
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
 * $Id: hndarm_gt.c 771714 2019-02-06 10:43:47Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <hndarm_gt.h>

#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)

/* ARM GT CNT*_CTL register fields. */
#define GT_IENABLE	0x1
#define GT_IMASK	0x2
#define GT_ISTATUS	0x4

/**
 * Initialize ARM Generic Timers.
 *
 * Configure the GT to provide a monotonically increasing 64-bit counter and two count-down
 * timers that can trigger interrupts when reaching zero.
 *
 * The EL1 physical timer interrupt ID is 29.
 * The EL1 virtual timer interrupt ID is 27.
 */

void
hnd_arm_gt_init(si_t *sih)
{
	/* Use defaults */
}

/**
 * Get the current value of the 64-bit monotonically increasing counter.
 *
 * @todo Need to handle counter overflow after 584942 years.
 *
 * @return		Counter value.
 */

uint64
hnd_arm_gt_get_count(void)
{
	uint64 count;

	asm volatile("isb");
	asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (count));			// CNTPCT

	return count;
}

/**
 * Set the value of a count-down timer.
 *
 * An interrupt will be triggered when the counter reaches zero. Setting the value to zero will
 * generate an immediate interrupt. The interrupt is level-sensitive and needs to be acknowledged
 * (@see hnd_arm_gt_ack_timer) in the interrupt handler.
 *
 * @param timer		Timer identifier.
 * @param count		Timeout in ticks.
 */

void
hnd_arm_gt_set_timer(arm_gt_timer_t timer, uint count)
{
	if (timer == ARM_GT_VIRTUAL_TIMER) {
		asm volatile("mcr p15, 0, %0, c14, c3, 0" : : "r" (count));		// CNTV_TVAL
		asm volatile("mcr p15, 0, %0, c14, c3, 1" : : "r" (GT_IENABLE));	// CNTV_CTL
		asm volatile("isb");
	} else {
		asm volatile("mcr p15, 0, %0, c14, c2, 0" : : "r" (count));		// CNTP_TVAL
		asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r" (GT_IENABLE));	// CNTP_CTL
		asm volatile("isb");
	}
}

/**
 * Acknowledge or disable a count-down timer.
 *
 * @param timer		Timer identifier.
 */

void
hnd_arm_gt_ack_timer(arm_gt_timer_t timer)
{
	if (timer == ARM_GT_VIRTUAL_TIMER) {
		asm volatile("mcr p15, 0, %0, c14, c3, 1" : : "r" (0));			// CNTV_CTL
		asm volatile("mcr p15, 0, %0, c14, c3, 0" : : "r" (0));			// CNTV_TVAL
		asm volatile("isb");
	} else {
		asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r" (0));			// CNTP_CTL
		asm volatile("mcr p15, 0, %0, c14, c2, 0" : : "r" (0));			// CNTP_TVAL
		asm volatile("isb");
	}
}

/**
 * Get the value (remaining ticks) of a count-down timer.
 *
 * @param timer		Timer identifier.
 * @return		Counter value in ticks.
 */

uint32
hnd_arm_gt_get_timer(arm_gt_timer_t timer)
{
	uint32 count;

	if (timer == ARM_GT_VIRTUAL_TIMER) {
		asm volatile("isb");
		asm volatile("mrc p15, 0, %0, c14, c3, 0" : "=r" (count));		// CNTV_TVAL
	} else {
		asm volatile("isb");
		asm volatile("mrc p15, 0, %0, c14, c2, 0" : "=r" (count));		// CNTP_TVAL
	}
	return count;
}

/**
 * Get the interrupt status of a count-down timer.
 *
 * @param timer		Timer identifier.
 * @return		Nonzero if an interrupt is pending.
 */

uint32
hnd_arm_gt_get_status(arm_gt_timer_t timer)
{
	uint32 status;

	if (timer == ARM_GT_VIRTUAL_TIMER) {
		asm volatile("isb");
		asm volatile("mrc p15, 0, %0, c14, c3, 1" : "=r" (status));		// CNTV_CTL
	} else {
		asm volatile("isb");
		asm volatile("mrc p15, 0, %0, c14, c2, 1" : "=r" (status));		// CNTP_CTL
	}
	return (status & GT_ISTATUS) != 0;
}

#endif /* __ARM_ARCH_7R__ || __ARM_ARCH_7A__ */
