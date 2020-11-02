/*
 * Threadx application support.
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
 * $Id: threadx_priv.h 597803 2015-11-06 08:46:00Z $
 */

#ifndef	_THREADX_PRIV_H
#define	_THREADX_PRIV_H

#include <typedefs.h>

/* These trap structure are private between the CPU/ThreadX and the ISR
 * routine threadx_isr i.e. they are determined by how CPU/ThreadX saves
 * thread context (either by CPU h/w or by ThreadX s/w).
 */
#ifdef __ARM_ARCH_7M__
typedef struct {
	uint32	IPSR;
	uint32	r10;
	uint32	lr_s;	/* same as lr but pushed by s/w */
	uint32	r0;	/* (Hardware stack starts here!!) */
	uint32	r1;
	uint32	r2;
	uint32	r3;
	uint32	r12;
	uint32	lr;
	uint32	pc;
	uint32	xPSR;
} threadx_trap_t;
#elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
typedef struct {
	uint32	SPSR;
	uint32	r10;
	uint32	ip;
	uint32	lr;
	uint32	r0;
	uint32	r1;
	uint32	r2;
	uint32	r3;
} threadx_trap_t;
#else /* Unsupported CPU architecture */
#error unsupported CPU architecture
#endif /* Unsupported CPU architecture */

/* top level interrupt handler */
void threadx_isr(threadx_trap_t *tr);
#ifndef BCMDBG_LOADAVG
void threadx_fiq_isr(void);
#endif // endif

#endif	/* _THREADX_PRIV_H */
