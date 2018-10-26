/***********************************************************************
 *
 *  Copyright (c) 2014  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2014:proprietary:standard

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

#ifndef _QDM_SYSTEM_H_
#define _QDM_SYSTEM_H_


/*!\file qdm_system.h
 * \brief This file contains declarations for system (device) information
 *
 */

#include "cms.h"

/** Get Device Info configuration
 *
 * @param  oui(OUT)  A pointer to string buffer (8 character) to hold the device's manufacturer OUI
 * @param  serial(OUT)  A pointer of string buffer (64 characters) to hold the device's serial number
 * @param  productClass(OUT)  A pointer of string buffer (256 characters) to hold the device's product class
 * @return CmsRet enum.
 */
CmsRet qdmSystem_getDeviceInfoLocked_dev2(char *oui, char *serial, char *productClass);
CmsRet qdmSystem_getDeviceInfoLocked_igd(char *oui, char *serial, char *productClass);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmSystem_getDeviceInfoLocked(a,b,c)    qdmSystem_getDeviceInfoLocked_igd((a),(b),(c))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmSystem_getDeviceInfoLocked(a,b,c)    qdmSystem_getDeviceInfoLocked_igd((a),(b),(c))
#elif defined(SUPPORT_DM_PURE181)
#define qdmSystem_getDeviceInfoLocked(a,b,c)    qdmSystem_getDeviceInfoLocked_dev2((a),(b),(c))
#elif defined(SUPPORT_DM_DETECT)
#define qdmSystem_getDeviceInfoLocked(a,b,c)    (cmsMdm_isDataModelDevice2() ? \
                                                 qdmSystem_getDeviceInfoLocked_dev2((a),(b),(c)): \
                                                 qdmSystem_getDeviceInfoLocked_igd((a),(b),(c)))
#endif

#endif /* _QDM_SYSTEM_H_ */



