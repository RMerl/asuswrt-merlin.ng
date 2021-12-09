/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2018:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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
    BCMULOG_LEVEL_DEBUG  = 7  /**< Message at debug level. */
 } BcmuLogLevel;

 #define BCMULOG_DEFAULT_LEVEL   BCMULOG_LEVEL_ERR

 #define bcmuLog_error(args...)  ulog_log(BCMULOG_LEVEL_ERR, __FUNCTION__, __LINE__, args)
 #define bcmuLog_notice(args...) ulog_log(BCMULOG_LEVEL_NOTICE, __FUNCTION__, __LINE__, args)
 #define bcmuLog_debug(args...)  ulog_log(BCMULOG_LEVEL_DEBUG, __FUNCTION__, __LINE__, args)


/** Set the logging level */
void bcmuLog_setLevel(UINT32 level);

/** Get the logging level */
UINT32 bcmuLog_getLevel(void);

/** Set the meta info on the log line, see HDRMASK_xxx */
void bcmuLog_setHdrMask(UINT32 mask);

/** Return the header mask, see HDRMASK_xxx */
UINT32 bcmuLog_getHdrMask(void);

/** Set where the log lines go. */
void bcmuLog_setDestMask(UINT32 mask);

/** set the app name on the log line.  Used with HDRMASK_APPNAME.
 *
 * bcmuLog will automatially get the app's name from /proc/self/comm.
 * However, this name is limited to 15 characters.  Use this function if your
 * app name is longer or you want to set a custom app name.
 */
void bcmuLog_setName(const char *name);


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
#define BCMULOG_DEFAULT_HDRMASK (BCMULOG_HDRMASK_APPNAME|BCMULOG_HDRMASK_LEVEL|BCMULOG_HDRMASK_LOCATION)


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
   char *buf;           /**< Buffer for the formatted log line. */
   UINT32 bufLen;       /**< Length of buffer */
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
void bcmuLog_formatLine(BcmuLogFormatInfo *info, const char *pFmt, va_list ap);

/** Do not call this function directly.  Use the bcmuLog_{error, notice, debug}
 *  macros instead.
 */
 void ulog_log(BcmuLogLevel level, const char *funcName, UINT32 lineNum, const char *pFmt, ... );


#if defined __cplusplus
};
#endif
#endif  /* __BCM_ULOG_H__ */
