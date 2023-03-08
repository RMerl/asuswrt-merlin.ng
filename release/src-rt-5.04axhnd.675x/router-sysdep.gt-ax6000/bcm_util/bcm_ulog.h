/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2018:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
:>
 *
************************************************************************/

#ifndef __BCM_ULOG_H__
#define __BCM_ULOG_H__

#if defined __cplusplus
extern "C" {
#endif


#include <stdarg.h>
#include <stdio.h>
#include "number_defs.h"

/*!\file bcm_ulog.h
 * \brief Broadcom Userspace Logging utility.
 *
 * Broadcom apps and daemons which do not want to have dependencies on CMS
 * can use this logging system instead of CMS logging.  They are very similar,
 * and CMS logging even uses this logging system for formatting the log line.
 *
 * Basically, your code can just call the various bcmuLog_error,
 * bcmuLog_notice, and bcmuLog_debug macros.  There is no need to call an
 * "init" function.  Optionally, you can modify the logging level, destination,
 * or the meta info in the log line by calling the various bcmuLog_set
 * functions.
 */

 /*!\enum BcmuLogLevel
  * \brief Logging levels.
  * These must be the same as LINUX syslog levels.
  */
 typedef enum
 {
    BCMULOG_LEVEL_ERR    = 3, /**< Message at error level. */
    BCMULOG_LEVEL_NOTICE = 5, /**< Message at notice level. */
    BCMULOG_LEVEL_DEBUG  = 7,  /**< Message at debug level. */
    BCMULOG_LEVEL_PERFTRACE  = 256  /**< Message at trace level. */
 } BcmuLogLevel;

 #define BCMULOG_DEFAULT_LEVEL   BCMULOG_LEVEL_ERR

 #define bcmuLog_error(args...)  ulog_log(BCMULOG_LEVEL_ERR, __FUNCTION__, __LINE__, args)
 #define bcmuLog_notice(args...) ulog_log(BCMULOG_LEVEL_NOTICE, __FUNCTION__, __LINE__, args)
 #define bcmuLog_debug(args...)  ulog_log(BCMULOG_LEVEL_DEBUG, __FUNCTION__, __LINE__, args)
 #define bcmuLog_perftrace_begin(args...)  ulog_trace(__FUNCTION__, __LINE__, ">>>", args)
 #define bcmuLog_perftrace_end(args...)  ulog_trace(__FUNCTION__, __LINE__, "<<<", args)


/** Set the app-wide logging level */
void bcmuLog_setLevel(UINT32 level);

/** Set the log level for a specific thread or eid.
 *  threadId and/or eid must be specified.
 *  If only threadId is specified, that is ok.
 *  If only eid is specified, then a EID to threadId mapping must already exist.
 *  If both threadId and eid are specified, then this call will also create
 *  a EID to threadId mapping.
 *  Internally, the thread specific log level is associated with the threadId.
 */
void bcmuLog_setLevelEx(UINT32 level, SINT32 threadId, UINT32 eid);

/** Get the app-wide logging level */
UINT32 bcmuLog_getLevel(void);

/** Get the log level for a specific thread or eid.
 *  The use of threadId and eid is the same as bcmuLog_setLevelEx().
 *  If no thread or eid specific setting is found, the app wide-setting is
 *  returned.
 */
UINT32 bcmuLog_getLevelEx(SINT32 threadId, UINT32 eid);


/** Set the meta info on the log line, see HDRMASK_xxx */
void bcmuLog_setHdrMask(UINT32 mask);

/** Set the header mask for a specific thread or eid.
 *  See comments above for setLevelEx.
 */
void bcmuLog_setHdrMaskEx(UINT32 mask, SINT32 threadId, UINT32 eid);

/** Return the header mask, see HDRMASK_xxx */
UINT32 bcmuLog_getHdrMask(void);

/** Get the header mask for a specific thread or eid.
 *  See comments above for getLevelEx.
 */
UINT32 bcmuLog_getHdrMaskEx(SINT32 threadId, UINT32 eid);


/** Set where the log lines go. */
void bcmuLog_setDestMask(UINT32 mask);

/** Set the destination mask for a specific thread or eid.
 *  See comments above for setLevelEx.
 */
void bcmuLog_setDestMaskEx(UINT32 mask, SINT32 threadId, UINT32 eid);

/** Get the destination mask */
UINT32 bcmuLog_getDestMask();

/** Get the destination mask for a specific thread or eid.
 *  See comments above for getLevelEx.
 */
UINT32 bcmuLog_getDestMaskEx(SINT32 threadId, UINT32 eid);


/** set the app name on the log line.  Used with HDRMASK_APPNAME.
 *
 * This call is optional.  If the app name has not been explicitly set,
 * bcmuLog will automatially get the app's name by calling
 * bcmuLog_getNameFromProc(). However, Linux limits the app name in
 * /proc/self/comm to 15 characters.  Whereas bcmuLog_setName() allows up to
 * BCMULOG_MAX_APP_NAME_LEN characters for the name.
 */
void bcmuLog_setName(const char *name);

/** set the app name from /proc entry. */
void bcmuLog_setNameFromProc();

/** Set the thread name for a specific thread or eid.
 *  See comments above for setLevelEx.
 */
void bcmuLog_setNameEx(const char *name, SINT32 threadId, UINT32 eid);

/** Get the app name */
const char *bcmuLog_getName(void);

/** Get the thread name for a specific thread or eid.
 *  See comments above for getLevelEx.
 */
const char *bcmuLog_getNameEx(SINT32 threadId, UINT32 eid);


/** Map EID to threadId.  Assumes one-to-one EID to threadId mapping.  Does not
 *  work if there are multiple threads or processes associated with a single
 *  EID, e.g. ppp.
 */
void bcmuLog_setEidToThreadId(UINT32 eid, SINT32 threadId);

SINT32 bcmuLog_getEidToThreadId(UINT32 eid);


/** Show application name in the log line. */
#define BCMULOG_HDRMASK_APPNAME    0x0001

/** Show log level in the log line. */
#define BCMULOG_HDRMASK_LEVEL      0x0002

/** Show timestamp in the log line. */
#define BCMULOG_HDRMASK_TIMESTAMP  0x0004

/** Show location (function name and line number) level in the log line. */
#define BCMULOG_HDRMASK_LOCATION   0x0008

/** Show thread id of calling thread.  For single threaded apps, thread id is
 * the same as process id. */
#define BCMULOG_HDRMASK_THREAD_ID  0x0010

/** Default header mask */
#define BCMULOG_DEFAULT_HDRMASK (BCMULOG_HDRMASK_APPNAME|BCMULOG_HDRMASK_LEVEL|BCMULOG_HDRMASK_TIMESTAMP|BCMULOG_HDRMASK_LOCATION)


/** Send log to stderr */
#define BCMULOG_DESTMASK_STDERR    0x0001

/** Send log to syslog */
#define BCMULOG_DESTMASK_SYSLOG    0x0002

/** CMS logging had concept of logging to telnet window.  support that? */
#define BCMULOG_DESTMASK_TELNET    0x0004

/** Send log to file (not supported yet.  maybe merge with action log?) */
#define BCMULOG_DESTMASK_FILE      0x0010

/** default destination mask */
#define BCMULOG_DEFAULT_DESTMASK   (BCMULOG_DESTMASK_STDERR)


/** Max length of the app name in the log line. */
#define BCMULOG_MAX_APP_NAME_LEN    64

/** Max length of the function name where the logging line is. */
#define BCMULOG_MAX_FUNC_NAME_LEN   128

/** Max length of log line. */
#define BCMULOG_MAX_LINE_LEN        512

/** All the info needed by the log line formatter. */
typedef struct bcm_ulog_format_info
{
   UINT32 logLevel;
   UINT32 logDestMask;
   UINT32 logHeaderMask;
   UINT32 lineNum;
   char funcName[BCMULOG_MAX_FUNC_NAME_LEN];
   char appName[BCMULOG_MAX_APP_NAME_LEN];
} BcmuLogFormatInfo;


/** Create a log line based on setting in BcmuLogFormatInfo.
 *
 * This function is shared by bcm_ulog and CMS log.
*/
void bcmuLog_formatLine(FILE *stream, BcmuLogFormatInfo *info, const char *pFmt, va_list ap);

void bcmuLog_formatPerfTrace(FILE *stream, BcmuLogFormatInfo *info, const char *anchor, const char *pFmt, va_list ap);

/** Do not call this function directly.  Use the bcmuLog_{error, notice, debug}
 *  macros instead.
 */
void ulog_log(BcmuLogLevel level, const char *funcName, UINT32 lineNum, const char *pFmt, ... );

void ulog_trace(const char *funcName, UINT32 lineNum, const char *anchor, const char *pFmt, ... );


#define BCMULOG_NUM_THREAD_SLOTS   4

typedef struct bcm_ulog_thread_info
{
   SINT32 threadId;   // only threadId as lookup key is supported.
   UINT32 eid;        // used for mapping EID to threadid.
   UINT32 value;      // logLevel, logDestMask, or logHdrMask;
   char   appName[BCMULOG_MAX_APP_NAME_LEN];  // more accurately: thread name
} BcmuLogThreadInfo;


#if defined __cplusplus
};
#endif
#endif  /* __BCM_ULOG_H__ */
