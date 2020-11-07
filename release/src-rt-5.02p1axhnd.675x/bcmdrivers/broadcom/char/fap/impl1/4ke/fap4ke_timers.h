#ifndef __FAPTMR_H_INCLUDED__
#define __FAPTMR_H_INCLUDED__

/*
<:copyright-BRCM:2009:DUAL/GPL:standard

   Copyright (c) 2009 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/*
 *******************************************************************************
 * File Name  : fapMips_timers.h
 *
 * Description: This file contains global definitions of the Timers
 *              implementation for the BCM63268 FAP.
 *
 *******************************************************************************
 */

#include "fap4ke_task.h"

#define CC_FAP4KE_TIMER_HIRES

/* System timer tick rate */
#define FAPTMR_HZ_LORES 10    /* 100ms */
#define FAPTMR_HZ_HIRES 5000  /* 200us */

#define FAP4KE_TIMER_JIFFIES_32(_jiffies64) ( (uint32)(_jiffies64) )

#define fap4keTmr_jiffiesLoRes FAP4KE_TIMER_JIFFIES_32(p4keDspramGbl->timers.ctrl.loRes.jiffies64)
#define fap4keTmr_jiffiesHiRes FAP4KE_TIMER_JIFFIES_32(p4keDspramGbl->timers.ctrl.hiRes.jiffies64)

#define FAP4KE_TIMER_INIT(_timer, _handler, _arg, _taskPriority)        \
    do {                                                                \
        (_timer)->taskPriority = (_taskPriority);                       \
        FAP4KE_TASK_INIT(&(_timer)->task, (_handler), (_arg));          \
    } while(0)

/* The following 4 macros are provided for comparing tick counts that correctly handle
   wraparound in the tick count. The _unknown parameter is typically fap4keTmr_Jiffies,
   and the _known parameter is the value against which you want to compare */

/* if _unknown is after _known, true; otherwise false */
#define fap4keTmr_isTimeAfter(_unknown, _known) ( (int32)(_known) - (int32)(_unknown) < 0 )

/* if _unknown is before _known, true; otherwise false */
#define fap4keTmr_isTimeBefore(_unknown, _known) fap4keTmr_isTimeAfter(_known, _unknown)

/* if _unknown is after than or equal to _known, true; otherwise false */
#define fap4keTmr_isTimeAfter_eq(_unknown, _known) ( (int32)(_unknown) - (int32)(_known) >= 0 )

/* if _unknown is before than or equal to _known, true; otherwise false */
#define fap4keTmr_isTimeBefore_eq(_unknown, _known) fap4keTmr_isTimeAfter_eq(_known, _unknown)

typedef struct {
    /* Timer Management */
    volatile int64 jiffies64;
    Dll_t list;
} fap4keTmr_CtrlInfo_t;

typedef struct {
    /* Timer Control Information */
    fap4keTmr_CtrlInfo_t loRes;
    fap4keTmr_CtrlInfo_t hiRes;
} fap4keTmr_Ctrl_t;

typedef struct {
    Dll_t node;            /* used internally to maintain linked-list of timers */
    uint32 expiration;     /* expiration time, in fap4keTmr_Jiffies */
    fap4keTsk_taskPriority_t taskPriority; /* timer task priority */
    fap4keTsk_task_t task; /* the task in which the timer handler will run */
} fap4keTmr_timer_t;

typedef enum {
    FAP4KE_TIMER_TYPE_LORES=0,
    FAP4KE_TIMER_TYPE_HIRES,
    FAP4KE_TIMER_TYPE_MAX
} fap4keTmr_type_t;

fapRet fap4keTmr_add(fap4keTmr_timer_t *timer, fap4keTmr_type_t type);
void fap4keTmr_Init(void);

#define FAP4KE_PM_CPU_HISTORY_MAX 8

typedef struct {
    uint32 cp0Count;
    uint32 busy;
    uint32 capture;
} fap4keTmr_cpuSample_t;

#define p4keCpuSample ( (&p4keDspramGbl->timers.cpu) )

typedef struct {
    uint32 index;
    uint32 busy[FAP4KE_PM_CPU_HISTORY_MAX];
} fap4keTmr_cpuHistory_t;

#define p4keCpuHistory ( (&p4kePsmGbl->timers.cpu) )

#define pHostCpuHistory(fapIdx) ( (&pHostPsmGbl(fapIdx)->timers.cpu) )

#endif  /* defined(__FAPTMR_H_INCLUDED__) */
