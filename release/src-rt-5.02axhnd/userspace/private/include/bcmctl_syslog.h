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
*      Broadcom Control Layer syslog utility definitions and implementations.
*      Include this header file in the applicable Control Layer API
*      implementation.
*
*****************************************************************************/

#ifndef BCMCTL_SYSLOG_H
#define BCMCTL_SYSLOG_H


#include "os_defs.h"
#include "bcmctl_syslogdefs.h"


#if defined(BCMCTL_SYSLOG_SUPPORTED)

#define IMPL_setSyslogLevel(module) \
  int module##_setSyslogLevel(BcmCtlSysLogLevel_t logLevel) \
  { \
      return bcmCtl_setSyslogLevel(logLevel); \
  }

#define IMPL_getSyslogLevel(module) \
  BcmCtlSysLogLevel_t module##_getSyslogLevel(void) \
  { \
      return bcmCtl_getSyslogLevel(); \
  }

#define IMPL_isSyslogLevelEnabled(module) \
  int module##_isSyslogLevelEnabled(BcmCtlSysLogLevel_t logLevel) \
  { \
      return bcmCtl_isSyslogLevelEnabled(logLevel); \
  }

#define IMPL_isSyslogEnabled(module) \
  int module##_isSyslogEnabled(void) \
  { \
      return bcmCtl_isSyslogEnabled(); \
  }

#define IMPL_setSyslogMode(module) \
  void module##_setSyslogMode(int mode) \
  { \
      return bcmCtl_setSyslogMode(mode); \
  }


static BcmCtlSysLogLevel_t ctlSysLogLevel = BCMCTL_SYSLOG_ERR;
static UBOOL8 ctlSysLogMode = 0;


static inline int bcmCtl_setSyslogLevel(BcmCtlSysLogLevel_t logLevel)
{
    if (logLevel > BCMCTL_SYSLOG_MAX)
    {
        printf("Invalid log Level: %d (allowed values are 0 to %d)",
          logLevel, BCMCTL_SYSLOG_MAX);
        return -EINVAL;
    }

    ctlSysLogLevel = logLevel;

    return 0;
}

static inline BcmCtlSysLogLevel_t bcmCtl_getSyslogLevel(void)
{
    return ctlSysLogLevel;
}

static inline int bcmCtl_isSyslogLevelEnabled(BcmCtlSysLogLevel_t logLevel)
{
    return ((logLevel <= ctlSysLogLevel) ? 1 : 0);
}

static inline void bcmCtl_setSyslogMode(int mode)
{
    ctlSysLogMode = (mode == 0) ? 0 : 1;
}

static inline int bcmCtl_isSyslogEnabled(void)
{
    return ((ctlSysLogMode == 1) ? 1 : 0);
}

#endif /* BCMCTL_SYSLOG_SUPPORTED */

#endif /* BCMCTL_SYSLOG_H */
