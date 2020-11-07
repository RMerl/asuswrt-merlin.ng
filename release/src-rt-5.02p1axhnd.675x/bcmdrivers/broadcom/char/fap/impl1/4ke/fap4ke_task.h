#ifndef __FAP4KE_TASK_H_INCLUDED__
#define __FAP4KE_TASK_H_INCLUDED__

/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom 
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
 * File Name  : fap4ke_task.h
 *
 * Description: This file contains the Task Manager Definitions.
 *
 *******************************************************************************
 */


/*******************************************************************
 * Public API
 *******************************************************************/

#include "fap_task.h"

#define FAP4KE_TASK_INIT(_task, _taskHandler, _arg)     \
    do {                                                \
        (_task)->handler = (_taskHandler);              \
        (_task)->arg = (_arg);                          \
        (_task)->name = #_taskHandler;                  \
        (_task)->refCount = 0;                          \
    } while(0)

void fap4keTsk_init(void);

void fap4keTsk_loop(void);

char *fap4keTsk_currTaskName(void);

/* IMPORTANT: tasks may only be scheduled in interrupt context! */
fapRet fap4keTsk_schedule(fap4keTsk_taskPriority_t taskPriority,
                          fap4keTsk_task_t *task);

fapRet fap4keTsk_unschedule(fap4keTsk_task_t *task);

/*******************************************************************
 * Private
 *******************************************************************/

typedef struct {
    Dll_t list;
    uint32 count;
} fap4keTsk_workQueue_t;

typedef struct {
    fap4keTsk_workQueue_t workQueue[FAP4KE_TASK_PRIORITY_MAX];
    volatile int32 count;
    char *currTaskName;
} fap4keTsk_scheduler_t;

#endif  /* defined(__FAP4KE_TASK_H_INCLUDED__) */
