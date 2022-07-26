/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2018:DUAL/GPL:standard

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
 *
 ************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syslog.h>

#include "sysutil.h"
#include "sysutil_proc.h"
#include "bcm_ulog.h"
#include "bcm_timestamp.h"


/** These variables are now used by CMS Log as well.  They are not static,
 * but don't advertise their visibility in the header file either.  Normal
 * callers should still use APIs to get/set them.
 * uLogLevel, ulogDestMask, ulogHdrMask, ulogAppName will apply to all threads
 * in a multi-threaded application unless a thread-specific value has been
 * set via setLevelEx, setDestMaskEx, setHdrMaskEx, or setNameEx.
 */
BcmuLogLevel ulogLevel = BCMULOG_DEFAULT_LEVEL;
UINT32 ulogDestMask = BCMULOG_DEFAULT_DESTMASK;
UINT32 ulogHdrMask = BCMULOG_DEFAULT_HDRMASK;
char ulogAppName[BCMULOG_MAX_APP_NAME_LEN] = {0};

// For performance tracing messages (for all threads)
UINT32 uTraceDestMask = BCMULOG_DESTMASK_SYSLOG;


// These are per-thread level, destMask, hdrMask, and appName settings.
// According to StackOverflow (C89 standard), global vars are automatically
// initialized to 0.  But I still like to initialize them if possible.
BcmuLogThreadInfo ulogLevelArray[BCMULOG_NUM_THREAD_SLOTS];
UINT32 ulogLevelEntries = 0;

BcmuLogThreadInfo ulogDestMaskArray[BCMULOG_NUM_THREAD_SLOTS];
UINT32 ulogDestMaskEntries = 0;

BcmuLogThreadInfo ulogHdrMaskArray[BCMULOG_NUM_THREAD_SLOTS];
UINT32 ulogHdrMaskEntries = 0;

BcmuLogThreadInfo ulogAppNameArray[BCMULOG_NUM_THREAD_SLOTS];
UINT32 ulogAppNameEntries = 0;

// EID to threadId mapping table.
BcmuLogThreadInfo ulogEidToThreadIdArray[BCMULOG_NUM_THREAD_SLOTS];
UINT32 ulogEidToThreadIdEntries = 0;


static UINT32 getMyValueCommon(const BcmuLogThreadInfo *array, const char *arrayName,
                               UINT32 numSlots, UINT32 numEntries,
                               SINT32 threadId, UINT32 eid,
                               UINT32 globalValue)
{
   UINT32 i;

   if (threadId == 0 && eid != 0)
   {
      threadId = bcmuLog_getEidToThreadId(eid);
   }

   if (threadId == 0)
   {
      bcmuLog_debug("table %s: unknown threadId (eid=%d) returning app-wide value 0x%x",
                    arrayName, eid, globalValue);
      return globalValue;
   }

   // Most common case: no per thread info, just return the app-wide setting.
   if (numEntries == 0)
      return globalValue;

   for (i=0; i < numSlots; i++)
   {
      if (array[i].threadId == threadId)
         return (array[i].value);
   }

   // setting for this thread not found, return app-wide setting.
   return globalValue;
}

static void setValueNameCommon(BcmuLogThreadInfo *array, const char *arrayName,
                               UINT32 numSlots, UINT32 *numEntries,
                               SINT32 threadId, UINT32 eid,
                               const char *name, UINT32 value)
{
   UINT32 i;

   if (threadId != 0 && eid != 0)
   {
      // both are specified, so we can fill in the mapping entry.
      bcmuLog_setEidToThreadId(eid, threadId);
   }

   if (threadId == 0 && eid != 0)
   {
      threadId = bcmuLog_getEidToThreadId(eid);
   }

   if (threadId == 0)
   {
      bcmuLog_error("table %s: unknown threadId %d (eid=%d)",
                    arrayName, threadId, eid);
      return;
   }

   // At this point, we have a valid threadId.
   // First check for modification to existing entry.
   for (i=0; i < numSlots; i++)
   {
      if (array[i].threadId == threadId)
      {
         if (name != NULL)
         {
            strncpy(array[i].appName, name, sizeof(array[i].appName)-1);
         }
         else
         {
            array[i].value = value;
         }
         return;
      }
   }

   // New threadId setting, try to find an empty slot.
   // TODO: add locking?  Technically, it is possible that two threads will
   // find the same slot free and try to fill it.  But in practice, very few
   // threads will set their per-thread info, and usually only once right at
   // startup, so it is extremely unlikely we will have a collision/overwrite.
   for (i=0; i < numSlots; i++)
   {
      if (array[i].threadId == 0)
      {
         array[i].threadId = threadId;
         if (name != NULL)
         {
            strncpy(array[i].appName, name, sizeof(array[i].appName)-1);
         }
         else
         {
            array[i].value = value;
         }
         (*numEntries)++;
         return;
      }
   }

   // No more empty slots.
   // TODO: there is no mechanism to clear a slot after it has been set.
   // Assuming all of these threads are long-lived.
   bcmuLog_error("BCM ULog (%s) thread table full (max=%d)",
                 arrayName, numSlots);
   return;
}


void bcmuLog_formatInfo(FILE *stream, BcmuLogFormatInfo *info)
{
   if (info->logHeaderMask & BCMULOG_HDRMASK_APPNAME)
   {
      if (info->appName[0] != '\0')
      {
         fprintf(stream, "%s", info->appName);
      }
   }
   fprintf(stream, ":");

   if (info->logHeaderMask & BCMULOG_HDRMASK_THREAD_ID)
   {
      int tid = sysUtl_gettid();
      fprintf(stream, "%d", tid);
   }
   fprintf(stream, ":");

   /*
    * Eventhough syslog will also log the time, this timestamp is more accurate.
    * But caller can always turn it off.
    */
   if (info->logHeaderMask & BCMULOG_HDRMASK_TIMESTAMP)
   {
      struct timespec ts = {0, 0};
      bcm_libc_clock_gettime(CLOCK_MONOTONIC, &ts);
      fprintf(stream, "%lu.%03lu", ts.tv_sec%1000, ts.tv_nsec/1000000);
   }
   fprintf(stream, ":");

   if (info->logHeaderMask & BCMULOG_HDRMASK_LEVEL)
   {
      /*
       * Don't include severity when going to syslog because syslog puts
       * that in for us.
       */
      if (!(info->logDestMask & BCMULOG_DESTMASK_SYSLOG))
      {
         char *logLevelStr;
         switch(info->logLevel)
         {
         case BCMULOG_LEVEL_ERR:
            logLevelStr = "error";
            break;
         case BCMULOG_LEVEL_NOTICE:
            logLevelStr = "notice";
            break;
         case BCMULOG_LEVEL_DEBUG:
            logLevelStr = "debug";
            break;
         case BCMULOG_LEVEL_PERFTRACE:
            logLevelStr = "trace";
            break;
         default:
            logLevelStr = "invalid-log-level";
            break;
         }
         fprintf(stream, "%s", logLevelStr);
      }
   }
   fprintf(stream, ":");

   if (info->logHeaderMask & BCMULOG_HDRMASK_LOCATION)
   {
      fprintf(stream, "%s:%u", info->funcName, info->lineNum);
   }
   fprintf(stream, ":");
}

void bcmuLog_formatLine(FILE *stream, BcmuLogFormatInfo *info, const char *pFmt, va_list ap)
{
   bcmuLog_formatInfo(stream, info);

   vfprintf(stream, pFmt, ap);
}

void bcmuLog_formatPerfTrace(FILE *stream, BcmuLogFormatInfo *info, const char *anchor, const char *pFmt, va_list ap)
{
   bcmuLog_formatInfo(stream, info);
   fprintf(stream, "%s:", anchor);
   vfprintf(stream, pFmt, ap);
}

void bcmuLog_setLevel(UINT32 level)
{
   ulogLevel = level;
}

void bcmuLog_setLevelEx(UINT32 level, SINT32 threadId, UINT32 eid)
{
   setValueNameCommon(ulogLevelArray, "level",
                      BCMULOG_NUM_THREAD_SLOTS, &ulogLevelEntries,
                      threadId, eid,
                      NULL, level);
   return;
}

UINT32 bcmuLog_getLevel(void)
{
   return ulogLevel;
}

UINT32 bcmuLog_getLevelEx(SINT32 threadId, UINT32 eid)
{
   return (getMyValueCommon(ulogLevelArray, "level",
                            BCMULOG_NUM_THREAD_SLOTS, ulogLevelEntries,
                            threadId, eid,
                            (UINT32) ulogLevel));
}

void bcmuLog_setHdrMask(UINT32 mask)
{
   ulogHdrMask = mask;
}

void bcmuLog_setHdrMaskEx(UINT32 mask, SINT32 threadId, UINT32 eid)
{
   setValueNameCommon(ulogHdrMaskArray, "hdrMask",
                      BCMULOG_NUM_THREAD_SLOTS, &ulogHdrMaskEntries,
                      threadId, eid,
                      NULL, mask);
   return;
}

UINT32 bcmuLog_getHdrMask(void)
{
   return ulogHdrMask;
}

UINT32 bcmuLog_getHdrMaskEx(SINT32 threadId, UINT32 eid)
{
   return (getMyValueCommon(ulogHdrMaskArray, "hdrMask",
                            BCMULOG_NUM_THREAD_SLOTS, ulogHdrMaskEntries,
                            threadId, eid,
                            ulogHdrMask));
}

void bcmuLog_setDestMask(UINT32 mask)
{
   ulogDestMask = mask;
}

void bcmuLog_setDestMaskEx(UINT32 mask, SINT32 threadId, UINT32 eid)
{
   setValueNameCommon(ulogDestMaskArray, "destMask",
                      BCMULOG_NUM_THREAD_SLOTS, &ulogDestMaskEntries,
                      threadId, eid,
                      NULL, mask);
   return;
}

UINT32 bcmuLog_getDestMask()
{
   return ulogDestMask;
}

UINT32 bcmuLog_getDestMaskEx(SINT32 threadId, UINT32 eid)
{
   return (getMyValueCommon(ulogDestMaskArray, "destMask",
                            BCMULOG_NUM_THREAD_SLOTS, ulogDestMaskEntries,
                            threadId, eid,
                            ulogDestMask));
}

void bcmuLog_setName(const char *name)
{
   if (name != NULL)
   {
      strncpy(ulogAppName, name, sizeof(ulogAppName)-1);
   }
}

void bcmuLog_setNameFromProc()
{
   ProcThreadInfo tInfo;
   int tid;

   tid = sysUtl_gettid();
   sysUtl_getThreadInfoFromProc(tid, &tInfo);
   strncpy(ulogAppName, tInfo.name, sizeof(ulogAppName)-1);
   return;
}

void bcmuLog_setNameEx(const char *name, SINT32 threadId, UINT32 eid)
{
   if (name != NULL)
   {
      setValueNameCommon(ulogAppNameArray, "appName",
                         BCMULOG_NUM_THREAD_SLOTS, &ulogAppNameEntries,
                         threadId, eid,
                         name, 0);
   }
   return;
}

const char *bcmuLog_getName(void)
{
   if (ulogAppName[0] == '\0')
   {
      bcmuLog_setNameFromProc();
   }

   return ulogAppName;
}

const char *bcmuLog_getNameEx(SINT32 threadId, UINT32 eid)
{
   UINT32 i;

   if (ulogAppName[0] == '\0')
   {
      bcmuLog_setNameFromProc();
   }

   if (threadId == 0 && eid != 0)
   {
      threadId = bcmuLog_getEidToThreadId(eid);
   }

   if (threadId == 0)
   {
      bcmuLog_debug("table appName: unknown threadId (eid=%d) returning app-wide value %s",
                    eid, ulogAppName);
      return ulogAppName;
   }

   // Most common case: no per thread info, just return the app-wide setting.
   if (ulogAppNameEntries == 0)
      return ulogAppName;

   for (i=0; i < BCMULOG_NUM_THREAD_SLOTS; i++)
   {
      if (ulogAppNameArray[i].threadId == threadId)
         return (ulogAppNameArray[i].appName);
   }

   // setting for this thread not found, return app-wide setting.
   return ulogAppName;
}


void bcmuLog_setEidToThreadId(UINT32 eid, SINT32 threadId)
{
   UINT32 i;

   // First check for modification to existing entry.
   for (i=0; i < BCMULOG_NUM_THREAD_SLOTS; i++)
   {
      if (ulogEidToThreadIdArray[i].eid == eid)
      {
         ulogEidToThreadIdArray[i].threadId = threadId;
         return;
      }
   }

   // New mapping entry, try to find an empty slot.
   for (i=0; i < BCMULOG_NUM_THREAD_SLOTS; i++)
   {
      if (ulogEidToThreadIdArray[i].eid == 0)
      {
         ulogEidToThreadIdArray[i].eid = eid;
         ulogEidToThreadIdArray[i].threadId = threadId;
         ulogEidToThreadIdEntries++;
         return;
      }
   }

   // No more empty slots.
   // TODO: there is no mechanism to clear a slot after it has been set.
   // Assuming all of these threads are long-lived.
   bcmuLog_error("BCM uLog (eidToThreadId) table full (max=%d)",
                 BCMULOG_NUM_THREAD_SLOTS);
   return;
}

SINT32 bcmuLog_getEidToThreadId(UINT32 eid)
{
   UINT32 i;

   // Don't bother scanning through the array if there are no entries.
   if (ulogEidToThreadIdEntries == 0)
      return 0;

   for (i=0; i < BCMULOG_NUM_THREAD_SLOTS; i++)
   {
      if (ulogEidToThreadIdArray[i].eid == eid)
      {
         return ulogEidToThreadIdArray[i].threadId;
      }
   }

   return 0;
}

static void fillFormatInfo(BcmuLogLevel level,
                           const char *funcName,
                           UINT32 lineNum,
                           BcmuLogFormatInfo *info)
{
   SINT32 threadId = sysUtl_getThreadId();

   memset(info, 0, sizeof(BcmuLogFormatInfo));
   info->logLevel = level;
   info->logDestMask = bcmuLog_getDestMaskEx(threadId, 0);
   info->logHeaderMask = bcmuLog_getHdrMaskEx(threadId, 0);
   info->lineNum = lineNum;
   strncpy(info->funcName, funcName, sizeof(info->funcName)-1);
   if (ulogAppName[0] == '\0')
   {
      bcmuLog_setNameFromProc();
   }
   strncpy(info->appName, bcmuLog_getNameEx(threadId, 0), sizeof(info->appName)-1);
}

static void outputLogBuf(int level, const char *buf)
{
   if (ulogDestMask & BCMULOG_DESTMASK_STDERR)
   {
      fprintf(stderr, "%s\n", buf);
      fflush(stderr);
   }

   if (ulogDestMask & BCMULOG_DESTMASK_SYSLOG)
   {
      /* use of openlog is optional, so just call syslog */
      syslog(level, "%s", buf);
   }

   // Now that CMS Log uses ulogDestMask to store its own log destination
   // state, it is possible uLogDestkMask has the telnet bit set.  Ignore it
   // for now.
   return;
}

void ulog_log(BcmuLogLevel level, const char *funcName, UINT32 lineNum, const char *pFmt, ... )
{
   char *logBuf = NULL;
   size_t bufSize;
   BcmuLogFormatInfo info;
   FILE *stream = NULL;
   SINT32 threadId;
   va_list ap;

   threadId = sysUtl_getThreadId();
   if (level > bcmuLog_getLevelEx(threadId, 0))
   {
      return;
   }

   stream = open_memstream(&logBuf, &bufSize);
   if (stream == NULL)
   {
       printf("Failed to open memory stream!\n");
       return;
   }

   fillFormatInfo(level, funcName, lineNum, &info);
   va_start(ap, pFmt);
   bcmuLog_formatLine(stream, &info, pFmt, ap);
   va_end(ap);

   fflush(stream);
   fclose(stream);

   outputLogBuf(level, logBuf);
   free(logBuf);
}

static void outputTraceBuf(const char *buf)
{

   if (uTraceDestMask & BCMULOG_DESTMASK_STDERR)
   {
       fprintf(stderr, "%s\n", buf);
       fflush(stderr);
   }

   if (uTraceDestMask & BCMULOG_DESTMASK_SYSLOG)
   {
       syslog(LOG_DEBUG, "%s", buf);
   }

   return;
}

void ulog_trace(const char *funcName, UINT32 lineNum, const char *anchor, const char *pFmt, ... )
{
   char *logBuf = NULL;
   size_t bufSize;
   BcmuLogFormatInfo info;
   FILE *stream = NULL;
   va_list ap;

   stream = open_memstream(&logBuf, &bufSize);
   if (stream == NULL)
   {
       printf("Failed to open memory stream!\n");
       return;
   }

   fillFormatInfo(BCMULOG_LEVEL_PERFTRACE, funcName, lineNum, &info);
   va_start(ap, pFmt);
   bcmuLog_formatPerfTrace(stream, &info, anchor, pFmt, ap);
   va_end(ap);

   fflush(stream);
   fclose(stream);

   outputTraceBuf(logBuf);
   free(logBuf);
}
