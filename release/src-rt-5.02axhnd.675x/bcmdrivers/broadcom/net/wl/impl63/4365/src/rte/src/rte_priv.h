/*
 * RTE private interfaces between different modules in RTE.
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
 * $Id: rte_priv.h 696357 2017-04-26 07:10:19Z $
 */

#ifndef _hnd_rte_priv_h_
#define _hnd_rte_priv_h_

#include <typedefs.h>
#include <siutils.h>
#include <osl_decl.h>

#include <rte_trap.h>

/* Forward declaration */
extern si_t *hnd_sih;		/* Chip backplane handle */
extern osl_t *hnd_osh;		/* Chip backplane osh */

extern uint32 c0counts_per_us;
extern uint32 c0counts_per_ms;

/* ========================== system ========================== */
void hnd_idle_init(si_t *sih);
/* Each CPU/Arch must implement this interface - idle loop */
void hnd_idle_loop(si_t *sih);
/* Each CPU/Arch must implement this interface - wait for interrupt */
void hnd_wait_irq(si_t *sih);

/* ======================= trap ======================= */
void hnd_trap_init(void);
void hnd_trap_common(trap_t *tr);
void hnd_print_stack(uint32 sp);

/* ================ CPU ================= */
void hnd_cpu_init(si_t *sih);

int hnd_cpu_stats_init(si_t *sih);
void hnd_cpu_stats_upd(uint32 start_time);
int hnd_cpu_deadman_init(si_t *sih);
uint32 hnd_cpu_gtimer_trap_validation(void);
void hnd_cpu_gtimer_fiq_hdl(void);

#define LOADAVG_HZ	100		/* Sample Hz */

extern uint32 loadavg_time;

void hnd_cpu_load_avg(uint epc, uint lr, uint sp);

/* ========================== time ========================== */
uint32 hnd_update_now(void);
uint32 hnd_update_now_us(void);

/* ========================== timer ========================== */
/* Each CPU/Arch must implement this interface - init possible global states */
int hnd_timer_init(si_t *sih);
/* Each CPU/Arch must implement this interface - program the h/w */
void hnd_set_irq_timer(uint ms);
/* Each CPU/Arch must implement this interface - ack h/w interrupt */
void hnd_ack_irq_timer(void);
/* Each CPU/Arch must implement this interface - run timeouts */
void hnd_run_timeouts(void);
/* Each CPU/Arch must implement this interface - register possible command line proc */
void hnd_timer_cli_init(void);

/* ======================= debug ===================== */
void hnd_stack_prot(void *stack_top);

/* ===================== debug ====================== */
void hnd_debug_info_init(void);

/* ======================= cache ===================== */
#ifdef RTE_CACHED
void hnd_caches_init(si_t *sih);
#endif // endif

/* accessor functions */
extern si_t* get_hnd_sih(void);
extern void set_hnd_sih(si_t *val);
#ifdef RAMSIZE
uint rte_ramsize(void);
#endif // endif
#endif /* _hnd_rte_priv_h_ */
