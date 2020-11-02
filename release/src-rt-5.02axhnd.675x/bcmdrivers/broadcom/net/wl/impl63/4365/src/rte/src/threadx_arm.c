/*
 * ThreadX ARM port specific routines.
 * Contains functions that are specific to both ARM processor *and* RTE RTOS.
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
 * $Id: threadx_arm.c 591227 2015-10-07 09:01:53Z $
 */

#include <typedefs.h>
#include <hnd_debug.h>
#include <hnd_trap.h>
#include <rte_assert.h>
#include "rte_priv.h"
#include <bcmstdlib.h>
#include <bcmstdlib_ext.h>

/**
 * this handler is called by low level assembly code. It should withstand reentrancy. CM3 nor CR4
 * make use of the FVIC controller in the ARM subsystem. An implication is that a 'normal' interrupt
 * can be preempted by a 'fast interrupt', but not by another 'normal' interrupt.
 */
void
hnd_trap_handler(trap_t *tr)
{
#ifdef HND_PRINTF_THREAD_SAFE
	in_trap_handler ++;
#endif	/* HND_PRINTF_THREAD_SAFE */

	/* Save the trap pointer */
	get_hnd_debug_info()->trap_ptr = tr;

	/* Common trap handling */
	hnd_trap_common(tr);

	/* Halt processing without forcing a trap since we are already in the trap handler. */
	if ((get_g_assert_type() == 0) || (get_g_assert_type() == 2)) {
		hnd_die_no_trap();
	}

#ifdef HND_PRINTF_THREAD_SAFE
	in_trap_handler --;
#endif	/* HND_PRINTF_THREAD_SAFE */
} /* hnd_trap_handler */
