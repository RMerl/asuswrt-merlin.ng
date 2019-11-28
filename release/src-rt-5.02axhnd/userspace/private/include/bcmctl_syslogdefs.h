/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom
 *  All Rights Reserved
 *
<:label-BRCM:2018:proprietary:standard

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
