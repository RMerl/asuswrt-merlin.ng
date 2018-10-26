/***********************************************************************
 *
 *  Copyright (c) 2012  Broadcom Corporation
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

#ifndef _QDM_IPV6_H_
#define _QDM_IPV6_H_


/*!\file qdm_ipv6.h
 * \brief This file contains declarations for IPv6 query functions.
 *
 */

#ifdef SUPPORT_IPV6
#include "cms.h"

/** Get the IPv6 prefix from the full path name.
 *
 * @param mdmPath    (IN) the full path name.
 * @param prefix     (OUT) On success, this buffer will be filled with
 *                         the IPv6 prefix.  The buffer must be at least
 *                         CMS_IPADDR_LENGTH bytes long.
 *
 * @return CmsRet enum.
 */
CmsRet qdmIpv6_fullPathToPefixLocked_dev2(const char *mdmPath, char *prefix);


/** Get the IPv6 ULA address of a LAN interface.
 * 
 * @param ifname     (IN)  The LAN interface name.
 * @param addr       (OUT) The IPv6 ULA address.
 *
 * @return CmsRet enum.
 */
CmsRet qdmIpv6_getLanULAAddr6(const char *ifname, char *addr);
CmsRet qdmIpv6_getLanULAAddr6_igd(const char *ifname, char *addr);
CmsRet qdmIpv6_getLanULAAddr6_dev2(const char *ifname, char *addr);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpv6_getLanULAAddr6(i, a)  qdmIpv6_getLanULAAddr6_igd((i), (a))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpv6_getLanULAAddr6(i, a)  qdmIpv6_getLanULAAddr6_dev2((i), (a))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpv6_getLanULAAddr6(i, a)  qdmIpv6_getLanULAAddr6_dev2((i), (a))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpv6_getLanULAAddr6(i, a)  (cmsMdm_isDataModelDevice2() ? \
                                    qdmIpv6_getLanULAAddr6_dev2((i), (a)) : \
                                    qdmIpv6_getLanULAAddr6_dev2((i), (a)))
#endif


/** Get the ipv6 dns info
 * 
 * @param dns6Type   (OUT) The system ipv6 dns server type, either static or dhcp.
 * @param dns6Ifc    (OUT) The broadcom interface name
 * @param dns6Pri    (OUT) The primary dns server, either statically specified or dhcp assigned.
 * @param dns6Sec    (OUT) The secondary dns server, either statically specified or dhcp assigned.
 *
 * @return CmsRet enum.
 */
CmsRet qdmIpv6_getDns6Info(char *dns6Type, char *dns6Ifc, char *dns6Pri, char *dns6Sec);
CmsRet qdmIpv6_getDns6Info_igd(char *dns6Type, char *dns6Ifc, char *dns6Pri, char *dns6Sec);
CmsRet qdmIpv6_getDns6Info_dev2(char *dns6Type, char *dns6Ifc, char *dns6Pri, char *dns6Sec);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpv6_getDns6Info(t, i, p, s)  qdmIpv6_getDns6Info_igd((t), (i), (p), (s))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpv6_getDns6Info(t, i, p, s)  qdmIpv6_getDns6Info_dev2((t), (i), (p), (s))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpv6_getDns6Info(t, i, p, s)  qdmIpv6_getDns6Info_dev2((t), (i), (p), (s))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpv6_getDns6Info(t, i, p, s)  (cmsMdm_isDataModelDevice2() ? \
                                    qdmIpv6_getDns6Info_dev2((t), (i), (p), (s)) : \
                                    qdmIpv6_getDns6Info_dev2((t), (i), (p), (s)))
#endif


/** Update Dhcp6s info to webvar
 * 
 * @param (OUT) pointer to the webvar struct to fill out.
 *
 */
void qdmIpv6_getDhcp6sInfo(UBOOL8 *stateful, char *intfIDStart, char *intfIDEnd, SINT32 *leasedTime);
void qdmIpv6_getDhcp6sInfo_igd(UBOOL8 *stateful, char *intfIDStart, char *intfIDEnd, SINT32 *leasedTime);
void qdmIpv6_getDhcp6sInfo_dev2(UBOOL8 *stateful, char *intfIDStart, char *intfIDEnd, SINT32 *leasedTime);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpv6_getDhcp6sInfo(s, b, e, t)  qdmIpv6_getDhcp6sInfo_igd((s), (b), (e), (t))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpv6_getDhcp6sInfo(s, b, e, t)  qdmIpv6_getDhcp6sInfo_dev2((s), (b), (e), (t))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpv6_getDhcp6sInfo(s, b, e, t)  qdmIpv6_getDhcp6sInfo_dev2((s), (b), (e), (t))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpv6_getDhcp6sInfo(s, b, e, t)  (cmsMdm_isDataModelDevice2() ? \
                                            qdmIpv6_getDhcp6sInfo_dev2((s), (b), (e), (t)) : \
                                            qdmIpv6_getDhcp6sInfo_dev2((s), (b), (e), (t)))
#endif


/** Check wheter dhcp6s is enabled
 * 
 * @return enable flag.
 *
 */
UBOOL8 qdmIpv6_isDhcp6sEnabled(void);
UBOOL8 qdmIpv6_isDhcp6sEnabled_igd(void);
UBOOL8 qdmIpv6_isDhcp6sEnabled_dev2(void);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpv6_isDhcp6sEnabled()  qdmIpv6_isDhcp6sEnabled_igd()
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpv6_isDhcp6sEnabled()  qdmIpv6_isDhcp6sEnabled_dev2()
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpv6_isDhcp6sEnabled()  qdmIpv6_isDhcp6sEnabled_dev2()
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpv6_isDhcp6sEnabled()  (cmsMdm_isDataModelDevice2() ? \
                                            qdmIpv6_isDhcp6sEnabled_dev2() : \
                                            qdmIpv6_isDhcp6sEnabled_dev2())
#endif


/** Update ULA info to webvar
 * 
 * @param (OUT) pointer to the webvar struct to fill out.
 *
 */
void qdmIpv6_getRadvdUlaInfo(UBOOL8 *enblUla, UBOOL8 *randomUla, char *ulaPrefix, 
                             SINT32 *plt, SINT32 *vlt);
void qdmIpv6_getRadvdUlaInfo_igd(UBOOL8 *enblUla, UBOOL8 *randomUla, char *ulaPrefix, 
                             SINT32 *plt, SINT32 *vlt);
void qdmIpv6_getRadvdUlaInfo_dev2(UBOOL8 *enblUla, UBOOL8 *randomUla, char *ulaPrefix, 
                             SINT32 *plt, SINT32 *vlt);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpv6_getRadvdUlaInfo(e, r, p, b, v)  qdmIpv6_getRadvdUlaInfo_igd((e), (r), (p), (b), (v))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpv6_getRadvdUlaInfo(e, r, p, b, v)  qdmIpv6_getRadvdUlaInfo_dev2((e), (r), (p), (b), (v))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpv6_getRadvdUlaInfo(e, r, p, b, v)  qdmIpv6_getRadvdUlaInfo_dev2((e), (r), (p), (b), (v))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpv6_getRadvdUlaInfo(e, r, p, b, v)  (cmsMdm_isDataModelDevice2() ? \
                                            qdmIpv6_getRadvdUlaInfo_dev2((e), (r), (p), (b), (v)) : \
                                            qdmIpv6_getRadvdUlaInfo_dev2((e), (r), (p), (b), (v)))
#endif


/** Check wheter radvd is enabled
 * 
 * @return enable flag.
 *
 */
UBOOL8 qdmIpv6_isRadvdEnabled(void);
UBOOL8 qdmIpv6_isRadvdEnabled_igd(void);
UBOOL8 qdmIpv6_isRadvdEnabled_dev2(void);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpv6_isRadvdEnabled()  qdmIpv6_isRadvdEnabled_igd()
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpv6_isRadvdEnabled()  qdmIpv6_isRadvdEnabled_dev2()
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpv6_isRadvdEnabled()  qdmIpv6_isRadvdEnabled_dev2()
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpv6_isRadvdEnabled()  (cmsMdm_isDataModelDevice2() ? \
                                            qdmIpv6_isRadvdEnabled_dev2() : \
                                            qdmIpv6_isRadvdEnabled_dev2())
#endif




/** Get ipv6 site prefix info  TR-98 only
 * 
 * @param sitePrefixType (OUT) The site prefix type, either static or delegated.
 * @param pdWanIfc (OUT) Full path name of the wan interface selected for acquiring site prefix.
 * @param sitePrefix (OUT) The site prefix, either statically specified or dynamically delegated.
 *
 * @return CmsRet enum.
 */
CmsRet qdmIpv6_getSitePrefixInfo(char *sitePrefixType, char *pdWanIfc, char *sitePrefix);
CmsRet qdmIpv6_getSitePrefixInfo_igd(char *sitePrefixType, char *pdWanIfc, char *sitePrefix);
CmsRet qdmIpv6_getSitePrefixInfo_dev2(char *sitePrefixType, char *pdWanIfc, char *sitePrefix);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpv6_getSitePrefixInfo(t, i, p)  qdmIpv6_getSitePrefixInfo_igd((t), (i), (p))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpv6_getSitePrefixInfo(t, i, p)  qdmIpv6_getSitePrefixInfo_dev2((t), (i), (p))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpv6_getSitePrefixInfo(t, i, p)  qdmIpv6_getSitePrefixInfo_dev2((t), (i), (p))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpv6_getSitePrefixInfo(t, i, p)  (cmsMdm_isDataModelDevice2() ? \
                                       qdmIpv6_getSitePrefixInfo_dev2((t), (i), (p)) : \
                                       qdmIpv6_getSitePrefixInfo_dev2((t), (i), (p)))
#endif

/** Get ipv6 ip interface prefix from ifName. Due to the data model difference, this function is for TR-181 only and
 * is similar to qdmIpv6_getSitePrefixInfo in TR-98.
 * 
 * @param ifName     (IN) The ip interface name
 * @param orgin      (IN) Ip Interface prefix orgin.
 * @param staticType (IN) Prefix static type.
 * @param prefix     (OUT) The prefix of the ip interface if return is CMSRET_SUCCESS.
 *
 * @return CmsRet enum.
 */
CmsRet qdmIpv6_getIpPrefixInfo_dev2(const char *ifName, const char *origin, const char *staticType, char *prefix, UINT32 prefixLen);

#endif  /* SUPPORT_IPV6 */

#endif /* _QDM_IPV6_H_ */
