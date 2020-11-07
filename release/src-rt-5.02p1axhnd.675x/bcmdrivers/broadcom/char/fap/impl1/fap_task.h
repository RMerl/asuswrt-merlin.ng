#ifndef __FAP_TASK_H_INCLUDED__
#define __FAP_TASK_H_INCLUDED__

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
 * File Name  : fap_task.h
 *
 * Description: This file contains the constants and structs for FAP 4ke Tasks.
 *
 *******************************************************************************
 */

#include "fap_dll.h"

#ifndef __FAP_H_INCLUDED__
#define FAP_SUCCESS  0
#define FAP_ERROR   -1
#endif

typedef int32 fapRet;

typedef enum {
    FAP4KE_TASK_PRIORITY_HIGH=0,
    FAP4KE_TASK_PRIORITY_MEDIUM,
    FAP4KE_TASK_PRIORITY_LOW,
    FAP4KE_TASK_PRIORITY_MAX
} fap4keTsk_taskPriority_t;

typedef fapRet(* fap4keTsk_handler_t)(uint32 arg);

typedef struct {
    Dll_t node; /* used to maintain linked-lists of tasks */
    fap4keTsk_handler_t handler;
    uint32 arg;
    char *name;
    uint32 refCount;
} fap4keTsk_task_t;

#endif  /* defined(__FAP_TASK_H_INCLUDED__) */
