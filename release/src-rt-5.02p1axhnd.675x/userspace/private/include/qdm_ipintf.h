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
#include "cms.h"
#include "cms_mdm.h"
#include "cms_obj.h"
#include "cms_qdm.h"
#include "cms_util.h"

#ifndef _QDM_IP_INTF_H_
#define _QDM_IP_INTF_H_


/*!\file qdm_ipintf.h
 * \brief This file contains declarations for IP Interface related functions.
 *
 */



/** Get the default LAN side intf name (i.e. br0).
 *  Note this function does not indicate whether the intfName is UP or not.
 *
 * @param ifName (OUT) On success, filled in with the default LAN side intf name.
 *
 * @return CmsRet enum.
 */
CmsRet qdmIpIntf_getDefaultLanIntfNameLocked(char *ifName);
CmsRet qdmIpIntf_getDefaultLanIntfNameLocked_igd(char *ifName);
CmsRet qdmIpIntf_getDefaultLanIntfNameLocked_dev2(char *ifName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_getDefaultLanIntfNameLocked(i)   qdmIpIntf_getDefaultLanIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_getDefaultLanIntfNameLocked(i)   qdmIpIntf_getDefaultLanIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_getDefaultLanIntfNameLocked(i)   qdmIpIntf_getDefaultLanIntfNameLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_getDefaultLanIntfNameLocked(i)   (cmsMdm_isDataModelDevice2() ? \
                 qdmIpIntf_getDefaultLanIntfNameLocked_dev2((i)) : \
                 qdmIpIntf_getDefaultLanIntfNameLocked_igd((i)))
#endif



/** Find and return the IPv4 address associated with ipIntfName.
 *
 *  @param ipIntfName  (IN) Linux interface name
 *  @param ipAddress  (OUT) Buffer must be at least CMS_IPADDR_LENGTH bytes
 *
 *  @return CmsRet
 */
CmsRet qdmIpIntf_getIpv4AddressByNameLocked(const char *ipIntfName, char *ipv4Address);
CmsRet qdmIpIntf_getIpv4AddressByNameLocked_igd(const char *ipIntfName, char *ipv4Address);
CmsRet qdmIpIntf_getIpv4AddressByNameLocked_dev2(const char *ipIntfName, char *ipv4Address);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_getIpv4AddressByNameLocked(i, a)   qdmIpIntf_getIpv4AddressByNameLocked_igd((i), (a))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_getIpv4AddressByNameLocked(i, a)   qdmIpIntf_getIpv4AddressByNameLocked_igd((i), (a))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_getIpv4AddressByNameLocked(i, a)   qdmIpIntf_getIpv4AddressByNameLocked_dev2((i), (a))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_getIpv4AddressByNameLocked(i, a)   (cmsMdm_isDataModelDevice2() ? \
                 qdmIpIntf_getIpv4AddressByNameLocked_dev2((i), (a)) : \
                 qdmIpIntf_getIpv4AddressByNameLocked_igd((i), (a)))
#endif



/** Find either IPv6 or IPv4 address associated with ipIntfName.
 *  If both IPv6 and IPv4 addresses are present on the interface, the
 *  IPv6 one is returned first.
 *
 * @param ipvx  (IN) If CMS_AF_SELECT_IPV4, look for IPv4 addr on this intf
 *                   If CMS_AF_SELECT_IPV6, look for IPv6 addr on this intf
 *                   If CMS_AF_SELECT_IPVX, look for IPv6 addr on this intf first,
 *                   and if not found, look for IPv4 addr.  Test return
 *                   addr with cmsUtl_isValidIpv4Address().  Note that IPv6
 *                   addr will contain prefix; if you do not want prefix,
 *                   call cmsUtl_truncatePrefixFromIpv6AddrStr().
 *  @param ipIntfName  (IN) Linux interface name
 *  @param ipAddress  (OUT) Buffer must be at least CMS_IPADDR_LENGTH bytes
 *
 *  @return CmsRet
 */
CmsRet qdmIpIntf_getIpvxAddressByNameLocked(UINT32 ipvx,
                                            const char *ipIntfName,
                                            char *ipAddress);

CmsRet qdmIpIntf_getIpvxAddressByNameLocked_igd(UINT32 ipvx,
                                                const char *ipIntfName,
                                                char *ipAddress);

CmsRet qdmIpIntf_getIpvxAddressByNameLocked_dev2(UINT32 ipvx,
                                                 const char *ipIntfName,
                                                 char *ipAddress);


#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_getIpvxAddressByNameLocked(v, i, a)   qdmIpIntf_getIpvxAddressByNameLocked_igd((v), (i), (a))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_getIpvxAddressByNameLocked(v, i, a)   qdmIpIntf_getIpvxAddressByNameLocked_igd((v), (i), (a))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_getIpvxAddressByNameLocked(v, i, a)   qdmIpIntf_getIpvxAddressByNameLocked_dev2((v), (i), (a))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_getIpvxAddressByNameLocked(v, i, a)   (cmsMdm_isDataModelDevice2() ? \
                 qdmIpIntf_getIpvxAddressByNameLocked_dev2((v), (i), (a)) : \
                 qdmIpIntf_getIpvxAddressByNameLocked_igd((v), (i), (a)))
#endif




CmsRet qdmIpIntf_getIpv4AddrAndSubnetByNameLocked_dev2(const char *ipIntfName, char *ipAddress, char *subnetMask);
UBOOL8 qdmIpIntf_isStaticWanLocked_dev2(const char *ifName);

CmsRet qdmIpIntf_getIpv4AddrInfoByNameLocked(const char *ifName, char *ipAddress, char *subnetMask, UBOOL8 *isWan, UBOOL8 *isStatic);
CmsRet qdmIpIntf_getIpv4AddrInfoByNameLocked_igd(const char *ifName, char *ipAddress, char *subnetMask, UBOOL8 *isWan, UBOOL8 *isStatic);
CmsRet qdmIpIntf_getIpv4AddrInfoByNameLocked_dev2(const char *ifName, char *ipAddress, char *subnetMask, UBOOL8 *isWan, UBOOL8 *isStatic);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_getIpv4AddrInfoByNameLocked(a, b, c, d, e)   qdmIpIntf_getIpv4AddrInfoByNameLocked_igd((a), (b), (c), (d), (e))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_getIpv4AddrInfoByNameLocked(a, b, c, d, e)   qdmIpIntf_getIpv4AddrInfoByNameLocked_igd((a), (b), (c), (d), (e))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_getIpv4AddrInfoByNameLocked(a, b, c, d, e)   qdmIpIntf_getIpv4AddrInfoByNameLocked_dev2((a), (b), (c), (d), (e))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_getIpv4AddrInfoByNameLocked(a, b, c, d, e)   (cmsMdm_isDataModelDevice2() ? \
                 qdmIpIntf_getIpv4AddrInfoByNameLocked_dev2((a), (b), (c), (d), (e)) : \
                 qdmIpIntf_getIpv4AddrInfoByNameLocked_igd((a), (b), (c), (d), (e)))
#endif




/** Get the IPv4ServiceStatus or IPv6ServiceStatus string of the
 *  IP.Interface pointed to by ipIntfFullPath.
  * Only dev2 versions of this functions is needed right now.
  *
  * @param ipIntfFullPath (IN) FullPath to the IP.Interface object.
  * @param ipvx           (IN) selector for IPv4 or IPv6
  * @param statusBuf     (OUT) a buffer to hold the returned status value.
  * @param statusBufLen   (IN) Length of statusBuf.
  *
  * @return CmsRet.
  */
CmsRet qdmIpIntf_getIpvxServiceStatusFromFullPathLocked_dev2(
                                        const char *ipIntfFullPath,
                                        UINT32 ipvx,
                                        char *statusBuf, UINT32 statusBufLen);


/** Get the IPv4ServiceStatus or IPv6ServiceStatus string of the
 *  IP.Interface pointed to by ipIntfPathDesc.
  * Only dev2 versions of this functions is needed right now.
  *
  * @param ipIntfPathDesc (IN) the path descriptor to the IP.Interface object.
  * @param ipvx           (IN) selector for IPv4 or IPv6
  * @param statusBuf     (OUT) a buffer to hold the returned status value.
  * @param statusBufLen   (IN) Length of statusBuf.
  *
  * @return CmsRet.
  */
CmsRet qdmIpIntf_getIpvxServiceStatusFromPathDescLocked_dev2(
                                  const MdmPathDescriptor *ipIntfPathDesc,
                                  UINT32 ipvx,
                                  char *statusBuf, UINT32 statusBufLen);


/** wrapper function for qdmIpIntf_getIpvxServiceStatusFromPathDescLocked_dev2 */
CmsRet qdmIpIntf_getIpv4ServiceStatusFromPathDescLocked_dev2(
                                  const MdmPathDescriptor *ipIntfPathDesc,
                                  char *statusBuf, UINT32 statusBufLen);


/** wrapper function for qdmIpIntf_getIpvxServiceStatusFromPathDescLocked_dev2 */
CmsRet qdmIpIntf_getIpv6ServiceStatusFromPathDescLocked_dev2(
                                  const MdmPathDescriptor *ipIntfPathDesc,
                                  char *statusBuf, UINT32 statusBufLen);

CmsRet qdmIpIntf_getIpv6ServiceStatusFromFullPathLocked_dev2(
                   const char *fullPath, char *statusBuf, UINT32 statusBufLen);


/* bitmask to select which direction we are talking about.
 * Note that "WAN" means routed WAN, not bridged WAN.
 */
#define QDM_IPINTF_DIR_LAN        0x1
#define QDM_IPINTF_DIR_WAN        0x2
#define QDM_IPINTF_DIR_ANY        (QDM_IPINTF_DIR_LAN|QDM_IPINTF_DIR_WAN)

UBOOL8 qdmIpIntf_isIpvxServiceUpLocked_dev2(const char *ifName,
                                            UINT32 dirMask, UINT8 ipvx);


UBOOL8 qdmIpIntf_isIpv4ServiceStartingLocked_dev2(const char *ifName,
                                            UINT32 dirMask);

UBOOL8 qdmIpIntf_isIpv6ServiceStartingLocked_dev2(const char *ifName,
                                            UINT32 dirMask);

UBOOL8 qdmIpIntf_isIpv4ServiceUpLocked_dev2(const char *ifName,
                                            UINT32 dirMask);

UBOOL8 qdmIpIntf_isIpv6ServiceUpLocked_dev2(const char *ifName,
                                            UINT32 dirMask);


/** Return TRUE if the specified wan ifname is UP
 *
 *  Do not follow this example (or cut and paste!!) this "isIPv4" way of
 *  specifying IPv4/IPv6.  Follow the isIpv4ServiceUp, isIpv6ServiceUp,
 *  and isIpvxServiceUp example above.
 *
 * @param ifName (IN) Linux intf name to look at
 * @param isIPv4 (IN) If TRUE, look at IPv4 connection state.  Else, look at
 *                    IPv6 connection state.
 *
 * @return TRUE if interface is UP in the specified IPv4/IPv6 mode
 */
UBOOL8 qdmIpIntf_isWanInterfaceUpLocked(const char *ifName, UBOOL8 isIPv4);

UBOOL8 qdmIpIntf_isWanInterfaceUpLocked_igd(const char *ifName, UBOOL8 isIPv4);

UBOOL8 qdmIpIntf_isWanInterfaceUpLocked_dev2(const char *ifName, UBOOL8 isIPv4);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_isWanInterfaceUpLocked(i, v)   qdmIpIntf_isWanInterfaceUpLocked_igd((i), (v))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_isWanInterfaceUpLocked(i, v)   qdmIpIntf_isWanInterfaceUpLocked_igd((i), (v))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_isWanInterfaceUpLocked(i, v)   qdmIpIntf_isWanInterfaceUpLocked_dev2((i), (v))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_isWanInterfaceUpLocked(i, v)   (cmsMdm_isDataModelDevice2() ? \
                             qdmIpIntf_isWanInterfaceUpLocked_dev2((i), (v)) : \
                             qdmIpIntf_isWanInterfaceUpLocked_igd((i), (v)))
#endif




/** Return TRUE if the specified Layer 3 intf name is Upstream (WAN).
 *
 * @param l3IntfName (IN) The interface name to query.
 *
 * @return TRUE if the specified Layer 3 intf name is Upstream (WAN).
 */
UBOOL8 qdmIpIntf_isIntfNameUpstreamLocked(const char *l3IntfName);
UBOOL8 qdmIpIntf_isIntfNameUpstreamLocked_igd(const char *l3IntfName);
UBOOL8 qdmIpIntf_isIntfNameUpstreamLocked_dev2(const char *l3IntfName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_isIntfNameUpstreamLocked(i)   qdmIpIntf_isIntfNameUpstreamLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_isIntfNameUpstreamLocked(i)   qdmIpIntf_isIntfNameUpstreamLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_isIntfNameUpstreamLocked(i)   qdmIpIntf_isIntfNameUpstreamLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_isIntfNameUpstreamLocked(i)   (cmsMdm_isDataModelDevice2() ? \
                             qdmIpIntf_isIntfNameUpstreamLocked_dev2((i)) : \
                             qdmIpIntf_isIntfNameUpstreamLocked_igd((i)))
#endif




/** Return TRUE if Firewall is enabled on the specified ifname.
 *  Note this firewall setting is for both IPv4 and IPv6.
 *  Note that ifName can be be WAN or LAN.
 *
 *  @param ifName (IN) The interface name to query.
 *
 *  @return TRUE if Firewall is enabled on the specified ifName.
 */

UBOOL8 qdmIpIntf_isFirewallEnabledOnIntfnameLocked(const char *ifName);

UBOOL8 qdmIpIntf_isFirewallEnabledOnIntfnameLocked_igd(const char *ifName);

UBOOL8 qdmIpIntf_isFirewallEnabledOnIntfnameLocked_dev2(const char *ifName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_isFirewallEnabledOnIntfnameLocked(i)   qdmIpIntf_isFirewallEnabledOnIntfnameLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_isFirewallEnabledOnIntfnameLocked(i)   qdmIpIntf_isFirewallEnabledOnIntfnameLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_isFirewallEnabledOnIntfnameLocked(i)   qdmIpIntf_isFirewallEnabledOnIntfnameLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_isFirewallEnabledOnIntfnameLocked(i)   (cmsMdm_isDataModelDevice2() ? \
                             qdmIpIntf_isFirewallEnabledOnIntfnameLocked_dev2((i)) : \
                             qdmIpIntf_isFirewallEnabledOnIntfnameLocked_igd((i)))
#endif



/** Return TRUE if Firewall is enabled on the specified IP.Interface fullpath
 *  Note this firewall setting is for both IPv4 and IPv6.
 *  Note that ifName can be be WAN or LAN.
 *
 *  @param ifName (IN) The interface name to query.
 *
 *  @return TRUE if Firewall is enabled on the specified ipIntfFullPath.
 */

UBOOL8 qdmIpIntf_isFirewallEnabledOnIpIntfFullPathLocked(const char *ipIntfFullPath);

UBOOL8 qdmIpIntf_isFirewallEnabledOnIpIntfFullPathLocked_igd(const char *ipIntfFullPath);

UBOOL8 qdmIpIntf_isFirewallEnabledOnIpIntfFullPathLocked_dev2(const char *ipIntfFullPath);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_isFirewallEnabledOnIpIntfFullPathLocked(i)   qdmIpIntf_isFirewallEnabledOnIpIntfFullPathLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_isFirewallEnabledOnIpIntfFullPathLocked(i)   qdmIpIntf_isFirewallEnabledOnIpIntfFullPathLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_isFirewallEnabledOnIpIntfFullPathLocked(i)   qdmIpIntf_isFirewallEnabledOnIpIntfFullPathLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_isFirewallEnabledOnIpIntfFullPathLocked(i)   (cmsMdm_isDataModelDevice2() ? \
                             qdmIpIntf_isFirewallEnabledOnIpIntfFullPathLocked_dev2((i)) : \
                             qdmIpIntf_isFirewallEnabledOnIpIntfFullPathLocked_igd((i)))
#endif



/** Return TRUE if NAT is enabled on the specified ifname.
 *  Note this NAT setting is for both IPv4 and IPv6.
 *  Note that ifName can be be WAN or LAN.
 *
 *  @param ifName (IN) The interface name to query.
 *
 *  @return TRUE if NAT is enabled on the specified ifName.
 */

UBOOL8 qdmIpIntf_isNatEnabledOnIntfNameLocked(const char *ifName);

UBOOL8 qdmIpIntf_isNatEnabledOnIntfNameLocked_igd(const char *ifName);

UBOOL8 qdmIpIntf_isNatEnabledOnIntfNameLocked_dev2(const char *ifName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_isNatEnabledOnIntfNameLocked(i)   qdmIpIntf_isNatEnabledOnIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_isNatEnabledOnIntfNameLocked(i)   qdmIpIntf_isNatEnabledOnIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_isNatEnabledOnIntfNameLocked(i)   qdmIpIntf_isNatEnabledOnIntfNameLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_isNatEnabledOnIntfNameLocked(i)   (cmsMdm_isDataModelDevice2() ? \
                             qdmIpIntf_isNatEnabledOnIntfNameLocked_dev2((i)) : \
                             qdmIpIntf_isNatEnabledOnIntfNameLocked_igd((i)))
#endif



/** Return TRUE if NAT is enabled on the specified IP.Interface fullpath
 *  Note this NAT setting is for both IPv4 and IPv6.
 *  Note that ifName can be be WAN or LAN.
 *
 *  @param ifName (IN) The interface name to query.
 *
 *  @return TRUE if NAT is enabled on the specified ipIntfFullPath.
 */

UBOOL8 qdmIpIntf_isNatEnabledOnIpIntfFullPathLocked(const char *ipIntfFullPath);

UBOOL8 qdmIpIntf_isNatEnabledOnIpIntfFullPathLocked_igd(const char *ipIntfFullPath);

UBOOL8 qdmIpIntf_isNatEnabledOnIpIntfFullPathLocked_dev2(const char *ipIntfFullPath);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_isNatEnabledOnIpIntfFullPathLocked(i)   qdmIpIntf_isNatEnabledOnIpIntfFullPathLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_isNatEnabledOnIpIntfFullPathLocked(i)   qdmIpIntf_isNatEnabledOnIpIntfFullPathLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_isNatEnabledOnIpIntfFullPathLocked(i)   qdmIpIntf_isNatEnabledOnIpIntfFullPathLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_isNatEnabledOnIpIntfFullPathLocked(i)   (cmsMdm_isDataModelDevice2() ? \
                             qdmIpIntf_isNatEnabledOnIpIntfFullPathLocked_dev2((i)) : \
                             qdmIpIntf_isNatEnabledOnIpIntfFullPathLocked_igd((i)))
#endif


/** Return TRUE if FullConeNAT is enabled on the specified ifname.
 *  Note this NAT setting is for both IPv4 and IPv6.
 *  Note that ifName can be be WAN or LAN.
 *
 *  @param ifName (IN) The interface name to query.
 *
 *  @return TRUE if FullConeNAT is enabled on the specified ifName.
 */
UBOOL8 qdmIpIntf_isFullConeNatEnabledOnIntfNameLocked_dev2(const char *ifName);


/** Return TRUE if FullConeNAT is enabled on the specified IP.Interface fullpath
 *  Note this NAT setting is for both IPv4 and IPv6.
 *  Note that ifName can be be WAN or LAN.
 *
 *  @param ifName (IN) The interface name to query.
 *
 *  @return TRUE if FullConeNAT is enabled on the specified ipIntfFullPath.
 */
UBOOL8 qdmIpIntf_isFullConeNatEnabledOnIpIntfFullPathLocked_dev2(const char *ipIntfFullPath);


/** Return true if the specified interface is a WAN interface, and if it
 * is a bridge connection.
 *
 * @param ifname    (IN) Linux interface name.
 *
 * @return true if the specified interface is a WAN interface, and if it
 * is a bridge connection.
 */
UBOOL8 qdmIpIntf_isWanInterfaceBridgedLocked(const char *ifname);
UBOOL8 qdmIpIntf_isWanInterfaceBridgedLocked_igd(const char *ifname);
UBOOL8 qdmIpIntf_isWanInterfaceBridgedLocked_dev2(const char *ifname);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_isWanInterfaceBridgedLocked(i)  qdmIpIntf_isWanInterfaceBridgedLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_isWanInterfaceBridgedLocked(i)  qdmIpIntf_isWanInterfaceBridgedLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_isWanInterfaceBridgedLocked(i)  qdmIpIntf_isWanInterfaceBridgedLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_isWanInterfaceBridgedLocked(i)  (cmsMdm_isDataModelDevice2() ? \
                          qdmIpIntf_isWanInterfaceBridgedLocked_dev2((i)) : \
                          qdmIpIntf_isWanInterfaceBridgedLocked_igd((i)))
#endif




/** Return TRUE if there is at least 1 WAN service defined and all WAN
 *  services are Bridge services.
 *  Use this function instead of rutWan_isAllBridgePvcs().
 */

UBOOL8 qdmIpIntf_isAllBridgeWanServiceLocked(void);
UBOOL8 qdmIpIntf_isAllBridgeWanServiceLocked_igd(void);
UBOOL8 qdmIpIntf_isAllBridgeWanServiceLocked_dev2(void);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_isAllBridgeWanServiceLocked()   qdmIpIntf_isAllBridgeWanServiceLocked_igd()
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_isAllBridgeWanServiceLocked()   qdmIpIntf_isAllBridgeWanServiceLocked_igd()
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_isAllBridgeWanServiceLocked()   qdmIpIntf_isAllBridgeWanServiceLocked_dev2()
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_isAllBridgeWanServiceLocked()   (cmsMdm_isDataModelDevice2() ? \
                             qdmIpIntf_isAllBridgeWanServiceLocked_dev2() : \
                             qdmIpIntf_isAllBridgeWanServiceLocked_igd())
#endif




/** Return TRUE if there is a bridge wan service in the system.
 *
 * @return TRUE if there is a bridge wan service in the system.
 */
UBOOL8 qdmIpIntf_isBridgedWanExistedLocked(void);
UBOOL8 qdmIpIntf_isBridgedWanExistedLocked_igd(void);
UBOOL8 qdmIpIntf_isBridgedWanExistedLocked_dev2(void);


#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_isBridgedWanExistedLocked()  qdmIpIntf_isBridgedWanExistedLocked_igd()
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_isBridgedWanExistedLocked()  qdmIpIntf_isBridgedWanExistedLocked_igd()
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_isBridgedWanExistedLocked()  qdmIpIntf_isBridgedWanExistedLocked_dev2()
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_isBridgedWanExistedLocked()  (cmsMdm_isDataModelDevice2() ? \
                                 qdmIpIntf_isBridgedWanExistedLocked_dev2() : \
                                 qdmIpIntf_isBridgedWanExistedLocked_igd())
#endif




/** Return TRUE if there is a Routed wan service in the system.
 *
 * @return TRUE if there is a Routed wan service in the system.
 */
UBOOL8 qdmIpIntf_isRoutedWanExistedLocked(void);
UBOOL8 qdmIpIntf_isRoutedWanExistedLocked_igd(void);
UBOOL8 qdmIpIntf_isRoutedWanExistedLocked_dev2(void);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_isRoutedWanExistedLocked()  qdmIpIntf_isRoutedWanExistedLocked_igd()
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_isRoutedWanExistedLocked()  qdmIpIntf_isRoutedWanExistedLocked_igd()
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_isRoutedWanExistedLocked()  qdmIpIntf_isRoutedWanExistedLocked_dev2()
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_isRoutedWanExistedLocked()  (cmsMdm_isDataModelDevice2() ? \
                                 qdmIpIntf_isRoutedWanExistedLocked_dev2() : \
                                 qdmIpIntf_isRoutedWanExistedLocked_igd())
#endif




/** Return the number of WAN services (routed and bridged) that exists on
 *  the specified Layer 2 IntfName.
 *
 *  @param l2IntfName (IN) Layer 2 intfName
 *
 *  @return the number of WAN services (routed and bridged) that exists on
 *  the specified Layer 2 IntfName.
 */
UINT32 qdmIpIntf_getNumberOfWanServicesOnLayer2IntfNameLocked_dev2(const char *l2IntfName);




/** Given the Layer 3 intfName, return the Layer 2 intfName
 *
 * @param l3IntfName (IN) The layer 3 intfName
 * @param l2IntfName (OUT) The result layer 2 intfName.  Caller must pass in
 *                         a buffer of at least CMS_IFNAME_LENGTH bytes.
 *
 * @return CmsRet
 */
CmsRet qdmIpIntf_getLayer2IntfNameByLayer3IntfNameLocked(const char *l3IntfName, char *l2IntfName);
CmsRet qdmIpIntf_getLayer2IntfNameByLayer3IntfNameLocked_igd(const char *l3IntfName, char *l2IntfName);
CmsRet qdmIpIntf_getLayer2IntfNameByLayer3IntfNameLocked_dev2(const char *l3IntfName, char *l2IntfName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_getLayer2IntfNameByLayer3IntfNameLocked(i, a)   qdmIpIntf_getLayer2IntfNameByLayer3IntfNameLocked_igd((i), (a))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_getLayer2IntfNameByLayer3IntfNameLocked(i, a)   qdmIpIntf_getLayer2IntfNameByLayer3IntfNameLocked_igd((i), (a))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_getLayer2IntfNameByLayer3IntfNameLocked(i, a)   qdmIpIntf_getLayer2IntfNameByLayer3IntfNameLocked_dev2((i), (a))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_getLayer2IntfNameByLayer3IntfNameLocked(i, a)   (cmsMdm_isDataModelDevice2() ? \
                 qdmIpIntf_getLayer2IntfNameByLayer3IntfNameLocked_dev2((i), (a)) : \
                 qdmIpIntf_getLayer2IntfNameByLayer3IntfNameLocked_igd((i), (a)))
#endif




/** Given an interface group name, return the Linux bridge interface name
 *
 * @param groupName  (IN) interface group name
 * @param intfName   (OUT) Linux bridge interface name, buffer must be at least
 *                         CMS_IFNAME_LENGTH bytes
 *
 * @return CmsRet
 */
CmsRet qdmIpIntf_getBridgeIntfNameByGroupNameLocked_dev2(const char *groupName, char *intfName);


/** Return TRUE if found wan interface of specified group name
 *
 */
UBOOL8 qdmIpIntf_getWanIntfNameByGroupNameLocked_dev2(const char *groupName, char *intfName);





/** Return TRUE if IPv6 is enabled on the specified IP interface name.
 *  Note that in Hybrid mode, we look in the TR181 part of the data model
 *  for IPv6 Enabled.
 *
 */
UBOOL8 qdmIpIntf_isIpv6EnabledOnIntfNameLocked(const char *ipIntfName);
UBOOL8 qdmIpIntf_isIpv6EnabledOnIntfNameLocked_igd(const char *ipIntfName);
UBOOL8 qdmIpIntf_isIpv6EnabledOnIntfNameLocked_dev2(const char *ipIntfName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_isIpv6EnabledOnIntfNameLocked(i)   qdmIpIntf_isIpv6EnabledOnIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_isIpv6EnabledOnIntfNameLocked(i)   qdmIpIntf_isIpv6EnabledOnIntfNameLocked_dev2((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_isIpv6EnabledOnIntfNameLocked(i)   qdmIpIntf_isIpv6EnabledOnIntfNameLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_isIpv6EnabledOnIntfNameLocked(i)   (cmsMdm_isDataModelDevice2() ? \
                             qdmIpIntf_isIpv6EnabledOnIntfNameLocked_dev2((i)) : \
                             qdmIpIntf_isIpv6EnabledOnIntfNameLocked_dev2((i)))
#endif

UBOOL8 qdmIpIntf_findIpv6Prefix(const InstanceIdStack *iidStackIpIntf, const char *prefix,
                                const char *origin, const char *staticType, InstanceIdStack *iidStackIpv6Prefix);


typedef enum Ipv6OriginFilter {
   CMS_IPV6_ORIGIN_STATIC=0,
   CMS_IPV6_ORIGIN_AUTOCONFIG=1,
   CMS_IPV6_ORIGIN_DHCPV6=2,
   CMS_IPV6_ORIGIN_ANY=3
} CmsIpv6OriginFilterEnum;

/** Get Ipv6Address info from ip.interface.ipv6adreess object by ip interface name.  It is a base function and 
 * can be call by other qdm functions to get only needed address object element.
 * 
 *  @param ifName        (IN) IP Interface name
 *  @param ipAddress     (OUT) ipv6Address  string
 *  @param ipAddressLen  (IN) ipv6Address string len
 *  @param origin        (OUT) origin string (TR181 Enumeration, default "Static")
 *  @param originLen     (IN) origin string len
 *  @param prefixPath    (OUT) full path of corresponding prefix string
 *  @param prefixPathLen (IN) prefixPath len
 *  @param isWan         (OUT) if TRUE, it is a WAN IP interface otherwise, LAN IP interface.
 *  @param wanOriginFilter  (IN) specify origin filter for WAN
 *  @param lanOriginFilter  (IN) specify origin filter for LAN
 *
 * @return CmsRet.
 */

CmsRet qdmIpIntf_getIpv6AddrInfoByNameLocked_dev2(const char *ifName, 
                                                  char *ipAddress, 
                                                  UINT32 ipAdressLen,
                                                  char *origin,
                                                  UINT32 originLen,
                                                  char *prefixPath,
                                                  UINT32 prefixPathLen,
                                                  UBOOL8 *isWan,
                                                  CmsIpv6OriginFilterEnum wanOriginFilter,
                                                  CmsIpv6OriginFilterEnum lanOriginFilter);
                                                  

/** Get Ipv6Address with ip interface name.
 *  Note that in Hybrid mode, we look in the TR181 part of the data model
 *  for IPv6 Enabled.
 * 
 *  @param ifName        (IN) IP Interface name
 *  @param ipAddress    (OUT) Buffer must be at least CMS_IPADDR_LENGTH bytes
 *
 * @return CmsRet.
 */
CmsRet qdmIpIntf_getIpv6AddressByNameLocked(const char *ifName, char *ipAddress);
CmsRet qdmIpIntf_getIpv6AddressByNameLocked_igd(const char *ifName, char *ipAddress);
CmsRet qdmIpIntf_getIpv6AddressByNameLocked_dev2(const char *ifName, char *ipAddress);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIpIntf_getIpv6AddressByNameLocked(i, a)   qdmIpIntf_getIpv6AddressByNameLocked_igd((i), (a))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIpIntf_getIpv6AddressByNameLocked(i, a)   qdmIpIntf_getIpv6AddressByNameLocked_dev2((i), (a))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIpIntf_getIpv6AddressByNameLocked(i, a)   qdmIpIntf_getIpv6AddressByNameLocked_dev2((i), (a))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIpIntf_getIpv6AddressByNameLocked(i, a)   (cmsMdm_isDataModelDevice2() ? \
                 qdmIpIntf_getIpv6AddressByNameLocked_dev2((i), (a)) : \
                 qdmIpIntf_getIpv6AddressByNameLocked_dev2((i), (a)))
#endif


/** Get active delegated prefix information on a LAN interface
 * 
 *  @param ifName       (IN) IP Interface name
 *  @param prefix       (OUT) Buffer must be at least CMS_IPADDR_LENGTH bytes
 *
 * @return CmsRet.
 */
UBOOL8 qdmIpIntf_getIpv6DelegatedPrefixByNameLocked_dev2(const char *ifname, char *prefix);


/** Get LAN Ipv6Address for DHCPv6 server with ip interface name. (DProxy)
 *  Note: LAN address could be either ULA or GUA.
 * 
 *  @param ifName       (IN) IP Interface name
 *  @param ipAddress    (OUT) Buffer must be at least CMS_IPADDR_LENGTH bytes
 *
 * @return CmsRet.
 */
CmsRet qdmIpIntf_getDproxyIpv6AddressByNameLocked_dev2(const char *ifName, char *ipAddress);


/** Get Associated WAN interface name by LAN interface name
 *  Note: For IPv6, we use child prefix's parent prefix to get WAN interface
 * 
 *  @param lanFullPath         (IN) full path of LAN IP.Interface
 *  @param ipvx                (IN)
 *  @param wanIpIntfPathDesc   (OUT) path descriptor of WAN IP.Interface
 *
 * @return CmsRet.
 */
CmsRet qdmIpIntf_getAssociatedWanIpIntfPathDescByLanFullPathLocked_dev2(const char *lanFullPath, MdmPathDescriptor *wanIpIntfPathDesc, UINT32 ipvx);


/** check whether associated WAN service is up
 * 
 *  @param lanFullPath         (IN) full path of LAN IP.Interface
 *  @param ipvx                (IN)
 *
 * @return TRUE if service is UP in the specified IPv4/IPv6 mode
 */
UBOOL8 qdmIpIntf_isAssociatedWanInterfaceUpLocked_dev2(const char *lanFullPath, UINT32 ipvx);


/** Get PathDescriptor for Dev2MacFilterObject by
 *  IP.Interface pointed to by ipIntfFullPath.
 *  Only dev2 versions of this functions is needed right now.
 *   
 *  @Param ipIntfFullPath  (IN)  IP.Interface fullpath
 *  @Param macFiltPathDesc (OUT) pathDesc to macfilter object that corresponds to the given IP.Interface fullpath.
 *   
 *  @return CmsRet.
 */
CmsRet qdmIpIntf_getMacFilterByFullPathLocked_dev2(const char *ipIntfFullPath, MdmPathDescriptor *macFiltPathDesc);


#endif /* _QDM_INTF_H_ */

