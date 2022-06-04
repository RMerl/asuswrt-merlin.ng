/******************************************************************************
 *
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom
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
 *
******************************************************************************/
#ifndef __CMS_ACTIONLOG_H__
#define __CMS_ACTIONLOG_H__

#if defined __cplusplus
extern "C" {
#endif

/*!\file cms_actionlog.h
 * \brief Public header file for Broadcom CPE Management System Action
 *  Logging API.
 * Applications which need to call action logging API functions must
 * include this file.
 */

#define CAL_LOG_FILE    "/var/cal.log"

/*!\enum CmsLogDestination
 * \brief identifiers for message logging destinations.
 */
typedef enum
{
   LOG_SRC_LIBRARY  = 1,  /**< Message from library. */
   LOG_SRC_NETLINK  = 2,  /**< Message from netlink. */
   LOG_SRC_COMMAND  = 3,  /**< Message from shell command. */
   LOG_SRC_IOCTL    = 4   /**< Message from ioctl call. */
} CmsActionLogSrc;


/** Maxmimu length of a single log line; messages longer than this are truncated. */
#define MAX_ACTIONLOG_LINE_LENGTH      512


/** Macros Definition.
 * Applications should use these macros for action logging, instead of
 * calling the callog_log function directly.
 */
#ifdef CMS_ACTION_LOG
#define calLog_library(args...)  callog_log((CmsActionLogSrc)LOG_SRC_LIBRARY, args)
#define calLog_shell(args...) callog_log((CmsActionLogSrc)LOG_SRC_COMMAND, args)
#else
#define calLog_library(args...)
#define calLog_shell(args...)
#endif


static inline void printlog( const char *buf )
{
   FILE *fd = fopen(CAL_LOG_FILE, "a+");

   if (fd == NULL)
   {
      fprintf(stderr, "callog error: cannot open %s", CAL_LOG_FILE);
      fflush(stderr);
   }
   else
   {
      fprintf(fd, "%s", buf);
      fclose(fd);
   }
}

/** Internal action log function; do not call this function directly.
 *
 * NOTE: Applications should NOT call this function directly from code.
 *       Use the macros defined in cms_actionlog.h, i.e.
 *       calLog_library, calLog_shell, calLog_ioctl.
 *
 * @param pFmt (IN) The message string.
 *
 */
static inline void callog_log(CmsActionLogSrc src __attribute__((unused)),
  const char *pFmt, ... )
{
   va_list              ap;
   char buf[MAX_ACTIONLOG_LINE_LENGTH] = {0};
   int len=0, maxLen;

   maxLen = sizeof(buf);

   va_start(ap, pFmt);

   if (len < maxLen)
   {
      maxLen -= len;
      vsnprintf(&buf[len], maxLen, pFmt, ap);
   }

   printlog(buf);

   va_end(ap);
}


#if defined __cplusplus
};
#endif
#endif /* __CMS_ACTIONLOG_H__ */
