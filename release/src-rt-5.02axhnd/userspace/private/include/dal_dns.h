/*
* <:copyright-BRCM:2006:proprietary:standard
* 
*    Copyright (c) 2006 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/

#ifndef __DAL_DNS_H__
#define __DAL_DNS_H__


/*!\file dal_dns.h
 * \brief Header file for the DNS functionsof the CMS DAL API.
 *  This is in the cms_dal library.
 */

#include "cms.h"


/** Return list of intfNames to be considered for the system default or
 *  "Active" DNS server.
 *
 * @param ipvx  (IN) If CMS_AF_SELECT_IPV4, return IPv4 DNS ifNameList.
 *                   If CMS_AF_SELECT_IPV6, return IPv6 DNS ifNameList.
 *                   No other values are allowed.
 * @param dnsIfNameList (OUT) Must be a buffer of [CMS_MAX_DNSIFNAME * CMS_IFNAME_LENGTH] bytes.
 *
 * @return CmsRet
 */
CmsRet dalDns_getIpvxDnsIfNameList(UINT32 ipvx, char *dnsIfNameList);
CmsRet dalDns_getIpvxDnsIfNameList_igd(UINT32 ipvx, char *dnsIfNameList);
CmsRet dalDns_getIpvxDnsIfNameList_dev2(UINT32 ipvx, char *dnsIfNameList);

#if defined(SUPPORT_DM_LEGACY98)
#define dalDns_getIpvxDnsIfNameList(v, a)  dalDns_getIpvxDnsIfNameList_igd((v), (a))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDns_getIpvxDnsIfNameList(v, a)  dalDns_getIpvxDnsIfNameList_igd((v), (a))
#elif defined(SUPPORT_DM_PURE181)
#define dalDns_getIpvxDnsIfNameList(v, a)  dalDns_getIpvxDnsIfNameList_dev2((v), (a))
#elif defined(SUPPORT_DM_DETECT)
#define dalDns_getIpvxDnsIfNameList(v, a)  (cmsMdm_isDataModelDevice2() ? \
                    dalDns_getIpvxDnsIfNameList_dev2((v), (a)) : \
                    dalDns_getIpvxDnsIfNameList_igd((v), (a)))
#endif




/** Set list of intfNames to be considered for the system default or
 *  "Active" DNS server.
 *
 * @param ipvx  (IN) If CMS_AF_SELECT_IPV4, set IPv4 DNS ifNameList.
 *                   If CMS_AF_SELECT_IPV6, set IPv6 DNS ifNameList.
 *                   No other values are allowed.
 * @param dnsIfNameList (IN) List of linux interface names
 *
 * @return CmsRet
 */
CmsRet dalDns_setIpvxDnsIfNameList(UINT32 ipvx, const char *dnsIfNameList);
CmsRet dalDns_setIpvxDnsIfNameList_igd(UINT32 ipvx, const char *dnsIfNameList);
CmsRet dalDns_setIpvxDnsIfNameList_dev2(UINT32 ipvx, const char *dnsIfNameList);

#if defined(SUPPORT_DM_LEGACY98)
#define dalDns_setIpvxDnsIfNameList(v, a)  dalDns_setIpvxDnsIfNameList_igd((v), (a))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDns_setIpvxDnsIfNameList(v, a)  dalDns_setIpvxDnsIfNameList_igd((v), (a))
#elif defined(SUPPORT_DM_PURE181)
#define dalDns_setIpvxDnsIfNameList(v, a)  dalDns_setIpvxDnsIfNameList_dev2((v), (a))
#elif defined(SUPPORT_DM_DETECT)
#define dalDns_setIpvxDnsIfNameList(v, a)  (cmsMdm_isDataModelDevice2() ? \
                    dalDns_setIpvxDnsIfNameList_dev2((v), (a)) : \
                    dalDns_setIpvxDnsIfNameList_igd((v), (a)))
#endif




/** Set static DNS servers of the specified type.
 *  To get static DNS servers, use qdmDns_getStaticIpvxDnsServersLocked.
 *
 * @param ipvx  (IN) If CMS_AF_SELECT_IPV4, set static IPv4 DNS servers
 *                   If CMS_AF_SELECT_IPV6, set static IPv6 DNS servers
 *                   No other values are allowed.
 * @param dnsPrimary   (IN) Primary static DNS server
 * @param dnsSecondary (IN) Secondary static DNS server
 *
 * @return CmsRet
 */
CmsRet dalDns_setStaticIpvxDnsServers(UINT32 ipvx, const char *dnsPrimary, const char *dnsSecondary);
CmsRet dalDns_setStaticIpvxDnsServers_igd(UINT32 ipvx, const char *dnsPrimary, const char *dnsSecondary);
CmsRet dalDns_setStaticIpvxDnsServers_dev2(UINT32 ipvx, const char *dnsPrimary, const char *dnsSecondary);

#if defined(SUPPORT_DM_LEGACY98)
#define dalDns_setStaticIpvxDnsServers(v, a, b)  dalDns_setStaticIpvxDnsServers_igd((v), (a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDns_setStaticIpvxDnsServers(v, a, b)  dalDns_setStaticIpvxDnsServers_igd((v), (a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define dalDns_setStaticIpvxDnsServers(v, a, b)  dalDns_setStaticIpvxDnsServers_dev2((v), (a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define dalDns_setStaticIpvxDnsServers(v, a, b)  (cmsMdm_isDataModelDevice2() ? \
                       dalDns_setStaticIpvxDnsServers_dev2((v), (a), (b)) : \
                       dalDns_setStaticIpvxDnsServers_igd((v), (a), (b)))
#endif




/** Delete the static DNS servers of the specified type
 *
 * @param ipvx  (IN) If CMS_AF_SELECT_IPV4, delete static IPv4 DNS servers
 *                   If CMS_AF_SELECT_IPV6, delete static IPv6 DNS servers
 *                   If CMS_AF_SELECT_IPVx, delete all static IPv4 and IPv6
 *                   DNS servers.
 */
void dalDns_deleteAllStaticIpvxDnsServers(UINT32 ipvx);
void dalDns_deleteAllStaticIpvxDnsServers_igd(UINT32 ipvx);
void dalDns_deleteAllStaticIpvxDnsServers_dev2(UINT32 ipvx);

#if defined(SUPPORT_DM_LEGACY98)
#define dalDns_deleteAllStaticIpvxDnsServers(v)  dalDns_deleteAllStaticIpvxDnsServers_igd((v))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDns_deleteAllStaticIpvxDnsServers(v)  dalDns_deleteAllStaticIpvxDnsServers_igd((v))
#elif defined(SUPPORT_DM_PURE181)
#define dalDns_deleteAllStaticIpvxDnsServers(v)  dalDns_deleteAllStaticIpvxDnsServers_dev2((v))
#elif defined(SUPPORT_DM_DETECT)
#define dalDns_deleteAllStaticIpvxDnsServers(v)  (cmsMdm_isDataModelDevice2() ? \
                          dalDns_deleteAllStaticIpvxDnsServers_dev2((v)) : \
                          dalDns_deleteAllStaticIpvxDnsServers_igd((v)))
#endif




/** Set the IPv6 DNS info.  This is for Hybrid and Pure181 IPv6.
 *
 * @param webVar    (IN) wanVar contain all the ipv6 dns pamam
 *
 * @return CmsRet enum.
 */
CmsRet dalDns_setIpv6DnsInfo_dev2(const WEB_NTWK_VAR *webVar);


/** Set ipv6 dns info.  This is used by deprecated proprietary Broadcom IPv6 code.
 *
 * @param dns6Type   (IN) The system ipv6 dns server type, either static or dhcp.
 * @param dns6Ifc    (IN) The broadcom interface name
 * @param dns6Pri    (IN) The static primary dns server when dns6Type is static.
 * @param dns6Sec    (IN) The static secondary dns server when dns6Type is static.
 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_setDns6Info_igd(char *dns6Type, char *dns6Ifc, char *dns6Pri, char *dns6Sec);


#endif  /* __DAL_DNS_H__ */
