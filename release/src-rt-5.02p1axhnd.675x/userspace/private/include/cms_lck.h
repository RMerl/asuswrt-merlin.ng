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
 * \brief Header file for MDM locking functions.
 *
 * Usage: before a process or thread reads or writes to the MDM via the
 * CMS OBJ, PHL, or MGM API, it must acquire a lock by calling
 * cmsLck_acquireLock() or cmsLck_acquireLockWithTimeout(), and when it is
 * done, it must call cmsLck_releaseLock().
 *
 * Prior to 5.02L.07, there was a single (global) MDM lock.
 * Starting with 5.02L.07, the MDM has been divided into multiple lock zones.
 * However, existing and new code can still continue to use
 * cmsLck_acquireLock, cmsLck_acquireLockWithTimeout() and cmsLck_releaseLock.
 * The CMS OBJ, PHL, MGM layers will ensure the correct zones are locked and
 * unlocked.  (Additional zone locking APIs are defined, but application code
 * do not need to use them.)
 * 
 * There are 4 subtle changes in locking behavior that application developers
 * should be aware of:
 * 1. cmsLck_acquireLock() and cmsLck_acquireLockWithTimeout() do not acquire
 *    any locks.  So even when these functions return successfully, it does
 *    not mean the application has acquired any locks.  The locks are acquired
 *    when the application code calls the CMS OBJ, PHL, or MGM APIs.  Therefore,
 *    the report of locking errors will occur later in the code (e.g. during
 *    cmsObj_get() or cmsPhl_getParamValue()).
 * 2. An application can nest calls to cmsLck_acquireLock() and 
 *    cmsLck_releaseLock().  For example, the following sequence is allowed:
 *    cmsLck_acquireLock(), cmsLck_acquireLock(), cmsLck_releaseLock(),
 *    cmsLck_releaseLock().  All zone locks will be released on the last call
 *    to cmsLck_releaseLock().
 * 3. When an application calls cmsLck_acquireLock(), it is telling
 *    the locking system to pick a suitable timeout for acquiring a lock.
 *    The locking system will never block forever waiting for a lock -- this
 *    behavior is needed to prevent deadlocks.
 * 4. In order to support zone locking, the locking system has been enhanced
 *    with the ability to automatically lock and unlock zones as needed.  This
 *    feature is called full auto-locking.  Full auto-locking should be
 *    considered experimental and is not supported at this point.  However,
 *    customers are welcome to try it on a limited basis.  With full
 *    auto-locking, application code does not even need to call
 *    cmsLck_acquireLock() or cmsLck_releaseLock() anymore. The locking system
 *    will automatically lock and unlock the appropriate zones.  However, full
 *    auto-locking comes with one restriction: applications which get a MDM
 *    object via the CMS OBJ API must free the object when it is done.  (The
 *    object must not be stored away indefinitely.  The locking system will
 *    unlock a zone only after all MDM objects acquired haave been freed.  This
 *    restriction applies only when full auto-locking is in effect.  It does
 *    not apply to code which uses cmsLck_acquireLock and then cmsObj_get.)
 */


#include "cms.h"
#include "cms_tms.h"


/** Tell CMS locking system that application code is about to access the MDM.
 *
 * The locking system will pick an appropriate timeout to use for locking.
 * No locks are acquired in this function, so a successful return does not
 * mean any locks has been acquired.  Caller should still check return value
 * for other errors though.
 * @return CmsRet enum.
 */
#define cmsLck_acquireLock()  cmsLck_acquireLockTraced(__FUNCTION__)


/** Same as cmsLck_acquireLock except application code is specifying the
 *  timeout value (in milliseconds).
 *
 * @param  x (IN) timeout in milliseconds.
 * @return CmsRet enum.
 */
#define cmsLck_acquireLockWithTimeout(x)  cmsLck_acquireLockWithTimeoutTraced(__FUNCTION__, x)


/** Release all MDM zone locks held by the caller.*/
#define cmsLck_releaseLock()  cmsLck_releaseLockTraced(__FUNCTION__)


/** Maximum time that an application should hold a lock and will wait for a lock.
 *
 * When an application releases a lock, the locking code will
 * calculate the amount of time the application held the lock.
 * If the hold time is greater than this value, the locking code will print
 * an error message.  The hold time warning threshold can be overridden by
 * the thread in special cases, see cmsLck_setHoldTimeWarnThresh().
 */
#define CMSLCK_MAX_HOLDTIME    (5*MSECS_IN_SEC)


/** Acquire zone lock for the specified zone. Applications should not need to
 *  use zone locking APIs.  Just use cmsLck_acquireLock(),
 *  cmsLck_acquireLockWithTimeout() and cmsLck_releaseLock() as usual.
 */
#define cmsLck_acquireZoneLock(z) cmsLck_acquireZoneLockTraced(z, __FUNCTION__)
#define cmsLck_acquireZoneLockWithTimeout(z, t) cmsLck_acquireZoneLockWithTimeoutTraced(z, t, __FUNCTION__)

/** Acquire zone locks as specified in the zone array. */
#define cmsLck_acquireZoneLocks(z) cmsLck_acquireZoneLocksTraced(z, __FUNCTION__)
#define cmsLck_acquireZoneLocksWithTimeout(z, t) cmsLck_acquireZoneLocksWithTimeoutTraced(z, t, __FUNCTION__)

/** Release the specified zone lock. */
#define cmsLck_releaseZoneLock(z) cmsLck_releaseZoneLockTraced(z, __FUNCTION__)
/** Release the zone locks as specified in the zone array. */
#define cmsLck_releaseZoneLocks(z) cmsLck_releaseZoneLocksTraced(z, __FUNCTION__)

/** Acquire and release all zones locks.  Timeout value is required.  Backoff
 *  and retry will occur if timeout is greater than CMSLCK_MAX_HOLDTIME.
 *  Note that any zone locks held prior to calling acquireAllZoneLocks will
 *  still be held after releaseAllZoneLocks. */
#define cmsLck_acquireAllZoneLocksWithBackoff(o, t) cmsLck_acquireAllZoneLocksWithBackoffTraced(o, t, __FUNCTION__)
#define cmsLck_releaseAllZoneLocks() cmsLck_releaseAllZoneLocksTraced(__FUNCTION__)


/** Maximum number of lock zones (must be less than 255). */
#define MDM_MAX_LOCK_ZONES          16

/** Invalid zone number. */
#define MDM_INVALID_LOCK_ZONE       255

/** Maximum number of threads actively using MDM locks. */
#define MDM_MAX_LOCK_THREADS        20

/** Maximum number of tracked objects per zone. (actually it is this + 1) */
#define MAX_TRACKED_MDMOBJS         100

/** The number of bytes used to store function names of lock holders. */
#define CMSLCK_FUNC_NAME_LENGTH     64

/** Each PID namespace is separated by this amount (used in cmsLck_setPidNsOffset). */
#define CMSLCK_PID_NS_MULTIPLIER     1000000

/** Each locking thread has its own ODL setQ (no more global setQ). */
typedef struct
{
   void    *head;  /**< actually points to a OdlSetQueueEntry */
   void    *tail;  /**< actually points to a OdlSetQueueEntry */
   UINT32   count;
} OdlSetQueue;

/** Each locking thread has its own ODL get cache (no more global get cache).
 *
 * odl_get may be called multiple times to get different fields in 
 * the same object.  So instead of calling the STL handler function for
 * each field, odl_get uses the cached object as long as odl_get is
 * being called for the same object and no other odl functions have
 * been called.
 */
typedef struct 
{
   void *mdmObj;
   InstanceIdStack iidStack;
   CmsTimestamp tms;
} OdlGetCache;

#define INVALIDATE_ODL_GET_CACHE do {\
      if (thread->getCache.mdmObj != NULL) mdm_freeObject(&(thread->getCache.mdmObj)); \
   } while (0)


/** Per zone tracked MDM objects, needed by full auto-lock feature. */
typedef struct
{
   UINT16  count;  /**< curr number of tracked objs, including the single one */
   UINT16  max;    /**< Highest value hit, for stats purposes */
   const void *single; /**< In most cases, there is only 1 mdmObj to track */
   const void **array; /**< Array of MAX_TRACKED_MDMOBJS, alloc when needed */
} TrackedMdmObjs;

/** Info about a thread doing some locking operation. */
typedef struct
{
   SINT32  tid;  /**< ThreadId of this thread info struct */
   UINT32  holdTimeWarnThresh; /**< if thread holds lock over this many millseconds, log error msg */
   UINT32  timeoutMilliSeconds; /**< caller specified lock timeout */
   UBOOL8  useCallerTimeout;   /** caller has specified a lock timeout */
   UINT8   globalLockCount;  /**< count of cmsLck_acquireLock calls */
   UINT8   zoneLockCounts[MDM_MAX_LOCK_ZONES]; /**< count of cmsLck_lockZone calls on this zone */
   UINT8   zoneEntryCounts[MDM_MAX_LOCK_ZONES];
   UBOOL8  hasZones[MDM_MAX_LOCK_ZONES];
   UINT32  wantZone; /**< which zone is this thread currently waiting for */
   UINT32  totalTrackedMdmObjs; /**< total across all zones */
   TrackedMdmObjs tracked[MDM_MAX_LOCK_ZONES];
   OdlGetCache getCache; /** each thread has its own ODL get cache. */
   OdlSetQueue setQ; /**< each thread has its own ODL set queue */
   CmsTimestamp activityTs;  /**< for dead thread detection */
   char funcName[CMSLCK_FUNC_NAME_LENGTH]; /**< Func name which triggered the initial lock. */
} CmsLockThreadInfo;

/** Info about one of the zone locks. */
typedef struct
{
   SINT32         tid;  /**< ThreadId of the lock owner */
   MdmObjectId    firstOid;      /**< OID which initially triggered the lock. */
   CmsTimestamp   acquiredTs;   /**< Timestamp of when lock was acquired. */
   char funcName[CMSLCK_FUNC_NAME_LENGTH]; /**< Func name which triggered the initial lock. */
   MdmObjectId    currOid;  /**< currently processing this OID.  */
   UINT8          currFuncCode; /**<  currently in: 's' for STL, 'r' for RCL */
} CmsLockOwnerInfo;

/** Locking statistics. */
typedef struct
{
   UINT32 successes;
   UINT32 failures;
   UINT32 softFailures; /**< lock failures with short timeout. */
   UINT32 deadThreads; /**< number of dead threads detected */
   UINT32 undos;       /**< kernel UNDO triggered by dead process */
   UINT32 resets;      /**< kernel UNDO no triggered by dead pthread */
   UINT32 threadSlotsFull; /**< no thread slots even after dead thread collection */
   UINT32 trackedMdmObjErrors;
   UINT32 internalErrors; /**< lock bookkeeping or other unexpected errors */
} CmsLockStats;


#ifdef __cplusplus
extern "C" {
#endif

/** Get the MDM lock zone for the specified oid.
 *
 * @param oid (IN) The oid of the object node.
 *
 * @return MDM_INVALID_LOCK_ZONE if the oid is not valid.
 */
UINT8 cmsLck_getLockZone(MdmObjectId oid);

/** Return TRUE if this is a top-level lock zone.
 *  A top-level lock zone is a lock zone that has other lock zones under it in
 *  the data model tree.  Currently, only zones 0 and 1 are top-level zones,
 *  which means all other lock zones must not have any other lock zones under
 *  them in the data model tree.
 */
UBOOL8 cmsLck_isTopLevelLockZone(UINT8 zone);

/** Set the lock hold time warning threshold (in milliseconds) for this thread.
 *
 *  By default, the warning threshold is CMSLCK_MAX_HOLDTIME, but can be set
 *  to a different value for special cases.  However, this setting is
 *  lost when the thread releases all of its locks, meaning that it needs to
 *  be set at the beginning of every (special case) lock operation.
 *
 *  @param thresh (IN) warning threshold in milliseconds.
 */
void cmsLck_setHoldTimeWarnThresh(UINT32 thresh);

/** Set the pid namespace offset for this app.
 *
 *  Apps running in containers (which have their own pid namespace) must
 *  call this after cmsMdm_init() and before any locking calls.
 *
 * @param offset (IN) Must be a multiple of CMSLCK_PID_NS_MULTIPLIER and be
 *          unique among containers.  Broadcom apps can use their
 *          EID%1000 * CMSLCK_PID_NS_MULTIPLIER as their offset.  Third party
 *          apps can use offsets in range [1000-1999] * MULTIPLIER.
 *
 * @return CmsRet.
 */
CmsRet cmsLck_setPidNsOffset(SINT32 offset);

/** Toggle lock tracing.  Currently, tracing is either on or off. */
void cmsLck_toggleTracing();

/** Dump all locking info.  Includes lock owners, threads, and stats. */
void cmsLck_dumpInfo();

/** Get a snapshot of all lock threads.
 *
 * @param (OUT) threads must point to an array of MDM_MAX_LOCK_THREADS CmsLockThreadInfo structs.
 */
void cmsLck_getLockThreadInfo(CmsLockThreadInfo *threads);

/** Get a snapshot of all lock owners.
 *
 * @param (OUT) owners must point to an array of MDM_MAX_LOCK_ZONES CmsLockOwnerInfo structs.
 */
void cmsLck_getLockOwnerInfo(CmsLockOwnerInfo *owners);

/** Get a snapshot of the lock stats.
 *
 * @param (OUT) stats must point to a CmsLockStats struct.
 */
void cmsLck_getLockStats(CmsLockStats *stats);

/** Pretty print the info returned by cmsLck_getLockThreadInfo(). */
void cmsLck_dumpLockThreadInfo(const CmsLockThreadInfo *thread);
 
/** Pretty print the info returned by cmsLck_getLockOwnerInfo(). */
void cmsLck_dumpLockOwnerInfo(const CmsLockOwnerInfo *owners);
 
/** Pretty print the info returned by cmsLck_getLockStats(). */
void cmsLck_dumpLockStats(const CmsLockStats *stats);
 

/*
 * Everything below here are for internal use by the locking code.
 * Application developers can ignore it.
 */
 
/** Do not call this function directly.  Call cmsLck_acquireLock() instead. */
CmsRet cmsLck_acquireLockTraced(const char* callerFuncName);

/** Do not call this function directly.  Call cmsLck_acquireLockWithTimeout() instead. */
CmsRet cmsLck_acquireLockWithTimeoutTraced(const char* callerFuncName, UINT32 timeoutMilliSeconds);

/** Do not call this function directly.  Call cmsLck_acquireLock() instead. */
void cmsLck_releaseLockTraced(const char* callerFuncName);

CmsRet cmsLck_acquireZoneLockTraced(UINT8 zone, const char* callerFuncName);
CmsRet cmsLck_acquireZoneLockWithTimeoutTraced(UINT8 zone,
                    UINT32 timeoutMilliSeconds, const char* callerFuncName);

CmsRet cmsLck_acquireZoneLocksTraced(UBOOL8 *zones, const char* callerFuncName);
CmsRet cmsLck_acquireZoneLocksWithTimeoutTraced(UBOOL8 *zones,
                       UINT32 timeoutMilliSeconds, const char* callerFuncName);

void cmsLck_releaseZoneLockTraced(UINT8 zone, const char* callerFuncName);
void cmsLck_releaseZoneLocksTraced(UBOOL8 *zones, const char* callerFuncName);

CmsRet cmsLck_acquireAllZoneLocksWithBackoffTraced(MdmObjectId oid,
                       UINT32 timeoutMilliSeconds, const char *callerFuncName);
void cmsLck_releaseAllZoneLocksTraced(const char *callerFuncName);


/** Semaphore key for the MDM Zone locks (for internal use) */
#define MDM_ZONE_LOCK_SEMAPHORE_KEY 0x5eb8

/** Semaphore key for the MDM lock meta-info tables (for internal use). */
#define MDM_META_LOCK_SEMAPHORE_KEY 0x5eb9

#ifdef __cplusplus
} /* end of extern "C" */
#endif


#endif /* __CMS_LCK_H__ */
