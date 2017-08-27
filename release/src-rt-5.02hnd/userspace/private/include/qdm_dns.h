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

#ifndef __QDM_DNS_H__
#define __QDM_DNS_H__

/*!\file QDM_dns.h
 * \brief DNS query functions
 *
 */

#include "cms.h"


/** Get the current system default DNS servers.  These are also called the
 *  "Active" DNS servers.  The "system default" or "active" DNS servers
 *  may be statically configured (higher priority) or dynamically obtained.
 *
 * @param ipvx  (IN) If CMS_AF_SELECT_IPV4, return IPv4 active servers.
 *                   If CMS_AF_SELECT_IPV6, return IPv6 active servers.
 *                   If CMS_AF_SELECT_IPVX, try to return IPv6 servers first,
 *                   but if not found, then try to return IPv4 servers.
 * @param dns1  (OUT) First dns ip address.  Must point to a buffer at
 *                    least CMS_IPADDR_LENGTH bytes long.
 * @param dns1  (OUT) Second dns ip address.  May be NULL, but if not NULL,
 *                    must point to a buffer at east CMS_IPADDR_LENGTH bytes long.
 */
void qdmDns_getActiveIpvxDnsIpLocked(UINT32 ipvx, char *dns1, char *dns2);
void qdmDns_getActiveIpvxDnsIpLocked_igd(UINT32 ipvx, char *dns1, char *dns2);
void qdmDns_getActiveIpvxDnsIpLocked_dev2(UINT32 ipvx, char *dns1, char *dns2);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmDns_getActiveIpvxDnsIpLocked(v, a, b)  qdmDns_getActiveIpvxDnsIpLocked_igd((v), (a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmDns_getActiveIpvxDnsIpLocked(v, a, b)  qdmDns_getActiveIpvxDnsIpLocked_igd((v), (a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define qdmDns_getActiveIpvxDnsIpLocked(v, a, b)  qdmDns_getActiveIpvxDnsIpLocked_dev2((v), (a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define qdmDns_getActiveIpvxDnsIpLocked(v, a, b)  (cmsMdm_isDataModelDevice2() ? \
                    qdmDns_getActiveIpvxDnsIpLocked_dev2((v), (a), (b)) : \
                    qdmDns_getActiveIpvxDnsIpLocked_igd((v), (a), (b)))
#endif




/** Get statically configured DNS Servers.
 *
 * @param ipvx  (IN) If CMS_AF_SELECT_IPV4, return IPv4 static addrs
 *                   If CMS_AF_SELECT_IPV6, return IPv6 static addrs.
 *                   If CMS_AF_SELECT_IPVX, return at most CMS_MAX_ACTIVE_DNS_IP
 *                                          IPv6 and IPv4 static addrs
 * @param staticDnsServers (OUT) Caller must provide a buffer of at least
 *                  (CMS_MAX_ACTIVE_DNS_IP * CMS_IPADDR_LENGTH) bytes long.
 *                  On successful return, will contain one or more IP addresses
 *                  separated by comma.
 *
 * @param TRUE if staticDnsServers contain one or more valid addresses.
 *        FALSE if staticDnsServers is empty string or 0.0.0.0
 *
 * @return TRUE if any statically configured DNS servers are found.
 */
UBOOL8 qdmDns_getStaticIpvxDnsServersLocked(UINT32 ipvx, char *staticDnsServers);
UBOOL8 qdmDns_getStaticIpvxDnsServersLocked_igd(UINT32 ipvx, char *staticDnsServers);
UBOOL8 qdmDns_getStaticIpvxDnsServersLocked_dev2(UINT32 ipvx, char *staticDnsServers);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmDns_getStaticIpvxDnsServersLocked(v, a)  qdmDns_getStaticIpvxDnsServersLocked_igd((v), (a))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmDns_getStaticIpvxDnsServersLocked(v, a)  qdmDns_getStaticIpvxDnsServersLocked_igd((v), (a))
#elif defined(SUPPORT_DM_PURE181)
#define qdmDns_getStaticIpvxDnsServersLocked(v, a)  qdmDns_getStaticIpvxDnsServersLocked_dev2((v), (a))
#elif defined(SUPPORT_DM_DETECT)
#define qdmDns_getStaticIpvxDnsServersLocked(v, a)  (cmsMdm_isDataModelDevice2() ? \
                    qdmDns_getStaticIpvxDnsServersLocked_dev2((v), (a)) : \
                    qdmDns_getStaticIpvxDnsServersLocked_igd((v), (a)))
#endif


#endif /* __QDM_DNS_H__ */

