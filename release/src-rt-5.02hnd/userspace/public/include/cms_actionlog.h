/******************************************************************************
 *  
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

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
******************************************************************************/
#ifndef __CMS_ACTIONLOG_H__
#define __CMS_ACTIONLOG_H__

#if defined __cplusplus
extern "C" {
#endif

/*!\file cms_actionlog.h
 * \brief Public header file for Broadcom DSL CPE Management System Action 
 *  Logging API.
 * Applications which need to call action logging API functions must
 * include this file.
 */

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


/** Internal action log function; do not call this function directly.
 *
 * NOTE: Applications should NOT call this function directly from code.
 *       Use the macros defined in cms_actionlog.h, i.e.
 *       calLog_library, calLog_shell, calLog_ioctl.
 *
 * @param pFmt (IN) The message string.
 *
 */
void callog_log(CmsActionLogSrc src, const char *pFmt, ...);

#if defined __cplusplus
};
#endif
#endif /* __CMS_ACTIONLOG_H__ */
