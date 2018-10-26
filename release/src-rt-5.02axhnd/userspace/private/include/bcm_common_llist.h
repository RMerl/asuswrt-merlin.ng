/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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
 *
 ************************************************************************/

#ifndef _BCM_COMMON_LLIST_H_
#define _BCM_COMMON_LLIST_H_

/*
 * Macros
 */

/* Linked List API */

#define BCM_COMMON_DECLARE_LL(_name) BCM_COMMON_LLIST _name

#define BCM_COMMON_DECLARE_LL_ENTRY() BCM_COMMON_LLIST_ENTRY llEntry

#define BCM_COMMON_LL_INIT(_linkedList)                         \
    do {                                                        \
        (_linkedList)->head = NULL;                             \
        (_linkedList)->tail = NULL;                             \
    } while(0)

/* #define BCM_COMMON_LL_IS_EMPTY(_linkedList)                               \ */
/*     ( ((_linkedList)->head == NULL) && ((_linkedList)->tail == NULL) ) */

#define BCM_COMMON_LL_IS_EMPTY(_linkedList) ( (_linkedList)->head == NULL )

#define BCM_COMMON_LL_INSERT(_linkedList, _newObj, _pos, _currObj)      \
    do {                                                                \
        if(BCM_COMMON_LL_IS_EMPTY(_linkedList))                         \
        {                                                               \
            (_linkedList)->head = (void *)(_newObj);                    \
            (_linkedList)->tail = (void *)(_newObj);                    \
            (_newObj)->llEntry.prev = NULL;                             \
            (_newObj)->llEntry.next = NULL;                             \
        }                                                               \
        else                                                            \
        {                                                               \
            if((_pos) == LL_POSITION_APPEND)                    \
            {                                                           \
                typeof(*(_newObj)) *_tailObj = (_linkedList)->tail;     \
                _tailObj->llEntry.next = (_newObj);                     \
                (_newObj)->llEntry.prev = _tailObj;                     \
                (_newObj)->llEntry.next = NULL;                         \
                (_linkedList)->tail = (_newObj);                        \
            }                                                           \
            else                                                        \
            {                                                           \
                if((_pos) == LL_POSITION_BEFORE)                \
                {                                                       \
                    typeof(*(_newObj)) *_prevObj = (_currObj)->llEntry.prev; \
                    (_currObj)->llEntry.prev = (_newObj);               \
                    (_newObj)->llEntry.prev = _prevObj;                 \
                    (_newObj)->llEntry.next = (_currObj);               \
                    if(_prevObj != NULL)                                \
                    {                                                   \
                        _prevObj->llEntry.next = (_newObj);             \
                    }                                                   \
                    if((_linkedList)->head == (_currObj))               \
                    {                                                   \
                        (_linkedList)->head = (_newObj);                \
                    }                                                   \
                }                                                       \
                else                                                    \
                {                                                       \
                    typeof(*(_newObj)) *_nextObj = (_currObj)->llEntry.next; \
                    (_currObj)->llEntry.next = (_newObj);               \
                    (_newObj)->llEntry.prev = (_currObj);               \
                    (_newObj)->llEntry.next = _nextObj;                 \
                    if(_nextObj != NULL)                                \
                    {                                                   \
                        _nextObj->llEntry.prev = (_newObj);             \
                    }                                                   \
                    if((_linkedList)->tail == (_currObj))               \
                    {                                                   \
                        (_linkedList)->tail = (_newObj);                \
                    }                                                   \
                }                                                       \
            }                                                           \
        }                                                               \
    } while(0)

#define BCM_COMMON_LL_APPEND(_linkedList, _obj)                         \
    BCM_COMMON_LL_INSERT(_linkedList, _obj, LL_POSITION_APPEND, _obj)

#define BCM_COMMON_LL_REMOVE(_linkedList, _obj)                         \
    do {                                                                \
        if((_linkedList)->head == (_obj) && (_linkedList)->tail == (_obj)) \
        {                                                               \
            (_linkedList)->head = NULL;                                 \
            (_linkedList)->tail = NULL;                                 \
        }                                                               \
        else                                                            \
        {                                                               \
            if((_linkedList)->head == (_obj))                           \
            {                                                           \
                typeof(*(_obj)) *_nextObj = (_obj)->llEntry.next;       \
                (_linkedList)->head = _nextObj;                         \
                _nextObj->llEntry.prev = NULL;                          \
            }                                                           \
            else if((_linkedList)->tail == (_obj))                      \
            {                                                           \
                typeof(*(_obj)) *_prevObj = (_obj)->llEntry.prev;       \
                (_linkedList)->tail = _prevObj;                         \
                _prevObj->llEntry.next = NULL;                          \
            }                                                           \
            else                                                        \
            {                                                           \
                typeof(*(_obj)) *_prevObj = (_obj)->llEntry.prev;       \
                typeof(*(_obj)) *_nextObj = (_obj)->llEntry.next;       \
                _prevObj->llEntry.next = (_obj)->llEntry.next;          \
                _nextObj->llEntry.prev = (_obj)->llEntry.prev;          \
            }                                                           \
        }                                                               \
    } while(0)

#define BCM_COMMON_LL_GET_HEAD(_linkedList) (_linkedList).head

#define BCM_COMMON_LL_GET_NEXT(_obj) (_obj)->llEntry.next

/*
 * Type definitions
 */

typedef enum {
    LL_POSITION_BEFORE=0,
    LL_POSITION_AFTER,
    LL_POSITION_APPEND,
    LL_POSITION_MAX
} LL_INSERT_POSITION;

typedef struct {
    void* head;
    void* tail;
} BCM_COMMON_LLIST, *PBCM_COMMON_LLIST;

typedef struct {
    void* prev;
    void* next;
} BCM_COMMON_LLIST_ENTRY, *PBCM_COMMON_LLIST_ENTRY;

#endif /* _BCM_COMMON_LLIST_H_ */
