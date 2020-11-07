/*
<:copyright-BRCM:2018:proprietary:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:> 
*/

/*
*******************************************************************************
* File Name  : archer_thread.h
*
* Description: This file contains the Archer Thread Implementation
*
*******************************************************************************
*/

#ifndef __ARCHER_THREAD_H__
#define __ARCHER_THREAD_H__

#include <linux/brcm_dll.h>

typedef enum {
    ARCHER_THREAD_ID_US,
    ARCHER_THREAD_ID_MAX
} archer_thread_id_t;

typedef enum {
    ARCHER_TASK_PRIORITY_HIGH=0,
    ARCHER_TASK_PRIORITY_MEDIUM,
    ARCHER_TASK_PRIORITY_LOW,
    ARCHER_TASK_PRIORITY_MAX
} archer_task_priority_t;

typedef int(*archer_task_handler_t)(void *arg_p);

typedef struct {
    Dll_t node;
    void *thread_p;
    archer_task_handler_t handler;
    void *arg_p;
    char *name;
    int ref_count;
} archer_task_t;

#define ARCHER_TASK_INIT(_task, _thread_id, _handler, _arg_p)    \
    do {                                                         \
        (_task)->thread_p = archer_thread_get(_thread_id);       \
        (_task)->handler = (_handler);                           \
        (_task)->arg_p = (_arg_p);                               \
        (_task)->name = #_handler;                               \
        (_task)->ref_count = 0;                                  \
    } while(0)

int archer_task_schedule(archer_task_t *task,
                         archer_task_priority_t priority);

int archer_task_unschedule(archer_task_t *task);

static inline int archer_task_ref_count(archer_task_t *task)
{
    return task->ref_count;
}

int archer_task_loop(int *work_avail_p);

void __init archer_task_construct(void);

void *archer_thread_get(archer_thread_id_t thread_id);

void archer_thread_wakeup(archer_task_t *task);

#endif  /* __ARCHER_THREAD_H__ */
