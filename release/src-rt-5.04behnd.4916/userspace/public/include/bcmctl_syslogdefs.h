/***********************************************************************
 *
 * Copyright (c) 2019  Broadcom
 * All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
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
 * :>
 *
 ************************************************************************/

/*****************************************************************************
*    Description:
*
*      Broadcom Control Layer syslog utility definitions.
*
*****************************************************************************/

#ifndef BCMCTL_SYSLOGDEFS_H
#define BCMCTL_SYSLOGDEFS_H

/*
 * The syslog function for Broadcom Control Layer APIs is disabled by default.
 * Uncomment the following flag to enable it.
 * openlog() and closelog() are optional. An application that needs additional
 * options in openlog() may invoke it on its own before syslog() is used.
 */

/* #define BCMCTL_SYSLOG_SUPPORTED */

#if defined(BCMCTL_SYSLOG_SUPPORTED)

#include <syslog.h>


typedef enum
{
    BCMCTL_SYSLOG_ERR = LOG_ERR,
    BCMCTL_SYSLOG_NOTICE = LOG_NOTICE,
    BCMCTL_SYSLOG_INFO = LOG_INFO,
    BCMCTL_SYSLOG_DEBUG = LOG_DEBUG,
    BCMCTL_SYSLOG_MAX = BCMCTL_SYSLOG_DEBUG
} BcmCtlSysLogLevel_t;


#define DECLARE_setSyslogLevel(module) \
  int module##_setSyslogLevel(BcmCtlSysLogLevel_t logLevel);

#define DECLARE_getSyslogLevel(module) \
  BcmCtlSysLogLevel_t module##_getSyslogLevel(void);

#define DECALRE_isSyslogLevelEnabled(module) \
  int module##_isSyslogLevelEnabled(BcmCtlSysLogLevel_t logLevel);

#define DECLARE_setSyslogMode(module) \
  void module##_setSyslogMode(int mode);

#define DECLARE_isSyslogEnabled(module) \
  int module##_isSyslogEnabled(void);

#define BCMCTL_SYSLOGCODE(module, level, fmt, arg...) \
  if (module##_isSyslogEnabled() && module##_isSyslogLevelEnabled(level)) \
  { \
    syslog(level, "[%s] %-10s, %d: " fmt, #module, __FUNCTION__,  \
      __LINE__, ##arg); \
  }
#else
#define BCMCTL_SYSLOGCODE(module, level, fmt, arg...)
#endif /* BCMCTL_SYSLOG_SUPPORTED */


#endif /* BCMCTL_SYSLOGDEFS_H */
