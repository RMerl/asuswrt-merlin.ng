/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
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

#ifndef __CMS_LCK_H__
#define __CMS_LCK_H__

/*!\file cms_lck.h
 * \brief Header file containing locking functions that
 *  management entities use to control access to the MDM.
 *
 * Usage: whenever a management entity is about to call
 * a function in the OBJ, PHL, or MGM layers, it must
 * acquire a lock.
 * For function calls that require several entries into the
 * MDM, PHL, or MGM layers (e.g. a series of getNextObject calls),
 * the lock may be held over these calls to ensure that the
 * structure of the MDM does not change in the middle of the
 * sequence.
 * The management entity should try to hold the lock too long.
 * (i.e. grab lock, do what is needed, and release it.)
 */


#include "cms.h"
#include "cms_tms.h"


/** Maximum time that an application should hold a lock.
 *
 * When an application releases the lock, the locking code will
 * calculate the amount of time the application held the lock.
 * If the hold time is greater than this value, the locking code
 * will print out some error messages.
 */
#define CMSLCK_MAX_HOLDTIME    (5*MSECS_IN_SEC)


/** The number of bytes used to store function names of lock holders. */
#define CMSLCK_FUNC_NAME_LENGTH   BUFLEN_64


/** This structure is filled in by cmsLck_getLockInfo().
 *
 */
typedef struct 
{
   UBOOL8         locked;       /**< TRUE if a process has the lock. */
   UINT8          funcCode;     /**< 's' for STL, 'r' for RCL */
   MdmObjectId    oid;          /**< current oid accessed */
   SINT32         lockOwner;    /**< Pid of current lock owner. */
   CmsTimestamp   timeAquired;   /**< Time stamp of when lock was aquired. */
   char           callerFuncName[CMSLCK_FUNC_NAME_LENGTH]; /**< Function name of caller who aquired the lock. */
} CmsLckInfo;

#ifdef __cplusplus
extern "C" {
#endif

/** Get information about the current state of the global CMS MDM lock.
 *
 * You do not need to have the lock to call this function.
 *
 * @param lockInfo  (OUT) On return, this structure is filled in with lock info.
 */
void cmsLck_getInfo(CmsLckInfo *lockInfo);


/** Dump CMS MDM lock info.
 *
 * You do not need to have the lock to call this function.
 */
void cmsLck_dumpInfo(void);


/** Acquire the MDM lock.
 *
 * Do not call this function directly.  Use the cmsLck_acquireLock() macro instead.
 * Caller must have called cmsMdm_init() prior to calling this function.
 * If caller already has the MDM lock, calling this function
 * again will return CMSRET_WOULD_DEADLOCK.
 * Only one process/management entity may have the lock at any point
 * in time.
 *
 * @param callerFuncName (IN) caller function's name.
 * @return CmsRet enum.
 */
CmsRet cmsLck_acquireLockTraced(const char* callerFuncName);


/** Acquire the MDM lock and wait at most the specified number of 
 * milliseconds if another process/management entity is holding the lock.
 *
 * Do not call this function directly.  Use the
 * cmsLck_acquireLockWithTimeout() macro instead.
 * Caller must have called cmsMdm_init() prior to calling this function.
 * Caller must not be currently holding the lock.
 * If the lock could not be acquired in the specified timeout time, then
 * CMSRET_TIMED_OUT will be returned.
 *
 * @param callerFuncName (IN) caller function's name.
 * @return CmsRet enum.
 */
CmsRet cmsLck_acquireLockWithTimeoutTraced(const char* callerFuncName, UINT32 timeoutMilliSeconds);


/** Release the lock held by the caller.
 *
 *  Do not call this function directly.  Use the cmsLck_releaseLock() macro instead.
 *
 *  @param callerFuncName (IN) caller function's name.
 */
void cmsLck_releaseLockTraced(const char* callerFuncName);

#ifdef __cplusplus
} /* end of extern "C" */
#endif


/** Acquire the MDM lock.*/
#define cmsLck_acquireLock()  cmsLck_acquireLockTraced(__FUNCTION__)


/** Acquire the MDM lock and wait at most the specified number of 
 * milliseconds if another process/management entity is holding the lock.
 */
#define cmsLck_acquireLockWithTimeout(x)  cmsLck_acquireLockWithTimeoutTraced(__FUNCTION__, x)


/** Release the lock held by the caller.*/
#define cmsLck_releaseLock()  cmsLck_releaseLockTraced(__FUNCTION__)


#endif /* __CMS_LCK_H__ */
