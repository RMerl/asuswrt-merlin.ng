/***********************************************************************
 *
 * Copyright (c) 2019  Broadcom
 * All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
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
