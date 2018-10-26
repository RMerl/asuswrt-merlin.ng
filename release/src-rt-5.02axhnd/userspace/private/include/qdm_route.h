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
#include "cms_qdm.h"
#include "cms_util.h"


#ifndef _QDM_ROUTE_H_
#define _QDM_ROUTE_H_


/*!\file qdm_route.h
 * \brief This file contains declarations for routing related functions.
 *
 */




/** Get the active system default gateway's interface name
 *
 * @param gwIfName  (OUT)  gwIfName must point to a buffer of at least
 *                         CMS_IFNAME_LENGTH bytes long
 *
 * @return CmsRet enum
 */
CmsRet qdmRt_getActiveDefaultGatewayLocked(char *gwIfName);
CmsRet qdmRt_getActiveDefaultGatewayLocked_igd(char *gwIfName);
CmsRet qdmRt_getActiveDefaultGatewayLocked_dev2(char *gwIfName);


#if defined(SUPPORT_DM_LEGACY98)
#define qdmRt_getActiveDefaultGatewayLocked(g)  qdmRt_getActiveDefaultGatewayLocked_igd((g))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmRt_getActiveDefaultGatewayLocked(g)  qdmRt_getActiveDefaultGatewayLocked_igd((g))
#elif defined(SUPPORT_DM_PURE181)
#define qdmRt_getActiveDefaultGatewayLocked(g)  qdmRt_getActiveDefaultGatewayLocked_dev2((g))
#elif defined(SUPPORT_DM_DETECT)
#define qdmRt_getActiveDefaultGatewayLocked(g)  (cmsMdm_isDataModelDevice2() ? \
                          qdmRt_getActiveDefaultGatewayLocked_dev2((g)) : \
                          qdmRt_getActiveDefaultGatewayLocked_igd((g)))
#endif




/** Get the active system default gateway's IP address
 *
 * @param gwIpAddr  (OUT)  gwIpAddr must point to a buffer of at least
 *                         CMS_IPADDR_LENGTH bytes long
 *
 * @return CmsRet enum
 */

CmsRet qdmRt_getDefaultGatewayIPLocked(char *gwIpAddr);
CmsRet qdmRt_getDefaultGatewayIPLocked_igd(char *gwIpAddr);
CmsRet qdmRt_getDefaultGatewayIPLocked_dev2(char *gwIpAddr);


#if defined(SUPPORT_DM_LEGACY98)
#define qdmRt_getDefaultGatewayIPLocked(g)  qdmRt_getDefaultGatewayIPLocked_igd((g))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmRt_getDefaultGatewayIPLocked(g)  qdmRt_getDefaultGatewayIPLocked_igd((g))
#elif defined(SUPPORT_DM_PURE181)
#define qdmRt_getDefaultGatewayIPLocked(g)  qdmRt_getDefaultGatewayIPLocked_dev2((g))
#elif defined(SUPPORT_DM_DETECT)
#define qdmRt_getDefaultGatewayIPLocked(g)  (cmsMdm_isDataModelDevice2() ? \
                                             qdmRt_getDefaultGatewayIPLocked_dev2((g)) : \
                                             qdmRt_getDefaultGatewayIPLocked_igd((g)))
#endif




/** Get the IPv4 address of the default gateway of the IP.Interface fullpath.
 *
 * @param ipIntfFullPath (IN) Fullpath of the IP.Interface
 * @param gwIpAddr      (OUT) Gateway IPv4 address for the specified IP.Interface.
 *                            Must point to buffer of at least CMS_IPADDR_LENGTH bytes
 */
CmsRet qdmRt_getGatewayIpv4AddrByFullPathLocked_dev2(const char *ipIntfFullPath, char *gwIpAddr);


/** Get the IPv6 address of the default gateway by interface name.
 *
 * @param ifname        (IN)  gateway interface name
 * @param gwIpAddr      (OUT) Gateway IPv6 address for the specified interface name.
 *                            Must point to buffer of at least CMS_IPADDR_LENGTH bytes
 * @param origin        (OUT) If not NULL, the origin of the gwIpAddr will
 *                            be filled in.  Buffer should be at least 32 bytes.
 */
CmsRet qdmRt_getGatewayIpv6AddrByIfNameLocked_dev2(const char *ifname,
                                                   char *gwIpAddr,
                                                   char *origin);


/** Get the IPv6 address of the default gateway of the IP.Interface fullpath.
 *
 * @param ipIntfFullPath (IN) Fullpath of the IP.Interface
 * @param gwIpAddr      (OUT) Gateway IPv6 address for the specified IP.Interface.
 *                            Must point to buffer of at least CMS_IPADDR_LENGTH bytes
 * @param origin        (OUT) If not NULL, the origin of the gwIpAddr will
 *                            be filled in.  Buffer should be at least 32 bytes.
 */
CmsRet qdmRt_getGatewayIpv6AddrByFullPathLocked_dev2(const char *ipIntfFullPath,
                                                     char *gwIpAddr,
                                                     char *origin);




/** Get the system ipv6 default gateway interface.
 * 
 * @param gwIfc    (OUT) The gateway interface name.  Buffer must be at least
 *                       CMS_IFNAME_LENGTH bytes.
 * @param gwIpAddr (OUT) The gateway IPv6 address.  Buffer must be at least
 *                       CMS_IPADDR_LENGTH bytes.
 *
 * @return CmsRet enum.
 */
CmsRet qdmRt_getSysDfltGw6(char *gwIfc, char *gwIpAddr);
CmsRet qdmRt_getSysDfltGw6_igd(char *gwIfc, char *gwIpAddr);
CmsRet qdmRt_getSysDfltGw6_dev2(char *gwIfc, char *gwIpAddr);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmRt_getSysDfltGw6(i, g)  qdmRt_getSysDfltGw6_igd((i), (g))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmRt_getSysDfltGw6(i, g)  qdmRt_getSysDfltGw6_dev2((i), (g))
#elif defined(SUPPORT_DM_PURE181)
#define qdmRt_getSysDfltGw6(i, g)  qdmRt_getSysDfltGw6_dev2((i), (g))
#elif defined(SUPPORT_DM_DETECT)
#define qdmRt_getSysDfltGw6(i, g)  (cmsMdm_isDataModelDevice2() ? \
                                    qdmRt_getSysDfltGw6_dev2((i), (g)) : \
                                    qdmRt_getSysDfltGw6_dev2((i), (g)))
#endif




#endif /* _QDM_ROUTE_H_ */



