/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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

#ifndef __CMS_CLI_H__
#define __CMS_CLI_H__


/*!\file cms_cli.h
 * \brief Header file for the Command Line Interface library.
 */

#include "cms_eid.h"
#include "cms_dal.h"


/** Max number of failed authentications before we insert a delay penalty on the user. */
#define AUTH_NUM_MAX     3


/** Max number of seconds to wait for the MDM lock */
#define CLI_LOCK_TIMEOUT  (6 * MSECS_IN_SEC)


/** Print a message identifying the modem.
 */
void cmsCli_printWelcomeBanner(void);


/** Main entry point into the CLI library.
 *
 * @param msgHandle (IN) message handle that was initialized by the caller.
 *                       Must be given to the CLI library so it can send messages out.
 * @param exitOnIdleTimeout (IN) The amount of time, in seconds, of idle-ness before
 *                               timing out.
 */
void cmsCli_run(void *msgHandle, UINT32 exitOnIdleTimeout);


/** Get username and password from the user and authenticate.
 *  This function will keep trying to authenticate the user until success,
 *  exit-on-idle timeout, or signal interrupt.
 *
 * @param accessMode (IN) an enum describing where the access is coming from.
 * @param exitOnIdleTimeout (IN) The amount of time, in seconds, of idle-ness before
 *                               timing out.
 *
 * @return CMSRET_SUCCESS if authentication was successful.
 *         CMSRET_TIMED_OUT if user stops typing for the exit-on-idle number of seconds.
 *         CMSRET_OP_INTR if input was interrupted by a signal.
 *
 */
CmsRet cmsCli_authenticate(NetworkAccessMode accessMode, UINT32 exitOnIdleTimeout);


/** Use this to specify application data
 *
 * @param appName (IN) name of application
 * @param ipAddr  (IN) IP used for connection
 * @param curUser (IN) current user of application
 * @param appPort (IN) port of application
 *
 */
void cmsCli_setAppData(char *appName, char *ipAddr, char *curUser, UINT16 appPort);


#endif  /* __CMS_CLI_H__ */
