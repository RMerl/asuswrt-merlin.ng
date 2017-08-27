/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2013:proprietary:standard

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


#include "cms.h"
#include "cms_mdm.h"
#include "cms_obj.h"
#include "cms_qdm.h"
#include "cms_util.h"


#ifndef _QDM_DHCPV4_H_
#define _QDM_DHCPV4_H_


/*!\file qdm_dhcpv4.h
 * \brief This file contains declarations for DHCPv4related functions.
 *
 */


/** Get info about the dhcpv4 relay for a specific IP.Interface.
 *
 * @param ipIntfFullPath (IN)  fullpath to IP.Interface being queried
 * @param enabled        (OUT) If not NULL, will be filled in with whether
 *                             relay server is enabled
 * @param relayServerIpAddr (OUT) If not NULL, will be filled in with the
 *                             IPv4 address of relay server.  Caller must
 *                             provide a buffer big enough to hold IPv4 Addr.
 *
 * @return CMSRET_SUCCESS if a relay entry exists for ipIntfFullPath.
 *         Otherwise, return some error code.
 */
CmsRet qdmDhcpv4Relay_getInfo_dev2(const char *ipIntfFullPath,
                                   UBOOL8 *enabled,
                                   char *relayServerIpAddr);


/** Get DHCP sent options information 
 *
 * @param ipIntfFullPath (IN)  Full path of DHCPv4 client IP interface
 * @param tagNum         (IN)  DHCP option tag (60, 61, 77, 125).
 * @param option         (OUT) DHCP option value.
 * @param openLen        (IN)  max lenght of DHCP option value.
 *
 */
CmsRet qdmDhcpv4Client_getSentOption_dev2(const char *ipIntfFullPath,
                                          UINT32 tagNum,
                                          char *option,
                                          UINT32 optionLen);


/** Get DHCP request options information 
 *
 * @param ipIntfFullPath (IN)  Full path of DHCPv4 client IP interface
 * @param tagNum         (IN)  DHCP option tag (50, 51).
 * @param option         (OUT) DHCP option value.
 * @param openLen        (IN)  max lenght of DHCP option value.
 *
 */
CmsRet qdmDhcpv4Client_getReqOption_dev2(const char *ipIntfFullPath,
                                          UINT32 tagNum,
                                          char *option,
                                          UINT32 optionLen);


#endif /* _QDM_DHCPV4_H_ */



