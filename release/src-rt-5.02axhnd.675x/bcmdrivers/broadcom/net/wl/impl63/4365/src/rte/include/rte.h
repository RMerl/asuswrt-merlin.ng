/*
 * HND RTE misc interfaces.
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
 * $Id: rte.h 696357 2017-04-26 07:10:19Z $
 */

#ifndef _rte_h_
#define _rte_h_

#include <typedefs.h>
#include <siutils.h>

#include <rte_trap.h>
#include <rte_timer.h>

#define RTE_WORD_SIZE 4 /* Word size in rte */

/* ========================== CPU ========================= */
/* Each CPU/Arch must implement this interface */
int32 hnd_cpu_clockratio(si_t *sih, uint8 div);

/* ========================== timer ========================== */
int hnd_schedule_work(void *ctx, void *data,
	hnd_timer_mainfn_t taskfn, int delay);
/* Each CPU/Arch must implement this interface - suppress any further timer requests */
void hnd_suspend_timer(void);
/* Each CPU/Arch must implement this interface - resume timers */
void hnd_resume_timer(void);

/* ========================== system ========================== */
void hnd_enable_interrupts(void);
void hnd_disable_interrupts(void);
si_t *hnd_init(void);
#ifdef ATE_BUILD
#define hnd_poll(sih) wl_ate_cmd_proc()
void wl_ate_cmd_proc(void);
#else /* !ATE_BUILD */
void hnd_poll(si_t *sih);
#endif // endif
/* Each OS implement this interface */
void hnd_idle(si_t *sih);

/* ======================= debug ===================== */
void hnd_memtrace_enab(bool on);

#ifdef	BCMDBG
#if defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
#define	BCMDBG_TRACE(x)		__watermark = (x)
#else
#define	BCMDBG_TRACE(x)
#endif	/* !__arm__ && !__thumb__ && !__thumb2__ */
#else
#define	BCMDBG_TRACE(x)
#endif	/* BCMDBG */

extern volatile uint __watermark;

/* ============================ misc =========================== */
void hnd_unimpl(void);

#ifdef RAMSIZE
extern uint _ramsize_adj;
#define RAMSIZE_ADJ	(_ramsize_adj)
#endif /* RAMSIZE */

#endif /* _rte_h_ */
