/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2018:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
 *
 ************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syslog.h>

#include "sysutil.h"
#include "sysutil_proc.h"
#include "bcm_ulog.h"


static BcmuLogLevel ulogLevel = BCMULOG_DEFAULT_LEVEL;
static UINT32 ulogDestMask = BCMULOG_DEFAULT_DESTMASK;
static UINT32 ulogHdrMask = BCMULOG_DEFAULT_HDRMASK;
static char appName[BCMULOG_MAX_APP_NAME_LEN] = {0};


void bcmuLog_formatLine(BcmuLogFormatInfo *info, const char *pFmt, va_list ap)
{
   UINT32 len=0;
   UINT32 maxLen;

   /* start with empty buf and reserve 1 byte at end for termination. */
   memset(info->buf, 0, info->bufLen);
   maxLen = info->bufLen - 1;

   if ((info->logHeaderMask & BCMULOG_HDRMASK_APPNAME) && (len < maxLen))
   {
      if (info->appName[0] != '\0')
      {
         len += snprintf(&(info->buf[len]), maxLen - len, "%s:", info->appName);
      }
   }

   if ((info->logHeaderMask & BCMULOG_HDRMASK_THREAD_ID) && (len < maxLen))
   {
      int tid = sysUtl_gettid();
      len += snprintf(&(info->buf[len]), maxLen - len, "%d:", tid);
   }

   /*
    * Eventhough syslog will also log the time, this timestamp is more accurate.
    * But caller can always turn it off.
    */
   if ((info->logHeaderMask & BCMULOG_HDRMASK_TIMESTAMP) && (len < maxLen))
   {
      struct timespec ts = {0, 0};
      clock_gettime(CLOCK_MONOTONIC, &ts);
      len += snprintf(&(info->buf[len]), maxLen - len, "%lu.%03lu:",
                      ts.tv_sec%1000, ts.tv_nsec/1000000);
   }

   if ((info->logHeaderMask & BCMULOG_HDRMASK_LEVEL) && (len < maxLen))
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
         default:
            logLevelStr = "invalid-log-level";
            break;
         }
         len += snprintf(&(info->buf[len]), maxLen - len, "%s:", logLevelStr);
      }
   }

   if ((info->logHeaderMask & BCMULOG_HDRMASK_LOCATION) && (len < maxLen))
   {
      len += snprintf(&(info->buf[len]), maxLen - len, "%s:%u:",
                      info->funcName, info->lineNum);
   }

   if (len < maxLen)
   {
      vsnprintf(&(info->buf[len]), maxLen - len, pFmt, ap);
   }
}

void bcmuLog_setName(const char *name)
{
   if (name != NULL)
   {
      strncpy(appName, name, sizeof(appName)-1);
   }
}

void bcmuLog_setLevel(UINT32 level)
{
   ulogLevel = level;
}

UINT32 bcmuLog_getLevel(void)
{
   return ulogLevel;
}

void bcmuLog_setHdrMask(UINT32 mask)
{
   ulogHdrMask = mask;
}

UINT32 bcmuLog_getHdrMask(void)
{
   return ulogHdrMask;
}

void bcmuLog_setDestMask(UINT32 mask)
{
   ulogDestMask = mask;
}

static void setNameFromProc()
{
   ProcThreadInfo tInfo;
   int tid;

   tid = sysUtl_gettid();
   sysUtl_getThreadInfoFromProc(tid, &tInfo);
   strncpy(appName, tInfo.name, sizeof(appName)-1);
   return;
}

static void fillFormatInfo(char *buf,
                           UINT32 bufLen,
                           BcmuLogLevel level,
                           const char *funcName,
                           UINT32 lineNum,
                           BcmuLogFormatInfo *info)
{
   memset(info, 0, sizeof(BcmuLogFormatInfo));
   info->buf = buf;
   info->bufLen = bufLen;
   info->logLevel = level;
   info->logDestMask = ulogDestMask;
   info->logHeaderMask = ulogHdrMask;
   info->lineNum = lineNum;
   strncpy(info->funcName, funcName, sizeof(info->funcName)-1);
   if (appName[0] == '\0')
   {
      setNameFromProc();
   }
   strncpy(info->appName, appName, sizeof(info->appName)-1);
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
}

void ulog_log(BcmuLogLevel level, const char *funcName, UINT32 lineNum, const char *pFmt, ... )
{
   char logBuf[BCMULOG_MAX_LINE_LEN];
   BcmuLogFormatInfo info;
   va_list ap;

   if (level > ulogLevel)
   {
      return;
   }

   fillFormatInfo(logBuf, sizeof(logBuf), level, funcName, lineNum, &info);
   va_start(ap, pFmt);
   bcmuLog_formatLine(&info, pFmt, ap);
   va_end(ap);

   outputLogBuf(level, logBuf);
}
