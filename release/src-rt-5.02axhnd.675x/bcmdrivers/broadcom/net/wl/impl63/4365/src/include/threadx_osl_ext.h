/*
 * Threadx OS Support Extension Layer
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
 * $Id:$
 */

#ifndef _threadx_osl_ext_h_
#define _threadx_osl_ext_h_

#ifdef __cplusplus
extern "C" {
#endif // endif

/* ---- Include Files ---------------------------------------------------- */

#include <tx_api.h>
#include <typedefs.h>

/* ---- Constants and Types ---------------------------------------------- */

/* This is really platform specific and not OS specific. */
#ifndef BWL_THREADX_TICKS_PER_SECOND
#define BWL_THREADX_TICKS_PER_SECOND	1000
#endif // endif

#define OSL_MSEC_TO_TICKS(msec)  ((BWL_THREADX_TICKS_PER_SECOND * (msec)) / 1000)
#define OSL_TICKS_TO_MSEC(ticks) ((1000 * (ticks)) / BWL_THREADX_TICKS_PER_SECOND)

/* Semaphore. */
typedef TX_SEMAPHORE osl_ext_sem_t;
#define OSL_EXT_SEM_DECL(sem)		osl_ext_sem_t  sem;

/* Mutex. */
typedef TX_MUTEX osl_ext_mutex_t;
#define OSL_EXT_MUTEX_DECL(mutex)	osl_ext_mutex_t  mutex;

/* Timer. */
typedef TX_TIMER osl_ext_timer_t;
#define OSL_EXT_TIMER_DECL(timer)	osl_ext_timer_t  timer;

/* Task. */
typedef TX_THREAD osl_ext_task_t;
#define OSL_EXT_TASK_DECL(task)		osl_ext_task_t  task;

/* Queue. */
typedef TX_QUEUE osl_ext_queue_t;
#define OSL_EXT_QUEUE_DECL(queue)	osl_ext_queue_t  queue;

/* Event. */
typedef TX_EVENT_FLAGS_GROUP osl_ext_event_t;
#define OSL_EXT_EVENT_DECL(event)	osl_ext_event_t  event;

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#ifdef __cplusplus
	}
#endif // endif

#endif  /* _threadx_osl_ext_h_  */
